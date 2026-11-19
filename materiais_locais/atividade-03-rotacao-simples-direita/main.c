#include "avl.h"
#include <stdio.h>

int main(void) {
    int valores[] = {50, 30, 70, 20, 40, 10};
    int i;
    AVL *avl = avl_criar();

    for (i = 0; i < 6; i++) {
        printf("\n=== INSERCAO DE %d ===\n", valores[i]);
        avl_inserir(avl, valores[i]);
        avl_imprimir(avl);
    }

    avl_apagar(&avl);
    return 0;
}
