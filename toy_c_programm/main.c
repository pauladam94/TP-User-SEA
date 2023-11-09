#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>

int puissance(int p, int n) {
    if (n < 0) {
        printf("n suppose to be positive");
        return EXIT_FAILURE;
    }
    int result = 1;
    for (int i = 0; i < n; i ++) {
        result = p * result;
    }
    return result;
}

int main(int argc, char **argv) {
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
    
    /*if (argc < 3) {
        printf("no enough arguments\n");
        return EXIT_FAILURE;
    }*/
    int p = 1000;//atoi(argv[1]);
    int n = 1000;//atoi(argv[2]);
    printf("Adresse de la puissance : %p\n", puissance);

    while(1){
        for (int i = 0; i<p;i++){
            for (int j = 0; j<n;j++){
                printf("%d ^ %d = %d\n", i, j, puissance(i, j));
            }
        }
    }
    
}
