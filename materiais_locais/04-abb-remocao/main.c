#include "abb.h"
#include <stdio.h>

static void remover_e_mostrar(ABB *t, int chave) {
    printf("\nRemovendo %d: %s\n", chave, abb_remover(t, chave) ? "ok" : "nao encontrado");
    abb_imprimir(t);
    printf("em-ordem: "); abb_em_ordem(t);
}

int main(void) {
    int valores[] = {30, 20, 70, 25, 50, 90, 60, 10};
    ABB *t = abb_criar();
    for (int i = 0; i < 8; i++) abb_inserir(t, valores[i]);

    printf("Arvore inicial:\n");
    abb_imprimir(t);
    remover_e_mostrar(t, 10);  /* folha */
    remover_e_mostrar(t, 50);  /* um filho */
    remover_e_mostrar(t, 30);  /* dois filhos */

    abb_apagar(&t);
    return 0;
}
