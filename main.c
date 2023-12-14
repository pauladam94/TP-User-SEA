#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
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

// int read_memory(long adr, int length, int pid, char *res)
//
int read_memory(long adr, int length, int pid, char *res) {
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

int trap_call_trap(long addr_write_in_fun, int process_pid, long addr_fun_call,
                   int *args_fun, bool *args_ptr, char *new_value_stack,
                   char *return_param, int stack_push_size) {
  int status;
  // 3. Modifier dynamiquement le code : trap call trap
  const int length = 4;
  char new_inst[4] = {0xCC, 0xFF, 0xD0, 0xCC};
  char sauv_line[4]; // 20 = taille max d'une instruction (x86 CrInGe)
  if (write_memory(addr_write_in_fun, new_inst, length, process_pid,
                   sauv_line) == -1) {
    printf("Erreur lors de l'appel de write_memory pour trap call trap\n");
    return -1;
  }

  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // Wait for the first trap
  printf("pid: %d first trap\n", waitpid(process_pid, &status, 0));

  // Recupere la valeur des registres

  struct user_regs_struct regs;
  struct user_regs_struct sauv_regs;
  ptrace(PTRACE_GETREGS, process_pid, 0, &sauv_regs);
  ptrace(PTRACE_GETREGS, process_pid, 0, &regs);

  long long int *registr[3] = {&(regs.rdi), &(regs.rsi), &(regs.rdx)};
  int offset = 0;
  for (int i = 0; i < 3; i++) {
    // Modifier les registres pour call la bonne fonction
    if (args_ptr[i]) { // si le premier argument est un pointeur
      // On incremente le pointeur de pile
      regs.rsp -= args_fun[i];
      // On ajoute une valeur dans la pile
      // printf("regs.rsp : %p\n", regs.rsp);
      // char new_val[4] = {42, 0, 0, 0};
      char buffer[4] = {0, 0, 0, 0}; // inutile

      printf("Ecriture stack args_fun %d %d, offset %d\n", i, args_fun[i],
             offset);

      if (write_memory(regs.rsp, new_value_stack + offset, args_fun[i],
                       process_pid, buffer) == -1) {
        printf(
            "Erreur lors de l'appel de write_memory pour changer la stack\n");
        return -1;
      }
      *(registr[i]) = regs.rsp; // On pases en parametre un pointeur vers le
                                // sommet de la pile
      offset += args_fun[i];
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
  printf("pid: %d second trap\n", waitpid(process_pid, &status, 0));

  ptrace(PTRACE_GETREGS, process_pid, 0, &regs);
  printf("rax : %p\n", regs.rax);
  for (int i = 0; i < stack_push_size; i++) {
    char res;
    read_memory(regs.rsp + i * sizeof(char), 1, process_pid, &res);
    return_param[i] = res;
  }

  // Reecriture des lignes effacees
  // fwrite sauv_line addr : addr_puissance

  char buffer[sizeof(
      long long int)]; // inutile car stocke trap call trap (ancienne ligne)

  // printf("Réecriture des lignes\n");

  if (write_memory(addr_write_in_fun, sauv_line, length, process_pid, buffer) ==
      -1) {
    printf("Erreur lors de l'appel de write_memory pour restore\n");
    return -1;
  }
  sauv_regs.rip = sauv_regs.rip - 1;

  ptrace(PTRACE_SETREGS, process_pid, 0, &sauv_regs);

  ptrace(PTRACE_CONT, process_pid, 0, 0);
  printf("End of tct\n");
  return 0;
}

int main(int argc, char **argv) {
  printf("sizeof(int) = %d\n", sizeof(int));
  int process_pid = get_pid();
  long addr_puissance = get_adr_fun(argv[1]);
  long addr_puissance_opti = get_adr_fun(argv[2]);
  long addr_posix_memalign = get_adr_fun("posix_memalign");
  long addr_mprotect = get_adr_fun("mprotect");

  // 1. Attacher au processus cible
  if (ptrace(PTRACE_ATTACH, process_pid, 0, 0) == -1) {
    printf("ptrace_attach a echoue\n");
    return -1;
  }
  // {
  //   printf("--------------trap call trap addition\n");
  //   int status;
  //   printf("pid: %d\n", waitpid(process_pid, &status, 0));
  //   int size_code = 10;
  //   int args_fun[3] = {sizeof(int), 0, 0};
  //   bool args_ptr[3] = {true, false, false};

  //   // int args_fun[3] = {sizeof(int), 0, 0};
  //   // bool args_ptr[3] = {true, false, false};
  //   int stack_push_size = 0;
  //   for (int i = 0; i < 3; i++) {
  //     if (args_ptr[i]) {
  //       stack_push_size += args_fun[i];
  //     }
  //   }
  //   char *return_stack = malloc(stack_push_size);
  //   char new_value_stack[4] = {23, 0, 0, 0};

  //   // Tester le cas pthologique ou PC = adr ou PC = adr+1 ou Pc = adr+2
  //   trap_call_trap(addr_puissance, process_pid, addr_puissance_opti,
  //   args_fun,
  //                  args_ptr, new_value_stack, return_stack, stack_push_size);
  //   free(return_stack);
  // }
  int size_code = 113;
  char code[113] = {
      0x55, 0x48, 0x89, 0xe5, 0x48, 0x83, 0xec, 0x20, 0x89, 0x7d, 0xec, 0x89,
      0x75, 0xe8, 0x8b, 0x45, 0xec, 0x89, 0x45, 0xfc, 0x83, 0x7d, 0xe8, 0x00,
      0x79, 0x4d, 0x48, 0x8d, 0x05, 0xe0, 0x47, 0x07, 0x00, 0x48, 0x89, 0xc7,
      0xb8, 0x00, 0x00, 0x00, 0x00, 0xe8, 0x93, 0x2f, 0x00, 0x00, 0xb8, 0x01,
      0x00, 0x00, 0x00, 0xeb, 0x3b, 0x8b, 0x45, 0xe8, 0x83, 0xe0, 0x01, 0x85,
      0xc0, 0x75, 0x1a, 0x8b, 0x45, 0xfc, 0x0f, 0xaf, 0xc0, 0x89, 0x45, 0xfc,
      0x8b, 0x45, 0xe8, 0x89, 0xc2, 0xc1, 0xea, 0x1f, 0x01, 0xd0, 0xd1, 0xf8,
      0x89, 0x45, 0xe8, 0xeb, 0x0e, 0x8b, 0x45, 0xfc, 0x0f, 0xaf, 0x45, 0xec,
      0x89, 0x45, 0xfc, 0x83, 0x6d, 0xe8, 0x01, 0x83, 0x7d, 0xe8, 0x01, 0x7f,
      0xc8, 0x8b, 0x45, 0xfc, 0xc9};

  printf("--------------trap call trap posix meme align\n");

  int status;
  printf("pid: %d\n", waitpid(process_pid, &status, 0));

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
  char *new_value_stack = calloc(1, sizeof(long long int));

  // Tester le cas pathologique ou PC = adr ou PC = adr+1 ou Pc = adr+2
  if (trap_call_trap(addr_puissance, process_pid, addr_posix_memalign, args_fun,
                     args_ptr, new_value_stack, return_stack,
                     stack_push_size) == -1) {
    printf("Erreur dans trap call trap posix mem align\n");
    return -1;
  };

  long long int *addr_puissance_opti_heap = return_stack;

  printf("Pointer of the free memory %llx\n", *addr_puissance_opti_heap);

  printf("--------------trap call trap mprotect\n");

  printf("sizeof ptr: %d\n", sizeof(addr_puissance_opti));
  status;
  // printf("pid: %d\n", waitpid(process_pid, &status, 0));
  int args_fun_mprotect[3] = {sizeof(long long int), size_code,
                              PROT_EXEC | PROT_WRITE | PROT_READ};
  bool args_ptr_mprotect[3] = {true, false, false};

  // int args_fun[3] = {sizeof(int), 0, 0};
  // bool args_ptr[3] = {true, false, false};
  stack_push_size = 0;
  for (int i = 0; i < 3; i++) {
    if (args_ptr[i]) {
      stack_push_size += args_fun[i];
    }
  }
  char new_value_stack_mprotect[8] = {
      return_stack[0], return_stack[1], return_stack[2], return_stack[3],
      return_stack[4], return_stack[5], return_stack[6], return_stack[7]};

  // Tester le cas pathologique ou PC = adr ou PC = adr+1 ou Pc = adr+2
  if (trap_call_trap(addr_puissance, process_pid, addr_mprotect, args_fun,
                     args_ptr, new_value_stack_mprotect, return_stack,
                     stack_push_size) == -1) {
    printf("Erreur dans trap call trap mprotect\n");
    return -1;
  };

  // Ecriture du programme dans la heap
  char buffer[113];
  printf("Ecriture du programme dans la heap\n");
  if (write_memory(addr_puissance_opti_heap, code, size_code, process_pid,
                   buffer) == -1) {
    printf(
        "Erreur lors de l'appel de write_memory du programme dans la heap\n");
    return -1;
  }

  // Write in memory Jump to this programm (in the heap) in the function
  // puissance
  printf("Write Jump in puissance\n");
  char buffer_jump[10];
  printf("Ecriture du programme dans la heap\n");
  char jump_code[10] = {0x48,
                        0xb8,
                        return_stack[0],
                        return_stack[1],
                        return_stack[2],
                        return_stack[3],
                        return_stack[4],
                        return_stack[5],
                        return_stack[6],
                        return_stack[7]};
  if (write_memory(addr_puissance, jump_code, 10, process_pid, buffer_jump) ==
      -1) {
    printf(
        "Erreur lors de l'appel de write_memory du programme dans la heap\n");
    return -1;
  }

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

  free(return_stack);
}