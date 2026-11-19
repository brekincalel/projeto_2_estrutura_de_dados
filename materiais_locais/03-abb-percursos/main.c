#include "abb.h"
#include <stdio.h>

int main(void) {
    int valores[] = {27, 90, 20, 10, 100, 25, 15, 500};
    ABB *t = abb_criar();
    for (int i = 0; i < 8; i++) abb_inserir(t, valores[i]);

    abb_imprimir(t);
    printf("em-ordem : "); abb_em_ordem(t);
    printf("pre-ordem: "); abb_pre_ordem(t);
    printf("pos-ordem: "); abb_pos_ordem(t);

    abb_apagar(&t);
    return 0;
}
