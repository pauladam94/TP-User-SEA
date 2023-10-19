#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/types.h>

void write_memory(int adr, int inst) {
  // change pid by pid_value
  FILE *f = fopen("/proc/pid/mem", "rb");
  if (f == NULL) {
    printf("");
    return 1;
  }
  fseek(f, );
  fclose(f);
};

int main() {
  printf("Hello, World!\n");

  int process_pid = 34807;
  int adr_fun = 0x4011360;

  // 1. Attacher au processus cible
  if (ptrace(PTRACE_ATTACH, process_pid, 0, 0) == -1) {
    printf("ptrace_attach a echoue");
    return 1;
  }

  int status;
  waitpid(process_pid, &status, 0);

  // 2. Attendre que le processus cible atteigne la fonction spécifique
  /*while(1){
      int status;
      waitpid(process_pid, &status, 0);
      if (status == adr_fun) break;
  }*/

  // 3. Modifier dynamiquement le code
  // Remplacez la première instruction de la fonction par 0xCC (instruction de
  // trap)
  int new_inst = 0xCC;
  // write_memory(adr_fun+1, new_inst);

  // 4. Continuer l'exécution du processus
  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // 5. Détacher du processus
  ptrace(PTRACE_DETACH, process_pid, 0, 0);
}
