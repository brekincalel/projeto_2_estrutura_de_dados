#include "avl.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

struct no_ {
    int chave;
    NO *esq;
    NO *dir;
};

struct avl_ {
    NO *raiz;
};

static NO *no_criar(int chave) {
    NO *novo = (NO *) malloc(sizeof(NO));
    if (novo == NULL) return NULL;
    novo->chave = chave;
    novo->esq = NULL;
    novo->dir = NULL;
    return novo;
}

static NO *inserir_no(NO *raiz, int chave, int *ok) {
    if (raiz == NULL) {
        NO *novo = no_criar(chave);
        *ok = (novo != NULL);
        return novo;
    }

    if (chave < raiz->chave)
        raiz->esq = inserir_no(raiz->esq, chave, ok);
    else if (chave > raiz->chave)
        raiz->dir = inserir_no(raiz->dir, chave, ok);
    else
        *ok = 0;

    return raiz;
}

static void imprimir_no(NO *raiz, int nivel) {
    int i;
    if (raiz == NULL) return;
    imprimir_no(raiz->dir, nivel + 1);
    for (i = 0; i < nivel; i++) printf("        ");
    printf("%d\n", raiz->chave);
    imprimir_no(raiz->esq, nivel + 1);
}

static void apagar_no(NO *raiz) {
    if (raiz == NULL) return;
    apagar_no(raiz->esq);
    apagar_no(raiz->dir);
    free(raiz);
}

AVL *avl_criar(void) {
    AVL *avl = (AVL *) malloc(sizeof(AVL));
    if (avl != NULL) avl->raiz = NULL;
    return avl;
}

int avl_inserir(AVL *avl, int chave) {
    int ok = 0;
    if (avl == NULL) return 0;
    avl->raiz = inserir_no(avl->raiz, chave, &ok);
    return ok;
}

void avl_imprimir(AVL *avl) {
    if (avl == NULL || avl->raiz == NULL) {
        printf("<arvore vazia>\n");
        return;
    }
    imprimir_no(avl->raiz, 0);
}

void avl_apagar(AVL **avl) {
    if (avl == NULL || *avl == NULL) return;
    apagar_no((*avl)->raiz);
    free(*avl);
    *avl = NULL;
}
