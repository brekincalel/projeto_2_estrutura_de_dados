#include "avl.h"
#include <stdio.h>

static void executar(int valores[], int n, char nome[]) {
    int i;
    AVL *avl = avl_criar();
    printf("\n================ %s ================\n", nome);
    for (i = 0; i < n; i++) {
        printf("\n--- INSERCAO DE %d ---\n", valores[i]);
        avl_inserir(avl, valores[i]);
        avl_imprimir(avl);
    }
    avl_apagar(&avl);
}

int main(void) {
    int decrescente[] = {5, 4, 3, 2, 1};
    int crescente[] = {1, 2, 3, 4, 5};
    executar(decrescente, 5, "ROTACOES A DIREITA");
    executar(crescente, 5, "ROTACOES A ESQUERDA");
    return 0;
}
