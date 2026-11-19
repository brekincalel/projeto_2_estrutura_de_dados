#include "avl.h"
#include <stdio.h>

int main(void) {
    int valores[] = {10, 20, 30, 40, 50};
    int n = 5;
    int i;
    AVL *avl = avl_criar();

    for (i = 0; i < n; i++) {
        printf("\nInserindo %d\n", valores[i]);
        avl_inserir(avl, valores[i]);
        avl_imprimir(avl);
    }

    avl_apagar(&avl);
    return 0;
}
