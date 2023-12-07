#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>

long str_to_hexa(char *s) {
  long result = 0;
  int c;
  int w = 0;
  while (w < strlen(s) - 1) {
    result = result << 4;
    if (c = (*(s + w) - '0'), (c >= 0 && c <= 9))
      result |= c;
    else if (c = (*(s + w) - 'A'), (c >= 0 && c <= 5))
      result |= (c + 10);
    else if (c = (*(s + w) - 'a'), (c >= 0 && c <= 5))
      result |= (c + 10);
    else
      break;
    ++w;
  }
  return result;
}

int read_memory(long adr, int length, int pid, char *res){
  char path[16];
  sprintf(path, "/proc/%d/mem", pid);

  FILE *f = fopen(path, "r+");
  if (f == NULL) {
    printf("Erreur dans l'ouverture du fichier memoire du processus\n");
    return -1;
  }

  if (fseek(f, adr, SEEK_SET) == -1) {
    printf("Erreur de placement dans le fichier\n");
    return -1;
  }

  if (fread(res, length, sizeof(char), f) == 0) {
    printf("Erreur de lecture dans le fichier\n");
    return -1;
  }

  fclose(f);
  return 0;

}

// TODO rename res
int write_memory(long adr, char *inst, int length, int pid, char *res) {
  char path[16];
  printf("length : %d\n", length);
  sprintf(path, "/proc/%d/mem", pid);
  printf("path : %s\n", path);
  printf("adr : 0x%X\n", adr);
  for (int i = 0; i < length; i++) {
    printf("inst : 0x%X\n", inst[i]);
  }

  FILE *f = fopen(path, "r+");
  if (f == NULL) {
    printf("Erreur dans l'ouverture du fichier memoire du processus\n");
    return -1;
  }

  if (fseek(f, adr, SEEK_SET) == -1) {
    printf("Erreur de placement dans le fichier\n");
    return -1;
  }

  if (fread(res, length, sizeof(char), f) == 0) {
    printf("Erreur de lecture dans le fichier\n");
    return -1;
  }

  for (int i = 0; i < length; i++) {
    printf("instruction %d sauvegardée : 0x%X\n", i, res[i]);
  }

  if (fseek(f, adr, SEEK_SET) == -1) {
    printf("Erreur de placement dans le fichier\n");
    return -1;
  }
  int test = fwrite(inst, length, sizeof(char), f);
  if (test == 0) {
    printf("Erreur d'ecriture dans le fichier\n");
    printf("test : %d\n", test);
    return -1;
  }

  fclose(f);
  return 0;
}

int get_pid() {
  system("ps -aux | grep \"./toy_c_program/toy_c_program\" | cut -c 12-17 | "
         "head -n 1 > pid.txt");

  FILE *f = fopen("pid.txt", "r");
  if (f == NULL) {
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

long get_adr_fun(char *s) {
  char sys_call[200];
  sprintf(
      sys_call,
      "nm toy_c_program/toy_c_program | grep \"%s\" | cut -c 1-16 > adr.txt",
      s);
  printf("syscall : %s\n", sys_call);
  system(sys_call);

  FILE *f = fopen("adr.txt", "r");
  if (f == NULL) {
    printf("Erreur lors de la creation du fichier de l'adresse\n");
    return -1;
  }

  char adr[100];

  fgets(adr, 100, f);
  fclose(f);

  system("rm adr.txt");

  printf("adr:%s", adr);
  long i = str_to_hexa(adr);
  printf("hexa adr:%d\n", i);

  return i;
}

int trap_call_trap(long addr_write_in_fun, int process_pid, long addr_fun_call, int *args_fun, bool *args_ptr, char *return_param, int stack_push_size) {
  int status;
  // 3. Modifier dynamiquement le code : trap call trap
  int length = 4;
  char new_inst[4] = {0xCC, 0xFF, 0xD0, 0xCC};
  char sauv_line[4]; // 20 = taille max d'une instruction (x86 CrInGe)
  if (write_memory(addr_write_in_fun, new_inst, length, process_pid,
                   sauv_line) == -1) {
    printf("Erreur lors de l'appel de write_memory pour trap call trap\n");
    return -1;
  }

  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // Wait for the first trap
  printf("pid: %d\n", waitpid(process_pid, &status, 0));

  // Recupere la valeur des registres

  struct user_regs_struct regs;
  struct user_regs_struct sauv_regs;
  ptrace(PTRACE_GETREGS, process_pid, 0, &sauv_regs);
  ptrace(PTRACE_GETREGS, process_pid, 0, &regs);

  long long int *registr[3] = {&(regs.rdi), &(regs.rsi), &(regs.rdx)};

  for (int i = 0; i < 3; i++) {
    // Modifier les registres pour call la bonne fonction
    if (args_ptr[i]) { // si le premier argument est un pointeur
      // On incremente le pointeur de pile

      regs.rsp -= args_fun[i];
      // On ajoute une valeur dans la pile
      // printf("regs.rsp : %p\n", regs.rsp);
      length = 4;
      char new_val[4] = {42, 0, 0, 0};
      char res[4] = {0, 0, 0, 0}; // inutile

      if (write_memory(regs.rsp, new_val, length, process_pid, res) == -1) {
        printf(
            "Erreur lors de l'appel de write_memory pour changer la stack\n");
        return -1;
      }
      *(registr[i]) = regs.rsp; // On pases en parametre un pointeur vers le
                                // sommet de la pile
    } else {
      *(registr[i]) = args_fun[i];
    }
  }

  regs.rax = addr_fun_call;

  // regs.rsi = 0;
  // regs.rdx = 0;

  // Modifier les valeurs des registres
  ptrace(PTRACE_SETREGS, process_pid, 0, &regs);

  ptrace(PTRACE_CONT, process_pid, 0, 0);
  // Wait for the second trap
  printf("pid: %d\n", waitpid(process_pid, &status, 0));

  ptrace(PTRACE_GETREGS, process_pid, 0, &regs);
  printf("rax : %d\n", regs.rax);
  for (int i = 0; i < stack_push_size; i++){
    char * res;
    read_memory(regs.rsp + i * sizeof(char), length, process_pid, res);
    return_param[i] = res;
  }

  // Reecriture des lignes effacees
  // fwrite sauv_line addr : addr_puissance

  char buffer[4];

  if (write_memory(addr_write_in_fun, sauv_line, length, process_pid, buffer) ==
      -1) {
    printf("Erreur lors de l'appel de write_memory pour restore\n");
    return -1;
  }
  sauv_regs.rip = sauv_regs.rip - 1;

  ptrace(PTRACE_SETREGS, process_pid, 0, &sauv_regs);

  ptrace(PTRACE_CONT, process_pid, 0, 0);
  return -1;
}

int main(int argc, char **argv) {
  printf("sizeof(int) = %d\n", sizeof(int));
  int process_pid = get_pid();
  long addr_puissance = get_adr_fun(argv[1]);
  long addr_puissance_opti = get_adr_fun(argv[2]);
  long addr_posix_memalign = get_adr_fun("posix_memalign");

  // 1. Attacher au processus cible
  if (ptrace(PTRACE_ATTACH, process_pid, 0, 0) == -1) {
    printf("ptrace_attach a echoue\n");
    return 1;
  }

  int status;
  printf("pid: %d\n", waitpid(process_pid, &status, 0));
  int size_code = 10;
  int args_fun[3] = {sizeof(long long int), getpagesize(), size_code};
  bool args_ptr[3] = {true, false, false};

  // int args_fun[3] = {sizeof(int), 0, 0};
  // bool args_ptr[3] = {true, false, false};
  int stack_push_size = 0;
  for (int i = 0; i < 3; i++) {
    if (args_ptr[i]) {
      stack_push_size += args_fun[i];
    }
  }
  char *return_stack = malloc(stack_push_size);

  // Tester le cas pthologique ou PC = adr ou PC = adr+1 ou Pc = adr+2
  trap_call_trap(addr_puissance, process_pid, addr_posix_memalign, args_fun,
                 args_ptr, return_stack, stack_push_size);

  // Traitement return_stack
  // Get a pointeur in the heap

  // Write the new function a this pointer (with write_memory)
  // Translation of our function in bitecode
  // Get size of this translation => put it in size_code

  // trap_call_trap of this new function


  // 4. Continuer l'exécution du processus
  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // 5. Détacher du processus
  ptrace(PTRACE_DETACH, process_pid, 0, 0);
}