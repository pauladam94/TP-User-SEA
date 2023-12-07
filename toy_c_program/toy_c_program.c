#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <time.h>

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

void infinite(int p, int n){
    for (int i = 0; i<p;i++){
        for (int j = 0; j<n;j++){
            //printf("%d ^ %d = %d\n", i, j, puissance(i, j));
            puissance(i,j);
        }
    }
}

int* addition(int *t){//int p, int n){
    printf("addition, %d\n", *t);
    return t;
}

int main(int argc, char **argv) {
    prctl(PR_SET_PTRACER, PR_SET_PTRACER_ANY);
    
    /*if (argc < 3) {
        printf("no enough arguments\n");
        return EXIT_FAILURE;
    }*/
    int p = 2000;//atoi(argv[1]);
    int n = 2000;//atoi(argv[2]);
    //printf("Adresse de infinite : %p\n", infinite);

    while(1){        
        clock_t begin = clock();
        infinite(p,n);
        printf("done\n");
        clock_t end = clock();        
        double time_spent = (double) (end - begin) / CLOCKS_PER_SEC;
        printf("p = %d, n = %d, time spent = %f s\n", p, n, time_spent);
    }
    
}
