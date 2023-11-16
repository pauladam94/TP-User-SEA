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





on peut pas changer le rip(PC) comme des bourrins parce que ça nique les ret

quand on est le tracant d'un tracé on récupère tous les signaux -> notamment les sig_trap qui sont lancés par l'instruction trap

le tricks d'écrire 
trap call
trap

dans un bout de code qui s'éxécute souvent
on va ensuite faire waitpid pour le premier trap 
onn récup les registre avec ptrace(pgetregs)
(la fonction s'éxécute)
on attend le deuxième trap pour arreter de nouveau la fonction
on verifie le retour de la fonction (on peit meme l'utiliser)
on remet les registres précedent sauf ceux qu'on a modif (peut etre faire des dingueries avec rip)
réecrit les lignes qu'on a utilisé.



potentiellement on veut aprtir direct de la fonction non optimisé (infinite)


On remet les registres precedents (prtrace(PSETREGS, ...))
while(1){

    // arretere le programme a un endroit random
    waitpid()
    // Ecriture de trap call / trap / ret dans un endroit execute regulierement et souvenir des lignes overwrite
    waitpid du 1e trap
    recup des registres + set args du call
    ptrace cont
    waitpid du 2e trap
    verif du retour
    reecrire les bonnes lignes /?\ Sortir de la fonction /?\
    
}