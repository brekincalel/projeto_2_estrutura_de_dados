#include "rubro_negra.h"

#include <stdint.h>
#include <stdlib.h>

typedef enum { PRETO, VERMELHO } COR;
typedef struct no NO;

struct no {
    CHAVE_ARVORE chave;
    LISTA_OFFSETS offsets;
    COR cor;
    NO *esquerda;
    NO *direita;
    NO *pai;
};

struct rubro_negra {
    NO *raiz;
    NO nulo;
    size_t tamanho;
    METRICAS_ARVORE metricas;
};

static void definir_cor(RUBRO_NEGRA *arvore, NO *no, COR cor) {
    if (no->cor != cor) {
        no->cor = cor;
        ++arvore->metricas.alteracoes_cor;
    }
}

static void rotacao_esquerda(RUBRO_NEGRA *arvore, NO *a) {
    NO *b = a->direita;
    a->direita = b->esquerda;
    if (b->esquerda != &arvore->nulo)
        b->esquerda->pai = a;
    b->pai = a->pai;
    if (a->pai == &arvore->nulo)
        arvore->raiz = b;
    else if (a == a->pai->esquerda)
        a->pai->esquerda = b;
    else
        a->pai->direita = b;
    b->esquerda = a;
    a->pai = b;
    ++arvore->metricas.rotacoes;
}

static void rotacao_direita(RUBRO_NEGRA *arvore, NO *a) {
    NO *b = a->esquerda;
    a->esquerda = b->direita;
    if (b->direita != &arvore->nulo)
        b->direita->pai = a;
    b->pai = a->pai;
    if (a->pai == &arvore->nulo)
        arvore->raiz = b;
    else if (a == a->pai->direita)
        a->pai->direita = b;
    else
        a->pai->esquerda = b;
    b->direita = a;
    a->pai = b;
    ++arvore->metricas.rotacoes;
}

static void corrigir_insercao(RUBRO_NEGRA *arvore, NO *no) {
    while (no->pai->cor == VERMELHO) {
        if (no->pai == no->pai->pai->esquerda) {
            NO *tio = no->pai->pai->direita;
            if (tio->cor == VERMELHO) {
                definir_cor(arvore, no->pai, PRETO);
                definir_cor(arvore, tio, PRETO);
                definir_cor(arvore, no->pai->pai, VERMELHO);
                no = no->pai->pai;
            } else {
                if (no == no->pai->direita) {
                    no = no->pai;
                    rotacao_esquerda(arvore, no);
                }
                definir_cor(arvore, no->pai, PRETO);
                definir_cor(arvore, no->pai->pai, VERMELHO);
                rotacao_direita(arvore, no->pai->pai);
            }
        } else {
            NO *tio = no->pai->pai->esquerda;
            if (tio->cor == VERMELHO) {
                definir_cor(arvore, no->pai, PRETO);
                definir_cor(arvore, tio, PRETO);
                definir_cor(arvore, no->pai->pai, VERMELHO);
                no = no->pai->pai;
            } else {
                if (no == no->pai->esquerda) {
                    no = no->pai;
                    rotacao_direita(arvore, no);
                }
                definir_cor(arvore, no->pai, PRETO);
                definir_cor(arvore, no->pai->pai, VERMELHO);
                rotacao_esquerda(arvore, no->pai->pai);
            }
        }
    }
    definir_cor(arvore, arvore->raiz, PRETO);
}

static NO *no_criar(RUBRO_NEGRA *arvore, const CAMPO *chave, off_t offset) {
    NO *no = calloc(1, sizeof(*no));
    if (no == NULL) return NULL;
    if (!chave_arvore_copiar(&no->chave, chave) ||
        !offsets_adicionar(&no->offsets, offset)) {
        chave_arvore_apagar(&no->chave);
        offsets_apagar(&no->offsets);
        free(no);
        return NULL;
    }
    no->cor = VERMELHO;
    no->esquerda = &arvore->nulo;
    no->direita = &arvore->nulo;
    return no;
}

static size_t altura_no(const RUBRO_NEGRA *arvore, const NO *no) {
    if (no == &arvore->nulo) return 0;
    size_t esquerda = altura_no(arvore, no->esquerda);
    size_t direita = altura_no(arvore, no->direita);
    return 1 + (esquerda > direita ? esquerda : direita);
}

static void apagar_no(RUBRO_NEGRA *arvore, NO *no) {
    if (no == &arvore->nulo) return;
    apagar_no(arvore, no->esquerda);
    apagar_no(arvore, no->direita);
    chave_arvore_apagar(&no->chave);
    offsets_apagar(&no->offsets);
    free(no);
}

RUBRO_NEGRA *rubro_negra_criar(void) {
    RUBRO_NEGRA *arvore = calloc(1, sizeof(*arvore));
    if (arvore == NULL) return NULL;
    arvore->nulo.cor = PRETO;
    arvore->nulo.esquerda = &arvore->nulo;
    arvore->nulo.direita = &arvore->nulo;
    arvore->nulo.pai = &arvore->nulo;
    arvore->raiz = &arvore->nulo;
    return arvore;
}

bool rubro_negra_inserir(RUBRO_NEGRA *arvore, const CAMPO *chave,
                         off_t offset) {
    if (arvore == NULL || !chave_arvore_valida(chave) || offset < 0 ||
        arvore->tamanho == SIZE_MAX)
        return false;

    NO *pai = &arvore->nulo;
    NO *atual = arvore->raiz;
    int comparacao = 0;
    while (atual != &arvore->nulo) {
        pai = atual;
        ++arvore->metricas.comparacoes_insercao;
        comparacao = chave_arvore_comparar(&atual->chave, chave);
        if (comparacao == 0)
            return offsets_adicionar(&atual->offsets, offset);
        atual = comparacao > 0 ? atual->esquerda : atual->direita;
    }

    NO *novo = no_criar(arvore, chave, offset);
    if (novo == NULL) return false;
    novo->pai = pai;
    if (pai == &arvore->nulo)
        arvore->raiz = novo;
    else if (comparacao > 0)
        pai->esquerda = novo;
    else
        pai->direita = novo;

    ++arvore->tamanho;
    corrigir_insercao(arvore, novo);
    return true;
}

const off_t *rubro_negra_buscar(RUBRO_NEGRA *arvore, const CAMPO *chave,
                                size_t *quantidade) {
    if (quantidade != NULL) *quantidade = 0;
    if (arvore == NULL || !chave_arvore_valida(chave)) return NULL;

    NO *atual = arvore->raiz;
    while (atual != &arvore->nulo) {
        ++arvore->metricas.comparacoes_busca;
        int comparacao = chave_arvore_comparar(&atual->chave, chave);
        if (comparacao == 0) {
            if (quantidade != NULL) *quantidade = atual->offsets.quantidade;
            return atual->offsets.dados;
        }
        atual = comparacao > 0 ? atual->esquerda : atual->direita;
    }
    return NULL;
}

size_t rubro_negra_tamanho(const RUBRO_NEGRA *arvore) {
    return arvore == NULL ? 0 : arvore->tamanho;
}

size_t rubro_negra_altura(const RUBRO_NEGRA *arvore) {
    return arvore == NULL ? 0 : altura_no(arvore, arvore->raiz);
}

const METRICAS_ARVORE *rubro_negra_metricas(const RUBRO_NEGRA *arvore) {
    return arvore == NULL ? NULL : &arvore->metricas;
}

void rubro_negra_apagar(RUBRO_NEGRA **arvore) {
    if (arvore == NULL || *arvore == NULL) return;
    apagar_no(*arvore, (*arvore)->raiz);
    free(*arvore);
    *arvore = NULL;
}
