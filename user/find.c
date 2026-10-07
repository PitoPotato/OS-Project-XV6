/* ===== LIBRARIES ===== */
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "kernel/param.h"
#include "user/user.h"

/* ===== PROTOTYPE ===== */
void run_exec(char *command_argv[], char *path);

/* ===== CODE =====*/
// Return the last element of path
char * basename(char *path) {
    char *p;

    p = path + strlen(path);
    while (p >= path && *p != '/')
        p--;

    return p + 1;
}

void find(char *path, char *target, char *exec_argv[]) {
    char buf[512];
    char *p;
    int fd;
    struct stat st;
    struct dirent de;

    // Open current path 
    if ((fd = open(path, O_RDONLY)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    // store the current path's metadata in stat struct
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    // check if current path's basename match 
    if (strcmp(basename(path), target) == 0) {
        if (exec_argv == 0)
            printf("%s\n", path);
        else
            run_exec(exec_argv, path);
    }

    // There is no children to search.
    if (st.type != T_DIR) {
        close(fd);
        return;
    }

    // Check if path is too long
    if (strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
        fprintf(2, "find: path too long\n");
        close(fd);
        return;
    }

    // Keep the current path in buf
    strcpy(buf, path);
    p = buf + strlen(buf);
    *p++ = '/';

    // Read each entry in folder and using recursion to search
    while (read(fd, &de, sizeof(de)) >= sizeof(de)) {
        if (de.inum == 0)
        continue;

        // skip "." and ".." entries
        if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0)
        continue;

        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = '\0';

        // Recursively search the entry
        find(buf, target, exec_argv);
    }

    close(fd);
}

void run_exec(char *command_argv[], char* path){
    int pid;
    int status;

    pid = fork();
    if (pid < 0) {
        fprintf(2, "find: fork failed\n");
        return;
    }

    if(pid == 0) {
        char *new_argv[MAXARG];
        int i = 0;
        while(command_argv[i] != 0){
            new_argv[i] = command_argv[i];
            i++;
        }

        new_argv[i++] = path;
        new_argv[i] = 0;

        exec(new_argv[0], new_argv);
        fprintf(2, "find: exec %s failed\n", new_argv[0]);
        exit(1);
    }

    wait(&status);
}

int main(int argc, char *argv[]) {
    char **command_argv;

    if (argc < 3) {
        fprintf(2, "usage: find path name [-exec command args...]\n");
        exit(1);
    }

    command_argv = 0;
    
    if (argc > 3) {
        if (strcmp(argv[3], "-exec") != 0 || argc < 5) {
            fprintf(2, "usage: find path name [-exec command args...]\n");
            exit(1);
        }
        command_argv = &argv[4];
    }

    find(argv[1], argv[2], command_argv);
    exit(0);
}