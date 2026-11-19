#include "avl.h"
#include <stdio.h>

int main(void) {
    int valores[] = {30, 10, 50, 40, 60, 70};
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
