#include "kernel/types.h"
#include "kernel/fcntl.h"
#include "user/user.h"
#include "kernel/riscv.h"

#define PGSIZE 4096
#define NPAGES 32

int
main(int argc, char *argv[])
{
  char *mem = sbrk(NPAGES * PGSIZE);
  
  // check if sbrk failed
  if (mem == (char *)-1) {
    exit(1);
  }

  char *marker = "Here it is: ";
  int markLength = strlen(marker);

  for (int i = 0; i <= NPAGES * PGSIZE - markLength; i++)
  {
    if (memcmp(mem + i, marker, markLength) == 0)
    {
      char *secret = mem + i + markLength;

      int remaining = NPAGES * PGSIZE - (i + markLength);
      int len = 0;

      while (len < remaining && secret[len] != '\0')
        len++;

      if (len == 0 || len == remaining) {
        continue;
      }

      printf("%s\n", secret);

      exit(0);
    }
  }

  exit(1);
}
