#include "avl.h"
#include <stdio.h>
#include <stdlib.h>

static void inserir_chave(AVL *avl, int chave) {
    ITEM *item = item_criar(chave, NULL);

    printf("\nInserindo %d\n", chave);

    if (item == NULL || !avl_inserir(avl, item)) {
        item_apagar(&item);
        printf("Nao foi possivel inserir %d.\n", chave);
        return;
    }

    avl_imprimir(avl);
}

static void remover_chave(AVL *avl, int chave) {
    ITEM *item;

    printf("\nRemovendo %d\n", chave);
    item = avl_remover(avl, chave);

    if (item == NULL) {
        printf("Chave %d nao encontrada.\n", chave);
        return;
    }

    item_apagar(&item);
    avl_imprimir(avl);
}

static void exemplo_esquerda_direita(void) {
    AVL *avl = avl_criar();

    printf("\n========================================\n");
    printf("ROTACAO DUPLA ESQUERDA/DIREITA\n");
    printf("Sequencia: 30, 10, 20\n");
    printf("========================================\n");

    inserir_chave(avl, 30);
    inserir_chave(avl, 10);
    inserir_chave(avl, 20);

    avl_apagar(&avl);
}

static void exemplo_direita_esquerda(void) {
    AVL *avl = avl_criar();

    printf("\n========================================\n");
    printf("ROTACAO DUPLA DIREITA/ESQUERDA\n");
    printf("Sequencia: 10, 30, 20\n");
    printf("========================================\n");

    inserir_chave(avl, 10);
    inserir_chave(avl, 30);
    inserir_chave(avl, 20);

    avl_apagar(&avl);
}

static void exemplo_remocao_com_rotacao_dupla(void) {
    int chaves[] = {20, 10, 40, 5, 30, 50, 25};
    int n = (int) (sizeof(chaves) / sizeof(chaves[0]));
    int i;
    AVL *avl = avl_criar();

    printf("\n========================================\n");
    printf("REMOCAO QUE EXIGE ROTACAO DUPLA\n");
    printf("========================================\n");

    for (i = 0; i < n; i++)
        inserir_chave(avl, chaves[i]);

    remover_chave(avl, 5);

    avl_apagar(&avl);
}

int main(void) {
    exemplo_esquerda_direita();
    exemplo_direita_esquerda();
    exemplo_remocao_com_rotacao_dupla();
    return EXIT_SUCCESS;
}
