#include "avl.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

struct no_ {
    int chave;
    int altura;
    NO *esq;
    NO *dir;
};

struct avl_ {
    NO *raiz;
};

static int maximo(int a, int b) { return a > b ? a : b; }
static int altura_no(NO *no) { return no == NULL ? 0 : no->altura; }
static int fb_no(NO *no) { return no == NULL ? 0 : altura_no(no->esq) - altura_no(no->dir); }

static void atualizar_altura(NO *no) {
    if (no != NULL)
        no->altura = 1 + maximo(altura_no(no->esq), altura_no(no->dir));
}

static NO *no_criar(int chave) {
    NO *novo = (NO *) malloc(sizeof(NO));
    if (novo == NULL) return NULL;
    novo->chave = chave;
    novo->altura = 1;
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
    else {
        *ok = 0;
        return raiz;
    }

    atualizar_altura(raiz);
    printf("Retorno em %d: h(esq)=%d h(dir)=%d FB=%+d\n",
           raiz->chave, altura_no(raiz->esq), altura_no(raiz->dir), fb_no(raiz));

    if (fb_no(raiz) > 1 || fb_no(raiz) < -1)
        printf(">>> GATILHO DETECTADO em %d: |FB| = %d. Ainda nao corrigimos.\n",
               raiz->chave, fb_no(raiz) > 0 ? fb_no(raiz) : -fb_no(raiz));

    return raiz;
}

static int balanceada_no(NO *raiz) {
    int f;
    if (raiz == NULL) return 1;
    f = fb_no(raiz);
    if (f < -1 || f > 1) return 0;
    return balanceada_no(raiz->esq) && balanceada_no(raiz->dir);
}

static void imprimir_no(NO *raiz, int nivel) {
    int i;
    if (raiz == NULL) return;
    imprimir_no(raiz->dir, nivel + 1);
    for (i = 0; i < nivel; i++) printf("        ");
    printf("%d [h=%d FB=%+d]\n", raiz->chave, raiz->altura, fb_no(raiz));
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

int avl_esta_balanceada(AVL *avl) {
    if (avl == NULL) return 0;
    return balanceada_no(avl->raiz);
}

void avl_apagar(AVL **avl) {
    if (avl == NULL || *avl == NULL) return;
    apagar_no((*avl)->raiz);
    free(*avl);
    *avl = NULL;
}
