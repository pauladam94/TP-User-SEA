#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/user.h>

/*
Structure globale du programme tracé

// Calcule a^b en b multiplications
int puissance(int a, int b);

// Calcule a^b par exponentiation rapide
// L'objectif finale est d'écrire nous même
// cette fonction dans la heap
int puissance_opti(int a, int b);

int main() {
  while 1 {
    init = time();
    for i in 0..2000 {
      for j in 0..2000 {
        puissance(i, j);
      }
    }
    end = time();
    printf("%d", end - init);
    // Prend environ 7.5s pour a = 2000 et b = 2000
    // avec la fonction puissance non optimisé
  }
}
*/
/// Objectif Final du TP :
// Lancer puissance_opti au lieu de puissance
// Cette fonction est écrite dans la heap
// On va donc écrire dans la heap le code de puissance_opti
// et modifier la fonction puissance pour qu'elle appelle
// puissance_opti à la place de puissance

/// Convertit une chaine de caractère s en hexadécimal
long str_to_hexa(char *s)
{
  long result = 0;
  int c;
  int w = 0;
  while (w < strlen(s) - 1)
  {
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

/// Lis la mémoire d'un processus tracé (pid) à l'adresse adr
/// - buffer : pointeur vers la zone mémoire où on veut stocker le résultat
/// - pid : pid du processus tracé
/// - adr : addresse à laquelle on veut lire
/// - length : nombre d'octets à lire
/// > return : EXIT_FAILURE si erreur, EXIT_SUCCESS sinon
int read_memory(long adr, int length, int pid, char *buffer)
{
  char path[16];
  sprintf(path, "/proc/%d/mem", pid);
  FILE *f = fopen(path, "r+");
  if (f == NULL)
  {
    printf("Erreur dans l'ouverture du fichier memoire du processus\n");
    return EXIT_FAILURE;
  }
  if (fseek(f, adr, SEEK_SET) == -1)
  {
    printf("Erreur de placement dans le fichier\n");
    return EXIT_FAILURE;
  }
  if (fread(buffer, length, sizeof(char), f) == 0)
  {
    printf("Erreur de lecture dans le fichier\n");
    return EXIT_FAILURE;
  }
  if (fclose(f) != 0)
  {
    printf("Erreur de fermeture du fichier\n");
    return EXIT_FAILURE;
  };
  return EXIT_SUCCESS;
}

/// Ecrit dans la mémoire d'un processus tracé (pid) à l'adresse adr
/// - buffer : pointeur vers la zone mémoire où on veut stocker les lignes de
///            code effacées.
/// - pid : pid du processus tracé
/// - adr : addresse à laquelle on veut lire
/// - length : nombre d'octets à lire
/// - inst : instruction à écrire (de taille length)
/// > return : EXIT_FAILURE si erreur, EXIT_SUCCESS sinon
int write_memory(long adr, char *inst, int length, int pid, char *buffer)
{
  char path[16];
  // printf("length : %d\n", length);
  sprintf(path, "/proc/%d/mem", pid);
  printf("path : %s\n", path);
  printf("adr : 0x%X\n", adr);
  for (int i = 0; i < length; i++)
  {
    // Affichage de toutes les instructions à écrire
    // printf("inst : 0x%X\n", inst[i]);
  }
  FILE *f = fopen(path, "r+");
  if (f == NULL)
  {
    printf("Erreur dans l'ouverture du fichier memoire du processus\n");
    return EXIT_FAILURE;
  }
  if (fseek(f, adr, SEEK_SET) == -1)
  {
    printf("Erreur de placement dans le fichier\n");
    return EXIT_FAILURE;
  }
  if (fread(buffer, length, sizeof(char), f) == 0)
  {
    printf("Erreur de lecture dans le fichier\n");
    return EXIT_FAILURE;
  }
  for (int i = 0; i < length; i++)
  {
    // Affichage de toutes les instructions sauvegardées
    printf("Instruction %d sauvegardée : 0x%X\n", i, buffer[i]);
  }
  if (fseek(f, adr, SEEK_SET) == -1)
  {
    printf("Erreur de placement dans le fichier\n");
    return EXIT_FAILURE;
  }
  if (fwrite(inst, length, sizeof(char), f) == 0)
  {
    printf("Erreur d'ecriture dans le fichier\n");
    return EXIT_FAILURE;
  }
  if (fclose(f) != 0)
  {
    printf("Erreur de fermeture du fichier\n");
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

/// Récupère le pid du processus à tracer
/// - return : pid du processus à tracer si succès, EXIT_FAILURE sinon
int get_pid()
{
  system("ps -aux | grep \"./toy_c_program/toy_c_program\" | cut -c 12-17 | "
         "head -n 1 > pid.txt");
  FILE *f = fopen("pid.txt", "r");
  if (f == NULL)
  {
    printf("Erreur lors de la creation du fichier du pid\n");
    return EXIT_FAILURE;
  }
  char pid[100];
  if (fgets(pid, 100, f) == NULL)
  {
    printf("Erreur lors de la lecture du fichier du pid\n");
    return EXIT_FAILURE;
  }
  if (fclose(f) != 0)
  {
    printf("Erreur de fermeture du fichier\n");
    return EXIT_FAILURE;
  }
  system("rm pid.txt");
  printf("pid:%s", pid);
  printf("atoi(pid):%d\n", atoi(pid));
  return atoi(pid);
}

/// Récupère l'adresse de la fonction à modifier
/// - s : nom de la fonction
/// - return : adresse de la fonction si succès, EXIT_FAILURE sinon
long get_adr_fun(char *s)
{
  char sys_call[200];
  // On écrit dans un fichier temporaire l'adresse de la fonction
  sprintf(
      sys_call,
      "nm toy_c_program/toy_c_program | grep \"%s\" | cut -c 1-16 > adr.txt",
      s);
  printf("syscall : %s\n", sys_call);
  system(sys_call);

  FILE *f = fopen("adr.txt", "r");
  if (f == NULL)
  {
    printf("Erreur lors de la creation du fichier de l'adresse\n");
    return EXIT_FAILURE;
  }

  char adr[100];
  if (fgets(adr, 100, f) == NULL)
  {
    printf("Erreur lors de la lecture du fichier du pid\n");
    return EXIT_FAILURE;
  }
  if (fclose(f) != 0)
  {
    printf("Erreur de fermeture du fichier\n");
    return EXIT_FAILURE;
  }
  system("rm adr.txt");
  printf("adr:%s", adr);
  printf("hexa adr:%d\n", str_to_hexa(adr));
  return str_to_hexa(adr);
}

/// Fonction qui va modifier la fonction puissance pour qu'elle appelle
/// puissance_opti à la place de puissance
/// - addr_write_in_fun : adresse de la fonction à modifier
/// - process_pid : pid du processus à tracer
/// - addr_fun_call : adresse de la fonction à appeler
/// - args_fun : arguments de la fonction à appeler
/// - args_ptr : indique si les arguments sont des pointeurs
/// - new_value_stack : valeur à mettre dans la pile pour les valeurs
///                     pointées
/// - return_param : valeur de retour de la fonction appelée
/// - stack_push_size : taille de la pile à modifier (pour les paramètres
/// pointeurs)
/// > return : EXIT_FAILURE si erreur, EXIT_SUCCESS sinon
int trap_call_trap(long addr_write_in_fun, int process_pid, long addr_fun_call,
                   long long int *args_fun, bool *args_ptr,
                   char *new_value_stack, char *return_param,
                   int stack_push_size)
{
  int status;
  // Ecriture Trap Call Trap au début de la fonction puissance
  const int length = 4;
  char new_inst[4] = {0xCC, 0xFF, 0xD0, 0xCC};
  char sauv_line[4]; // 20 = taille max d'une instruction (x86 CrInGe)
  if (write_memory(addr_write_in_fun, new_inst, length, process_pid,
                   sauv_line) == EXIT_FAILURE)
  {
    printf("Erreur lors de l'appel de write_memory pour trap call trap\n");
    return EXIT_FAILURE;
  }

  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // Wait for the first trap
  printf("pid: %d first trap\n", waitpid(process_pid, &status, 0));

  // Recuperation de la valeur des registres
  struct user_regs_struct regs;
  struct user_regs_struct sauv_regs;
  ptrace(PTRACE_GETREGS, process_pid, 0, &sauv_regs);
  ptrace(PTRACE_GETREGS, process_pid, 0, &regs);

  long long int *registr[3] = {&(regs.rdi), &(regs.rsi), &(regs.rdx)};
  int offset = 0;
  for (int i = 0; i < 3; i++)
  {
    // Modifier les registres pour call la bonne fonction
    if (args_ptr[i])
    { // si le premier argument est un pointeur
      // On incremente le pointeur de pile
      regs.rsp -= args_fun[i];
      // On ajoute une valeur dans la pile
      // printf("regs.rsp : %p\n", regs.rsp);
      // char new_val[4] = {42, 0, 0, 0};
      char buffer[4] = {0, 0, 0, 0};

      printf("Ecriture stack args_fun %d %d, offset %d\n", i, args_fun[i],
             offset);
      if (write_memory(regs.rsp, new_value_stack + offset, args_fun[i],
                       process_pid, buffer) == -1)
      {
        printf(
            "Erreur lors de l'appel de write_memory pour changer la stack\n");
        return EXIT_FAILURE;
      }
      *(registr[i]) = regs.rsp; // On pases en parametre un pointeur vers le
                                // sommet de la pile
      offset += args_fun[i];
    }
    else
    {
      *(registr[i]) = args_fun[i];
    }
  }
  regs.rax = addr_fun_call;

  // Modifier les valeurs des registres
  ptrace(PTRACE_SETREGS, process_pid, 0, &regs);

  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // Wait for the second trap
  printf("pid: %d second trap\n", waitpid(process_pid, &status, 0));

  ptrace(PTRACE_GETREGS, process_pid, 0, &regs);
  printf("rax : %p\n", regs.rax);
  for (int i = 0; i < stack_push_size; i++)
  {
    char res;
    if (read_memory(regs.rsp + i * sizeof(char), 1, process_pid, &res) == EXIT_FAILURE)
    {
      printf("Erreur lors de l'appel de read_memory pour lire la stack\n");
      return EXIT_FAILURE;
    };
    return_param[i] = res;
  }

  // Reecriture des lignes effacees
  char buffer[sizeof(
      long long int)]; // stocke les anciennes valeurs des instructions (trap call trap)

  // printf("Réecriture des lignes\n");

  if (write_memory(addr_write_in_fun, sauv_line, length, process_pid, buffer) ==
      EXIT_FAILURE)
  {
    printf("Erreur lors de l'appel de write_memory pour restore\n");
    return EXIT_FAILURE;
  }
  sauv_regs.rip = sauv_regs.rip - 1;

  ptrace(PTRACE_SETREGS, process_pid, 0, &sauv_regs);

  ptrace(PTRACE_CONT, process_pid, 0, 0);
  printf("End of tct\n");
  return EXIT_SUCCESS;
}

int main(int argc, char **argv)
{
  printf("sizeof(int) = %d\n", sizeof(int));
  int process_pid = get_pid();
  long addr_puissance = get_adr_fun(argv[1]);
  long addr_puissance_opti = get_adr_fun(argv[2]);
  long addr_posix_memalign = get_adr_fun("posix_memalign");
  long addr_mprotect = get_adr_fun("__mprotect");

  printf("long addr_mprotect : %p", addr_mprotect);

  // 1. Attacher au processus cible
  if (ptrace(PTRACE_ATTACH, process_pid, 0, 0) == -1)
  {
    printf("ptrace_attach a echoue\n");
    return EXIT_FAILURE;
  }
  // {
  //   printf("--------------trap call trap addition\n");
  //   int status;
  //   printf("pid: %d\n", waitpid(process_pid, &status, 0));
  //   long long int args_fun[3] = {sizeof(int), 0, 0};
  //   bool args_ptr[3] = {true, false, false};
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

  long long int args_fun[3] = {sizeof(long long int), getpagesize(), size_code};
  bool args_ptr[3] = {true, false, false};

  // int args_fun[3] = {sizeof(int), 0, 0};
  // bool args_ptr[3] = {true, false, false};
  int stack_push_size = 0;
  for (int i = 0; i < 3; i++)
  {
    if (args_ptr[i])
    {
      stack_push_size += args_fun[i];
    }
  }
  char *return_stack = malloc(stack_push_size);
  char *new_value_stack = calloc(1, sizeof(long long int));

  // Tester le cas pathologique ou PC = adr ou PC = adr+1 ou Pc = adr+2
  if (trap_call_trap(addr_puissance, process_pid, addr_posix_memalign, args_fun,
                     args_ptr, new_value_stack, return_stack,
                     stack_push_size) == EXIT_FAILURE)
  {
    printf("Erreur dans trap call trap posix mem align\n");
    return EXIT_FAILURE;
  };

  long long int *addr_puissance_opti_heap = return_stack;

  printf("Pointer of the free memory %llx\n", *addr_puissance_opti_heap);

  printf("--------------trap call trap mprotect\n");

  char return_stack_mprotect[4] = {0, 0, 0, 0};

  printf("sizeof ptr: %d\n", sizeof(addr_puissance_opti));
  status;
  // printf("pid: %d\n", waitpid(process_pid, &status, 0));
  long long int args_fun_mprotect[3] = {(long long int)return_stack, size_code,
                                        PROT_EXEC | PROT_WRITE | PROT_READ};
  bool args_ptr_mprotect[3] = {false, false, false};

  // int args_fun[3] = {sizeof(int), 0, 0};
  // bool args_ptr[3] = {true, false, false};
  stack_push_size = 0;
  for (int i = 0; i < 3; i++)
  {
    if (args_ptr[i])
    {
      stack_push_size += args_fun[i];
    }
  }
  char new_value_stack_mprotect[8] = {0, 0, 0, 0, 0, 0, 0, 0};
  // char new_value_stack_mprotect[8] = {
  //     return_stack[0], return_stack[1], return_stack[2], return_stack[3],
  //     return_stack[4], return_stack[5], return_stack[6], return_stack[7]};

  // Tester le cas pathologique ou PC = adr ou PC = adr+1 ou Pc = adr+2
  if (trap_call_trap(addr_puissance, process_pid, addr_mprotect, args_fun,
                     args_ptr, new_value_stack_mprotect, return_stack_mprotect,
                     stack_push_size) == EXIT_FAILURE)
  {
    printf("Erreur dans trap call trap mprotect\n");
    return EXIT_FAILURE;
  };

  // Ecriture du programme dans la heap
  char buffer[113];
  printf("Ecriture du programme dans la heap\n");
  if (write_memory(addr_puissance_opti_heap, code, size_code, process_pid,
                   buffer) == EXIT_FAILURE)
  {
    printf(
        "Erreur lors de l'appel de write_memory du programme dans la heap\n");
    return EXIT_FAILURE;
  }

  // Write in memory Jump to this programm (in the heap)
  // in the function puissance that is called very often
  printf("Write Jump in puissance\n");
  char buffer_jump[12];
  printf("Ecriture du programme dans la heap\n");
  char jump_code[12] = {0x48,
                        0xb8,
                        return_stack[0],
                        return_stack[1],
                        return_stack[2],
                        return_stack[3],
                        return_stack[4],
                        return_stack[5],
                        return_stack[6],
                        return_stack[7],
                        0xff,
                        0xe0};
  if (write_memory(addr_puissance, jump_code, 12, process_pid, buffer_jump) ==
      EXIT_FAILURE)
  {
    printf(
        "Erreur lors de l'appel de write_memory du programme dans la heap\n");
    return EXIT_FAILURE;
  }

  // Continuer l'éxécution du processus avec le nouveau code
  ptrace(PTRACE_CONT, process_pid, 0, 0);

  // Détacher le processus
  ptrace(PTRACE_DETACH, process_pid, 0, 0);

  free(return_stack);
  return EXIT_SUCCESS;
}