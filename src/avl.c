#include "avl.h"

#include <stdint.h>
#include <stdlib.h>

typedef struct no NO;

struct no {
    CHAVE_ARVORE chave;
    LISTA_OFFSETS offsets;
    int altura;
    NO *esquerda;
    NO *direita;
};

struct avl {
    NO *raiz;
    size_t tamanho;
    METRICAS_ARVORE metricas;
};

static int altura_no(const NO *no) {
    return no == NULL ? 0 : no->altura;
}

static int maximo(int a, int b) {
    return a > b ? a : b;
}

static void atualizar_altura(NO *no) {
    no->altura = 1 + maximo(altura_no(no->esquerda), altura_no(no->direita));
}

static int fator_balanceamento(const NO *no) {
    return altura_no(no->esquerda) - altura_no(no->direita);
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

static NO *rotacao_direita(AVL *avl, NO *a) {
    NO *b = a->esquerda;
    a->esquerda = b->direita;
    b->direita = a;
    atualizar_altura(a);
    atualizar_altura(b);
    ++avl->metricas.rotacoes;
    return b;
}

static NO *rotacao_esquerda(AVL *avl, NO *a) {
    NO *b = a->direita;
    a->direita = b->esquerda;
    b->esquerda = a;
    atualizar_altura(a);
    atualizar_altura(b);
    ++avl->metricas.rotacoes;
    return b;
}

static NO *rebalancear(AVL *avl, NO *no) {
    atualizar_altura(no);
    int fator = fator_balanceamento(no);

    if (fator > 1) {
        if (fator_balanceamento(no->esquerda) < 0)
            no->esquerda = rotacao_esquerda(avl, no->esquerda);
        return rotacao_direita(avl, no);
    }
    if (fator < -1) {
        if (fator_balanceamento(no->direita) > 0)
            no->direita = rotacao_direita(avl, no->direita);
        return rotacao_esquerda(avl, no);
    }
    return no;
}

static NO *inserir_no(AVL *avl, NO *raiz, const CAMPO *chave, off_t offset,
                      bool *sucesso, bool *novo_no) {
    if (raiz == NULL) {
        NO *novo = no_criar(chave, offset);
        *sucesso = novo != NULL;
        *novo_no = *sucesso;
        return novo;
    }

    ++avl->metricas.comparacoes_insercao;
    int comparacao = chave_arvore_comparar(&raiz->chave, chave);
    if (comparacao == 0) {
        *sucesso = offsets_adicionar(&raiz->offsets, offset);
        *novo_no = false;
        return raiz;
    }

    if (comparacao > 0)
        raiz->esquerda = inserir_no(avl, raiz->esquerda, chave, offset,
                                    sucesso, novo_no);
    else
        raiz->direita = inserir_no(avl, raiz->direita, chave, offset,
                                   sucesso, novo_no);

    if (!*sucesso || !*novo_no) return raiz;
    return rebalancear(avl, raiz);
}

static void apagar_no(NO *no) {
    if (no == NULL) return;
    apagar_no(no->esquerda);
    apagar_no(no->direita);
    chave_arvore_apagar(&no->chave);
    offsets_apagar(&no->offsets);
    free(no);
}

AVL *avl_criar(void) {
    return calloc(1, sizeof(AVL));
}

bool avl_inserir(AVL *avl, const CAMPO *chave, off_t offset) {
    if (avl == NULL || !chave_arvore_valida(chave) || offset < 0 ||
        avl->tamanho == SIZE_MAX)
        return false;

    bool sucesso = false, novo_no = false;
    avl->raiz = inserir_no(avl, avl->raiz, chave, offset,
                           &sucesso, &novo_no);
    if (sucesso && novo_no) ++avl->tamanho;
    return sucesso;
}

const off_t *avl_buscar(AVL *avl, const CAMPO *chave, size_t *quantidade) {
    if (quantidade != NULL) *quantidade = 0;
    if (avl == NULL || !chave_arvore_valida(chave)) return NULL;

    NO *atual = avl->raiz;
    while (atual != NULL) {
        ++avl->metricas.comparacoes_busca;
        int comparacao = chave_arvore_comparar(&atual->chave, chave);
        if (comparacao == 0) {
            if (quantidade != NULL) *quantidade = atual->offsets.quantidade;
            return atual->offsets.dados;
        }
        atual = comparacao > 0 ? atual->esquerda : atual->direita;
    }
    return NULL;
}

size_t avl_tamanho(const AVL *avl) {
    return avl == NULL ? 0 : avl->tamanho;
}

size_t avl_altura(const AVL *avl) {
    return avl == NULL ? 0 : (size_t)altura_no(avl->raiz);
}

const METRICAS_ARVORE *avl_metricas(const AVL *avl) {
    return avl == NULL ? NULL : &avl->metricas;
}

void avl_apagar(AVL **avl) {
    if (avl == NULL || *avl == NULL) return;
    apagar_no((*avl)->raiz);
    free(*avl);
    *avl = NULL;
}
