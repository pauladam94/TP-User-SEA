# TP-User-SEA


- comment compiler pour fixer les adresses des fonctions et variables
```bash
gcc -no-pie main.c -o main
```

- Vérifier les processus en cours et récupérer le pid
```bash
ps -aux | grep "main"
```
- pid c'est le 2e, 6chiffres


- Kill un processus en cours avec le pid
```bash
kill -9 <pid>
```


# TODO
- automatiser recup pid
```bash
touch pid.txt
```
```C
system(ps -aux | grep "./main" | cut -c 12-17 | head -n 1 > pid.txt)
```
    - lire fichier.txt dans le programme
- automatiser recup addr func 

```bash
touch addr.txt
```
```C
system(nm main | grep "puissance" | cut -c 1-16 > addr.txt)
```
