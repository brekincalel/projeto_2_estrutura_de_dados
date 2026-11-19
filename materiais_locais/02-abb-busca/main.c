#include "abb.h"
#include <stdio.h>

int main(void) {
    int valores[] = {17, 99, 13, 1, 3, 100, 400};
    int consultas[] = {3, 100, 15, 500};
    ABB *t = abb_criar();

    for (int i = 0; i < 7; i++) abb_inserir(t, valores[i]);
    abb_imprimir(t);

    for (int i = 0; i < 4; i++)
        printf("buscar(%d) = %s\n", consultas[i],
               abb_buscar(t, consultas[i]) ? "encontrado" : "nao encontrado");

    abb_apagar(&t);
    return 0;
}
