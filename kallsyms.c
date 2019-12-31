// This file is part of KASLD - https://github.com/bcoles/kasld
// Read startup symbol from /proc/kallsyms
// Requires kernel.kptr_restrict = 0 (Default on Debian <= 9 systems)
// Based on original code by spender:
// - https://grsecurity.net/~spender/exploits/exploit.txt

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/utsname.h>

struct utsname get_kernel_version() {
  struct utsname u;
  if (uname(&u) != 0) {
    printf("[-] uname(): %m\n");
    exit(1);
  }
  return u;
}

unsigned long get_kernel_addr_kallsyms() {
  FILE *f;
  unsigned long addr = 0;
  char dummy;
  char sname[256];
  char* name;
  const char* path = "/proc/kallsyms";

  printf("[.] trying %s...\n", path);
  f = fopen(path, "r");
  if (f == NULL) {
    printf("[-] open/read(%s): %m\n", path);
    return 0;
  }

  struct utsname u = get_kernel_version();

  if (strstr(u.machine, "64") != NULL) {
    name = "startup_64";
  } else if (strstr(u.machine, "86") != NULL) {
    name = "startup_32";
  } else {
    printf("[-] kernel startup symbol for arch '%s' is unknown\n", u.machine);
    return 0;
  }

  int ret = 0;
  while (ret != EOF) {
    ret = fscanf(f, "%p %c %s\n", (void **)&addr, &dummy, sname);
    if (ret == 0) {
      fscanf(f, "%s\n", sname);
      continue;
    }
    if (!strcmp(name, sname)) {
      fclose(f);
      if (addr == 0)
        printf("[-] kernel base not found in %s\n", path);
      return addr;
    }
  }

  fclose(f);
  printf("[-] kernel base not found in %s\n", path);
  return 0;
}

int main (int argc, char **argv) {
  unsigned long addr = get_kernel_addr_kallsyms();
  if (!addr) return 1;

  printf("kernel base (certain): %lx\n", addr);

  return 0;
}
