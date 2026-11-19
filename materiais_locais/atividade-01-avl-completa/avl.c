#include "avl.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;

struct no_ {
    int chave;
    ITEM *item;
    int altura;
    NO *esq;
    NO *dir;
};

struct avl_ {
    NO *raiz;
    int tamanho;
};

static int maximo(int a, int b) {
    return a > b ? a : b;
}

/* Nesta disciplina: NULL tem altura 0 e folha tem altura 1. */
static int altura_no(const NO *no) {
    return no == NULL ? 0 : no->altura;
}

static int fator_balanceamento(const NO *no) {
    if (no == NULL)
        return 0;

    return altura_no(no->esq) - altura_no(no->dir);
}

static void atualizar_altura(NO *no) {
    if (no != NULL)
        no->altura = 1 + maximo(altura_no(no->esq), altura_no(no->dir));
}

static NO *no_criar(ITEM *item) {
    NO *novo;

    if (item == NULL)
        return NULL;

    novo = (NO *) malloc(sizeof(NO));
    if (novo == NULL)
        return NULL;

    novo->chave = item_get_chave(item);
    novo->item = item;
    novo->altura = 1;
    novo->esq = NULL;
    novo->dir = NULL;

    return novo;
}

static NO *rotacao_direita(NO *a) {
    NO *b = a->esq;
    NO *t2 = b->dir;

    printf("      rotacao direita em A=%d\n", a->chave);

    b->dir = a;
    a->esq = t2;

    atualizar_altura(a);
    atualizar_altura(b);

    return b;
}

static NO *rotacao_esquerda(NO *a) {
    NO *b = a->dir;
    NO *t2 = b->esq;

    printf("      rotacao esquerda em A=%d\n", a->chave);

    b->esq = a;
    a->dir = t2;

    atualizar_altura(a);
    atualizar_altura(b);

    return b;
}

/* Esquerda/direita: primeiro gira o filho esquerdo para a esquerda. */
static NO *rotacao_esquerda_direita(NO *a) {
    printf("      passo 1: rotacao esquerda em B=%d\n", a->esq->chave);
    a->esq = rotacao_esquerda(a->esq);
    printf("      passo 2: rotacao direita em A=%d\n", a->chave);
    return rotacao_direita(a);
}

/* Direita/esquerda: primeiro gira o filho direito para a direita. */
static NO *rotacao_direita_esquerda(NO *a) {
    printf("      passo 1: rotacao direita em B=%d\n", a->dir->chave);
    a->dir = rotacao_direita(a->dir);
    printf("      passo 2: rotacao esquerda em A=%d\n", a->chave);
    return rotacao_esquerda(a);
}

static NO *rebalancear(NO *a) {
    int fb_a;
    int fb_b;

    if (a == NULL)
        return NULL;

    atualizar_altura(a);
    fb_a = fator_balanceamento(a);

    if (fb_a == 2) {
        fb_b = fator_balanceamento(a->esq);
        printf("  >>> GATILHO: A=%d FB(A)=%+d, B=%d FB(B)=%+d\n",
               a->chave, fb_a, a->esq->chave, fb_b);

        if (fb_b >= 0) {
            printf("      sinais compativeis -> rotacao simples direita\n");
            return rotacao_direita(a);
        }

        printf("      sinais diferentes -> rotacao dupla esquerda/direita\n");
        return rotacao_esquerda_direita(a);
    }

    if (fb_a == -2) {
        fb_b = fator_balanceamento(a->dir);
        printf("  >>> GATILHO: A=%d FB(A)=%+d, B=%d FB(B)=%+d\n",
               a->chave, fb_a, a->dir->chave, fb_b);

        if (fb_b <= 0) {
            printf("      sinais compativeis -> rotacao simples esquerda\n");
            return rotacao_esquerda(a);
        }

        printf("      sinais diferentes -> rotacao dupla direita/esquerda\n");
        return rotacao_direita_esquerda(a);
    }

    return a;
}

static NO *inserir_no(NO *raiz, ITEM *item, bool *inseriu) {
    int chave = item_get_chave(item);

    if (raiz == NULL) {
        NO *novo = no_criar(item);
        *inseriu = novo != NULL;
        return novo;
    }

    if (chave < raiz->chave)
        raiz->esq = inserir_no(raiz->esq, item, inseriu);
    else if (chave > raiz->chave)
        raiz->dir = inserir_no(raiz->dir, item, inseriu);
    else {
        *inseriu = false;
        return raiz;
    }

    if (!*inseriu)
        return raiz;

    return rebalancear(raiz);
}

static ITEM *buscar_no(const NO *raiz, int chave) {
    if (raiz == NULL)
        return NULL;

    if (chave == raiz->chave)
        return raiz->item;

    if (chave < raiz->chave)
        return buscar_no(raiz->esq, chave);

    return buscar_no(raiz->dir, chave);
}

static NO *menor_no(NO *raiz) {
    while (raiz != NULL && raiz->esq != NULL)
        raiz = raiz->esq;

    return raiz;
}

static NO *remover_no(NO *raiz, int chave, ITEM **removido, bool *removeu) {
    if (raiz == NULL) {
        *removeu = false;
        return NULL;
    }

    if (chave < raiz->chave) {
        raiz->esq = remover_no(raiz->esq, chave, removido, removeu);
    } else if (chave > raiz->chave) {
        raiz->dir = remover_no(raiz->dir, chave, removido, removeu);
    } else {
        NO *aux;

        *removeu = true;

        if (raiz->esq == NULL || raiz->dir == NULL) {
            aux = raiz->esq != NULL ? raiz->esq : raiz->dir;
            *removido = raiz->item;
            free(raiz);
            return aux;
        }

        /*
         * Com dois filhos, troca-se o conteúdo com o sucessor.
         * Em seguida, a chave original é removida da subárvore direita.
         */
        aux = menor_no(raiz->dir);

        {
            int chave_tmp = raiz->chave;
            ITEM *item_tmp = raiz->item;

            raiz->chave = aux->chave;
            raiz->item = aux->item;
            aux->chave = chave_tmp;
            aux->item = item_tmp;
        }

        raiz->dir = remover_no(raiz->dir, chave, removido, removeu);
    }

    if (!*removeu)
        return raiz;

    return rebalancear(raiz);
}

static void apagar_no(NO *raiz) {
    if (raiz == NULL)
        return;

    apagar_no(raiz->esq);
    apagar_no(raiz->dir);
    item_apagar(&raiz->item);
    free(raiz);
}

static void imprimir_no(const NO *raiz, int nivel) {
    int i;

    if (raiz == NULL)
        return;

    imprimir_no(raiz->dir, nivel + 1);

    for (i = 0; i < nivel; i++)
        printf("        ");

    printf("%d [h=%d FB=%+d]\n",
           raiz->chave, raiz->altura, fator_balanceamento(raiz));

    imprimir_no(raiz->esq, nivel + 1);
}

AVL *avl_criar(void) {
    AVL *avl = (AVL *) malloc(sizeof(AVL));

    if (avl != NULL) {
        avl->raiz = NULL;
        avl->tamanho = 0;
    }

    return avl;
}

void avl_apagar(AVL **avl) {
    if (avl == NULL || *avl == NULL)
        return;

    apagar_no((*avl)->raiz);
    free(*avl);
    *avl = NULL;
}

bool avl_inserir(AVL *avl, ITEM *item) {
    bool inseriu = false;

    if (avl == NULL || item == NULL)
        return false;

    avl->raiz = inserir_no(avl->raiz, item, &inseriu);

    if (inseriu)
        avl->tamanho++;

    return inseriu;
}

ITEM *avl_buscar(const AVL *avl, int chave) {
    if (avl == NULL)
        return NULL;

    return buscar_no(avl->raiz, chave);
}

ITEM *avl_remover(AVL *avl, int chave) {
    ITEM *removido = NULL;
    bool removeu = false;

    if (avl == NULL)
        return NULL;

    avl->raiz = remover_no(avl->raiz, chave, &removido, &removeu);

    if (removeu)
        avl->tamanho--;

    return removido;
}

bool avl_vazia(const AVL *avl) {
    return avl == NULL || avl->tamanho == 0;
}

int avl_tamanho(const AVL *avl) {
    return avl == NULL ? 0 : avl->tamanho;
}

int avl_altura(const AVL *avl) {
    return avl == NULL ? 0 : altura_no(avl->raiz);
}

void avl_imprimir(const AVL *avl) {
    if (avl == NULL || avl->raiz == NULL) {
        printf("<arvore vazia>\n");
        return;
    }

    imprimir_no(avl->raiz, 0);
}
