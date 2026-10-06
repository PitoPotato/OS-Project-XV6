#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"

static void print(char *digits, int len, int is_num, int rem6) {
    if(len == 0 || !is_num) return;
    if(digits[len - 1] != '0' && digits[len - 1] != '5' && rem6 != 0) return;

    digits[len] = '\0';
    int p = 0;
    while(p + 1 < len && digits[p] == '0') ++p;
    printf("%s\n", digits + p);
}

static int solve(int fd) {
    char buf[512];
    int capa = 128;
    char *digits = malloc(capa);

    if(digits == 0) {
        fprintf(2, "sixfive: out of memory\n");
        return 1;
    }

    int n; 
    int len = 0, is_num = 1, rem6 = 0;
    while((n = read(fd, buf, sizeof(buf))) > 0) {
        for(int i = 0; i < n; ++i) {
            char c = buf[i];
            
            if(strchr(" -\r\t\n./,", c) != 0) {
                print(digits, len, is_num, rem6);
                len = 0, is_num = 1, rem6 = 0;
            }
            else if(c < '0' || c > '9') is_num = 0;
            else if(is_num) {

                //auto_fit
                if(len + 1 >= capa) {
                    char *new_digits = malloc(capa * 2);
                    if (new_digits == 0) {
                        fprintf(2, "sixfive: out of memory\n");
                        free(digits);
                        return 1;
                    }

                    memmove(new_digits, digits, len);
                    free(digits);

                    digits = new_digits;
                    capa *= 2;
                }

                digits[len++] = c;
                rem6 = (rem6 * 10 + c - '0') % 6;
            }
        }
    }

    if (n < 0) {
        fprintf(2, "sixfive: read error\n");
        free(digits);
        return 1;
    }

    print(digits, len, is_num, rem6);
    free(digits);

    return 0;
}

int main(int argc, char *argv[]) {
    if(argc == 1) exit(solve(0));

    int status = 0;
    for(int i = 1; i < argc; ++i) {
        int fd = open(argv[i], O_RDONLY);

        if(fd < 0) {
            status = 1;
            fprintf(2, "sixfive: cannot open %s\n", argv[i]);
            continue;
        }

        status |= solve(fd);
        close(fd);
    }

    exit(status);
}