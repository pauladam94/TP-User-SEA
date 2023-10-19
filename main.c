#include <stdio.h>
#include <sys/types.h>
#include <sys/ptrace.h>

int main() {
    printf("Hello, World!\n");

    int process_pid = 0;
    int adr_fun =0;
    int a = ptrace(PT_TRACE_ME, NULL ,NULL, 0  );
    return 0;
    
    // 1. Attacher au processus cible
    ptrace(PTRACE_ATTACH, process_pid, 0, 0);

    // 2. Attendre que le processus cible atteigne la fonction spécifique
    while(1){
        int status = waitpid(process_pid, 0, 0);
        if (status == adr_fun) break;
    }
    
    // 3. Modifier dynamiquement le code
    // Remplacez la première instruction de la fonction par 0xCC (instruction de trap)
    int new_inst = 0xCC;
    write_memory(process_pid, adr_fun, new_inst);

    // 4. Continuer l'exécution du processus
    ptrace(PTRACE_CONT, process_pid, 0, 0);

    // 5. Détacher du processus
    ptrace(PTRACE_DETACH, process_pid, 0, 0);
}
