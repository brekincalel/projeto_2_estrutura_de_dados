#include "abb.h"
#include <stdio.h>
#include <stdlib.h>

struct no_ {
    int chave;
    NO *esq;
    NO *dir;
};

struct abb_ {
    NO *raiz;
};

static NO *criar_no(int chave) {
    NO *n = malloc(sizeof(NO));
    if (n == NULL) return NULL;
    n->chave = chave;
    n->esq = NULL;
    n->dir = NULL;
    return n;
}

ABB *abb_criar(void) {
    ABB *t = malloc(sizeof(ABB));
    if (t != NULL) t->raiz = NULL;
    return t;
}

static void apagar_no(NO *r) {
    if (r == NULL) return;
    apagar_no(r->esq);
    apagar_no(r->dir);
    free(r);
}

void abb_apagar(ABB **t) {
    if (t == NULL || *t == NULL) return;
    apagar_no((*t)->raiz);
    free(*t);
    *t = NULL;
}

static NO *inserir_no(NO *r, NO *novo) {
    if (r == NULL) return novo;
    if (novo->chave < r->chave)
        r->esq = inserir_no(r->esq, novo);
    else if (novo->chave > r->chave)
        r->dir = inserir_no(r->dir, novo);
    else
        free(novo);
    return r;
}

bool abb_inserir(ABB *t, int chave) {
    if (t == NULL) return false;
    NO *novo = criar_no(chave);
    if (novo == NULL) return false;
    t->raiz = inserir_no(t->raiz, novo);
    return true;
}

static void imprimir_no(const NO *r, int nivel) {
    if (r == NULL) return;
    imprimir_no(r->dir, nivel + 1);
    for (int i = 0; i < nivel; i++) printf("    ");
    printf("%d\n", r->chave);
    imprimir_no(r->esq, nivel + 1);
}

void abb_imprimir(const ABB *t) {
    if (t == NULL) return;
    imprimir_no(t->raiz, 0);
}

static bool buscar_no(const NO *r, int chave) {
    if (r == NULL) return false;
    if (chave == r->chave) return true;
    if (chave < r->chave) return buscar_no(r->esq, chave);
    return buscar_no(r->dir, chave);
}

bool abb_buscar(const ABB *t, int chave) {
    return t != NULL && buscar_no(t->raiz, chave);
}

static void em_ordem_no(const NO *r) {
    if (r == NULL) return;
    em_ordem_no(r->esq);
    printf("%d ", r->chave);
    em_ordem_no(r->dir);
}

static void pre_ordem_no(const NO *r) {
    if (r == NULL) return;
    printf("%d ", r->chave);
    pre_ordem_no(r->esq);
    pre_ordem_no(r->dir);
}

static void pos_ordem_no(const NO *r) {
    if (r == NULL) return;
    pos_ordem_no(r->esq);
    pos_ordem_no(r->dir);
    printf("%d ", r->chave);
}

void abb_em_ordem(const ABB *t) { if (t) { em_ordem_no(t->raiz); printf("\n"); } }
void abb_pre_ordem(const ABB *t) { if (t) { pre_ordem_no(t->raiz); printf("\n"); } }
void abb_pos_ordem(const ABB *t) { if (t) { pos_ordem_no(t->raiz); printf("\n"); } }

static NO *maior(NO *r) {
    while (r->dir != NULL) r = r->dir;
    return r;
}

static NO *remover_no(NO *r, int chave, bool *removeu) {
    if (r == NULL) return NULL;

    if (chave < r->chave) {
        r->esq = remover_no(r->esq, chave, removeu);
    } else if (chave > r->chave) {
        r->dir = remover_no(r->dir, chave, removeu);
    } else {
        *removeu = true;
        if (r->esq == NULL) {
            NO *dir = r->dir;
            free(r);
            return dir;
        }
        if (r->dir == NULL) {
            NO *esq = r->esq;
            free(r);
            return esq;
        }

        NO *pred = maior(r->esq);
        r->chave = pred->chave;
        bool ignorar = false;
        r->esq = remover_no(r->esq, pred->chave, &ignorar);
    }
    return r;
}

bool abb_remover(ABB *t, int chave) {
    if (t == NULL) return false;
    bool removeu = false;
    t->raiz = remover_no(t->raiz, chave, &removeu);
    return removeu;
}
