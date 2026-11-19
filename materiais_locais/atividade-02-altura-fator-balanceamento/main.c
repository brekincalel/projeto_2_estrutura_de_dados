#include "avl.h"
#include <stdio.h>

int main(void) {
    int valores[] = {30, 20, 10};
    int i;
    AVL *avl = avl_criar();

    for (i = 0; i < 3; i++) {
        printf("\n=== INSERCAO DE %d ===\n", valores[i]);
        avl_inserir(avl, valores[i]);
        avl_imprimir(avl);
        printf("Balanceada? %s\n", avl_esta_balanceada(avl) ? "sim" : "nao");
    }

    avl_apagar(&avl);
    return 0;
}
