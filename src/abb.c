#include "abb.h"

#include <stdlib.h>

typedef struct no NO;

struct no {
    CHAVE_ARVORE chave;
    LISTA_OFFSETS offsets;
    size_t altura;
    NO *esquerda;
    NO *direita;
};

struct abb {
    NO *raiz;
    size_t tamanho;
    METRICAS_ARVORE metricas;
};

static size_t altura_no(const NO *no) {
    return no == NULL ? 0 : no->altura;
}

static size_t maximo(size_t a, size_t b) {
    return a > b ? a : b;
}

static void atualizar_altura(NO *no) {
    no->altura = 1 + maximo(altura_no(no->esquerda), altura_no(no->direita));
}

static NO *no_criar(const CAMPO *chave, off_t offset) {
    NO *no = calloc(1, sizeof(*no));
    if (no == NULL) return NULL;
    if (!chave_arvore_copiar(&no->chave, chave) ||
        !offsets_adicionar(&no->offsets, offset)) {
        chave_arvore_apagar(&no->chave);
        offsets_apagar(&no->offsets);
        free(no);
        return NULL;
    }
    no->altura = 1;
    return no;
}

static NO *inserir_no(ABB *abb, NO *raiz, const CAMPO *chave, off_t offset,
                      bool *sucesso, bool *novo_no) {
    if (raiz == NULL) {
        NO *novo = no_criar(chave, offset);
        *sucesso = novo != NULL;
        *novo_no = *sucesso;
        return novo;
    }

    ++abb->metricas.comparacoes_insercao;
    int comparacao = chave_arvore_comparar(&raiz->chave, chave);
    if (comparacao == 0) {
        *sucesso = offsets_adicionar(&raiz->offsets, offset);
        *novo_no = false;
        return raiz;
    }

    if (comparacao > 0)
        raiz->esquerda = inserir_no(abb, raiz->esquerda, chave, offset,
                                    sucesso, novo_no);
    else
        raiz->direita = inserir_no(abb, raiz->direita, chave, offset,
                                   sucesso, novo_no);

    if (*sucesso) atualizar_altura(raiz);
    return raiz;
}

static void apagar_no(NO *no) {
    if (no == NULL) return;
    apagar_no(no->esquerda);
    apagar_no(no->direita);
    chave_arvore_apagar(&no->chave);
    offsets_apagar(&no->offsets);
    free(no);
}

ABB *abb_criar(void) {
    return calloc(1, sizeof(ABB));
}

bool abb_inserir(ABB *abb, const CAMPO *chave, off_t offset) {
    if (abb == NULL || !chave_arvore_valida(chave) || offset < 0) return false;
    bool sucesso = false, novo_no = false;
    abb->raiz = inserir_no(abb, abb->raiz, chave, offset,
                           &sucesso, &novo_no);
    if (sucesso && novo_no) ++abb->tamanho;
    return sucesso;
}

const off_t *abb_buscar(ABB *abb, const CAMPO *chave, size_t *quantidade) {
    if (quantidade != NULL) *quantidade = 0;
    if (abb == NULL || !chave_arvore_valida(chave)) return NULL;

    NO *atual = abb->raiz;
    while (atual != NULL) {
        ++abb->metricas.comparacoes_busca;
        int comparacao = chave_arvore_comparar(&atual->chave, chave);
        if (comparacao == 0) {
            if (quantidade != NULL) *quantidade = atual->offsets.quantidade;
            return atual->offsets.dados;
        }
        atual = comparacao > 0 ? atual->esquerda : atual->direita;
    }
    return NULL;
}

size_t abb_tamanho(const ABB *abb) {
    return abb == NULL ? 0 : abb->tamanho;
}

size_t abb_altura(const ABB *abb) {
    return abb == NULL ? 0 : altura_no(abb->raiz);
}

const METRICAS_ARVORE *abb_metricas(const ABB *abb) {
    return abb == NULL ? NULL : &abb->metricas;
}

void abb_apagar(ABB **abb) {
    if (abb == NULL || *abb == NULL) return;
    apagar_no((*abb)->raiz);
    free(*abb);
    *abb = NULL;
}
