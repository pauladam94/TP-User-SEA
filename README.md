# TP-User-SEA
## Lancer le projet

- `make tracant` pour compiler le traçant
- `make trace` pour compiler le tracé
- `make full` pour lancer `make trace` et `make tracant`
- `make clean` pour clean les fichiers de build
- `make run_tracant` pour lancer le traçant
- `make run_trace` pour lancer le tracé

Il faut toujours lancer le tracé avant le traçant.

## Option de Compilation

Toutes les options de compilation utilisées sont écrites dans le makefile.

## Config sur lequel tout le TP a été fait

```bash
OS: Manjaro Linux x86_64
Kernel: 5.15.131-1-MANJARO 
Packages: 1467 (pacman), 6 (flatpak), 4 (snap) 
Shell: bash 5.1.16 
DE: Plasma 5.27.8 
WM: KWin 
CPU: AMD Ryzen 5 5500U with Radeon Graphics (12) @ 2.100GHz 
GPU: AMD ATI 04:00.0 Lucienne 
Memory: 15315MiB 
```

## Ce qui a été fait

### Challenge 1

S'attacher à un processus et écrire un trap qui arrête
l'éxécution du tracé.

Etat : FAIT

### Challenge 2

Appel de toute fonction qui est déjà écrite dans le tracé (nous compilons en statique, nous avons donc accès dans le code du programme à toutes les fonctions de la libc qui est importée).

Ceci a été fait pour fonctions qui prennent en paramètre tout type et renvoie tout type. En effet nous supportons les paramètres pointeurs. Ces pointeurs sont des pointeurs vers la stack. Nous avons préalablement décalé le pointeur de stack et avons écrit une valeur initiale de cet emplacement de la stack.

Toutes les fonctions qui ont été testé :
- `posix_memalign`
- `mprotect`
- `int foo(int, int)`
- `int* foo(int, int)`
- `int* foo(int*, int*)`

Etat : FAIT

### Challenge 3

Appel à Posix_memalign avec ce qui a été fait dans le challenge 2.
Appel à m_protect avec ce qui a été fait dans le challenge 2.

Etat : FAIT

### Challenge 4

- Avec l'appel à Posix_memalign, on alloue la place nécéssaire pour écrire une fonction custom dans la heap. 
- On rend cette partie de la heap exécutable avec l'appel à m_protect.
- Ecriture de la fonction dans cette endroit de la heap

> Tout ceci a été fait, vérifié et semble fonctionner.

Ecriture du Jump vers cette fonction dans la heap dans le programme tracé.

> Ceci ce fonctionne pas. Quelque soit la fonction écrite dans la heap.

Etat : Non fonctionnel lorsqu'on écrit le jump vers notre fonction écrite dans la heap. Le tracé renvoie l'erreur "segfault". Ce problème n'a pas été résolu.