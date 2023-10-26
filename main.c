#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <string.h>

int write_memory(int adr, int inst, int pid) {
  // change pid by pid_value

  char path[100] = "/proc/";
  char pid_s[10];
  sprintf(pid_s,"%d",pid);
  strcat(path,pid_s);
  strcat(path, "/mem");

  printf("path : %s\n", path);
  printf("adr : 0x%X\ninst : 0x%X\n", adr, inst);

  FILE *f = fopen(path, "wb");
  if (f == NULL) {
    printf("Erreur dans l'ouverture du fichier memoire du processus\n");
    return -1;
  }

  printf("*f before fseek : 0x%X\n", *f);
  if(fseek(f, adr, SEEK_SET) == -1){
    printf("Erreur de placement dans le fichier\n");
    return -1;
  }
  printf("*f after fseek : 0x%X\n", *f);

  if(fwrite((void*)&inst, 1, sizeof(int),f) == 0){
    printf("Erreur d'ecriture dans le fichier\n");
    return -1;
  }
  fclose(f);
};

int main() {
  int process_pid = 11823;
  int adr_fun = 0x401156;

  // 1. Attacher au processus cible
  if (ptrace(PTRACE_ATTACH, process_pid, 0, 0) == -1) {
    printf("ptrace_attach a echoue\n");
    return 1;
  }

  int status;
  printf("pid: %d\n",waitpid(process_pid, &status, 0));

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
  if(write_memory(adr_fun, new_inst, process_pid) == -1){
    printf("Erreur lors de l'appel de write_memory\n");
    return -1;
  }

  // 4. Continuer l'exécution du processus
  // ptrace(PTRACE_CONT, process_pid, 0, 0);

  // 5. Détacher du processus
  ptrace(PTRACE_DETACH, process_pid, 0, 0);
}
