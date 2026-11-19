#include "avl.h"
#include <stdio.h>

static void inserir_lista(AVL *avl, int v[], int n) {
    int i;
    for (i = 0; i < n; i++) avl_inserir(avl, v[i]);
}

static void caso_direita(void) {
    int base[] = {30, 20, 40, 10, 25};
    AVL *avl = avl_criar();
    inserir_lista(avl, base, 5);
    printf("\n=== ANTES DE REMOVER 40 ===\n");
    avl_imprimir(avl);
    printf("\n=== REMOCAO DE 40 ===\n");
    avl_remover(avl, 40);
    avl_imprimir(avl);
    avl_apagar(&avl);
}

static void caso_esquerda(void) {
    int base[] = {30, 20, 40, 35, 50};
    AVL *avl = avl_criar();
    inserir_lista(avl, base, 5);
    printf("\n=== ANTES DE REMOVER 20 ===\n");
    avl_imprimir(avl);
    printf("\n=== REMOCAO DE 20 ===\n");
    avl_remover(avl, 20);
    avl_imprimir(avl);
    avl_apagar(&avl);
}

int main(void) {
    caso_direita();
    caso_esquerda();
    return 0;
}
