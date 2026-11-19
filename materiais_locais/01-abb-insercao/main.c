#include "abb.h"
#include <stdio.h>

int main(void) {
    int valores[] = {17, 99, 13, 1, 3, 100, 400};
    int n = sizeof(valores) / sizeof(valores[0]);
    ABB *t = abb_criar();

    for (int i = 0; i < n; i++) {
        printf("\nInserindo %d\n", valores[i]);
        abb_inserir(t, valores[i]);
        abb_imprimir(t);
    }

    abb_apagar(&t);
    return 0;
}
