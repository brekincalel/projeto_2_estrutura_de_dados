#include "abb.h"
#include <stdio.h>

static void carregar(ABB *t, const int *v, int n) {
    for (int i = 0; i < n; i++) abb_inserir(t, v[i]);
}

static void resumo(const char *nome, ABB *t) {
    printf("\n%s\n", nome);
    abb_imprimir(t);
    printf("altura = %d\n", abb_altura(t));
    printf("balanceada = %s\n", abb_balanceada(t) ? "sim" : "nao");
    printf("em-ordem = "); abb_em_ordem(t);
}

int main(void) {
    int ordenada[] = {10, 20, 30, 40, 50, 60, 70};
    int melhor[]   = {40, 20, 60, 10, 30, 50, 70};

    ABB *a = abb_criar();
    ABB *b = abb_criar();
    carregar(a, ordenada, 7);
    carregar(b, melhor, 7);

    resumo("Insercao ordenada", a);
    resumo("Insercao pelo elemento central", b);

    abb_apagar(&a);
    abb_apagar(&b);
    return 0;
}
