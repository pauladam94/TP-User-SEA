#include <stdio.h>
#include <sys/types.h>
#include <sys/ptrace.h>

int main() {
    printf("Hello, World!\n");
    
    int a = ptrace(PT_TRACE_ME, NULL ,NULL, 0  );
    return 0;
    
    // 1. Attacher au processus cible
    ptrace(PTRACE_ATTACH, process_pid, 0, 0)

    // 2. Attendre que le processus cible atteigne la fonction spécifique
    while True:
        status = waitpid(process_pid, 0, 0)
    if status indique que le processus a atteint la fonction spécifique:
    break
    
    // 3. Modifier dynamiquement le code
    // Remplacez la première instruction de la fonction par 0xCC (instruction de trap)
    int nouvelle_instruction = 0xCC;
    write_memory(process_pid, adresse_de_la_fonction, nouvelle_instruction);

    // 4. Continuer l'exécution du processus
    ptrace(PTRACE_CONT, process_pid, 0, 0);

    // 5. Détacher du processus
    ptrace(PTRACE_DETACH, process_pid, 0, 0);
}
