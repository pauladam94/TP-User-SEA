#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <string.h>


long str_to_hexa(char * s) {
 long result = 0;
 int c;
 int w=0;
  while (w<strlen(s)-1) {
   result = result << 4;
   if (c=(*(s+w)-'0'),(c>=0 && c <=9)) result|=c;
   else if (c=(*(s+w)-'A'),(c>=0 && c <=5)) result|=(c+10);
   else if (c=(*(s+w)-'a'),(c>=0 && c <=5)) result|=(c+10);
   else break;
   ++w;
  }
 return result;
}

int write_memory(long adr, int inst, int pid) {
  char path[16];
  sprintf(path, "/proc/%d/mem", pid);
  printf("path : %s\n", path);
  printf("adr : 0x%X\ninst : 0x%X\n", adr, inst);

  FILE *f = fopen(path, "wb");
  if (f == NULL) {
    printf("Erreur dans l'ouverture du fichier memoire du processus\n");
    return -1;
  }

  if(fseek(f, adr, SEEK_SET) == -1){
    printf("Erreur de placement dans le fichier\n");
    return -1;
  }

  if(fwrite((void*)&inst, 1, sizeof(int),f) == 0){
    printf("Erreur d'ecriture dans le fichier\n");
    return -1;
  }

  fclose(f);
  return 0;
}

int get_pid(){
  system("ps -aux | grep \"./main\" | cut -c 12-17 | head -n 1 > pid.txt");
  
  FILE* f = fopen("pid.txt","r");
  if(f==NULL){
    printf("Erreur lors de la creation du fichier du pid\n");
    return -1;
  }

  char pid[100];

  fgets(pid, 100, f);
  fclose(f);

  system("rm pid.txt");

  printf("pid:%s", pid);
  
  int i = atoi(pid);
  printf("atoi(pid):%d\n", i);

  return i;
}

long get_adr_fun(char* s){
  char sys_call[200];
  sprintf(sys_call, "nm toy_c_programm/main | grep \"%s\" | cut -c 1-16 > adr.txt",s);
  printf("syscall : %s\n", sys_call);
  system(sys_call);

  FILE* f = fopen("adr.txt","r");
  if(f==NULL){
    printf("Erreur lors de la creation du fichier de l'adresse\n");
    return -1;
  }

  char adr[100];

  fgets(adr, 100, f);
  fclose(f);

  system("rm adr.txt");

  printf("adr:%s", adr);
  long i = str_to_hexa(adr);
  printf("hexa adr:%d\n",i);

  return i;
}


int main(int argc, char **argv) {
  int process_pid = get_pid();
  long adr_fun = get_adr_fun(argv[1]);

  // 1. Attacher au processus cible
  if (ptrace(PTRACE_ATTACH, process_pid, 0, 0) == -1) {
    printf("ptrace_attach a echoue\n");
    return 1;
  }

  int status;
  printf("pid: %d\n",waitpid(process_pid, &status, 0));


  // Recupere la valeur des registres
  int* data;
  int* addr;
  ptrace(PTRACE_GETREGS, process_pid, data, addr);


  // Modifier le code du processus



  // Modifier les valeurs des registres
  ptrace(PTRACE_SETREGS, process_pid, data, addr);






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
  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // 5. Détacher du processus
  ptrace(PTRACE_DETACH, process_pid, 0, 0);

}