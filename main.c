#include <stdio.h>
#include <sys/types.h>
#include <sys/ptrace.h>

int main() {
    printf("Hello, World!\n");
    
    int a = ptrace(PT_TRACE_ME, NULL ,NULL, 0  );
    return 0;
}
