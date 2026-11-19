#include "abb.h"
#include "persistencia_privado.h"

#include <stdlib.h>
#include <stdint.h>

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
    if (abb == NULL || !chave_arvore_valida(chave) || offset < 0 ||
        abb->tamanho == SIZE_MAX ||
        (abb->raiz != NULL && abb->raiz->chave.campo.tipo != chave->tipo))
        return false;
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

static bool salvar_no(FILE *f, const NO *n) {
    uint8_t presente = n != NULL;
    if (fwrite(&presente, 1, 1, f) != 1 || !presente)
        return presente == 0;

    CHAVE_ARVORE chave = n->chave;
    LISTA_OFFSETS offsets = n->offsets;
    return gravar_chave(f, &chave) && gravar_offsets(f, &offsets) &&
           gravar_u64(f, (uint64_t)n->altura) &&
           salvar_no(f, n->esquerda) && salvar_no(f, n->direita);
}

static NO *carregar_no(FILE *f, ABB *abb, TIPO_CAMPO tipo, bool *ok) {
    uint8_t presente;
    uint64_t altura;
    if (fread(&presente, 1, 1, f) != 1 || presente > 1) {
        *ok = false;
        return NULL;
    }
    if (!presente) return NULL;

    NO *no = calloc(1, sizeof(*no));
    if (no == NULL) {
        *ok = false;
        return NULL;
    }
    if (!ler_chave(f, &no->chave, tipo) ||
        !ler_offsets(f, &no->offsets) || !ler_u64(f, &altura) ||
        altura == 0 || altura > SIZE_MAX) {
        *ok = false;
        goto falha;
    }
    no->altura = (size_t)altura;
    no->esquerda = carregar_no(f, abb, tipo, ok);
    if (!*ok) goto falha;
    no->direita = carregar_no(f, abb, tipo, ok);
    if (!*ok) goto falha;
    if (no->altura != 1 + maximo(altura_no(no->esquerda),
                                 altura_no(no->direita)) ||
        abb->tamanho == SIZE_MAX) {
        *ok = false;
        goto falha;
    }
    ++abb->tamanho;
    return no;

falha:
    apagar_no(no->esquerda);
    apagar_no(no->direita);
    chave_arvore_apagar(&no->chave);
    offsets_apagar(&no->offsets);
    free(no);
    return NULL;
}

static bool validar_no(const NO *no, const CAMPO *minimo,
                       const CAMPO *limite_superior, TIPO_CAMPO tipo,
                       uint64_t tamanho_arquivo) {
    if (no == NULL) return true;
    size_t altura = 1 + maximo(altura_no(no->esquerda),
                               altura_no(no->direita));
    if (no->chave.campo.tipo != tipo ||
        (minimo && chave_arvore_comparar(&no->chave, minimo) <= 0) ||
        (limite_superior &&
         chave_arvore_comparar(&no->chave, limite_superior) >= 0) ||
        no->altura != altura ||
        !offsets_no_arquivo(&no->offsets, tamanho_arquivo))
        return false;
    return validar_no(no->esquerda, minimo, &no->chave.campo, tipo,
                      tamanho_arquivo) &&
           validar_no(no->direita, &no->chave.campo, limite_superior, tipo,
                      tamanho_arquivo);
}

bool abb_salvar(const ABB *abb,const char *caminho,const char *coluna,
                TIPO_CAMPO tipo,const char *dados){
    uint64_t tamanho;
    if (abb == NULL || caminho == NULL || coluna == NULL ||
        !tipo_campo_valido(tipo) ||
        !persist_caminhos_distintos(caminho, dados) ||
        !persist_tamanho_arquivo(dados, &tamanho) ||
        !validar_no(abb->raiz, NULL, NULL, tipo, tamanho)) return false;
    FILE *f = fopen(caminho, "wb");
    if (f == NULL) return false;
    bool ok = persist_cabecalho(f, "abb", coluna, tipo, dados, true) &&
              salvar_no(f, abb->raiz);
    if (fclose(f) != 0) ok = false;
    return ok;
}

ABB *abb_carregar(const char *caminho,const char *coluna,TIPO_CAMPO tipo,
                  const char *dados){
    uint64_t tamanho;
    if (caminho == NULL || !persist_tamanho_arquivo(dados, &tamanho))
        return NULL;
    FILE *f = fopen(caminho, "rb");
    if (f == NULL) return NULL;
    bool ok = persist_cabecalho(f, "abb", coluna, tipo, dados, false);
    ABB *a = ok ? abb_criar() : NULL;
    if (ok && a == NULL) ok = false;
    if (ok) a->raiz = carregar_no(f, a, tipo, &ok);
    if (ok) ok = validar_no(a->raiz, NULL, NULL, tipo, tamanho);
    if (ok && (fgetc(f) != EOF || ferror(f))) ok = false;
    fclose(f);
    if (!ok) {
        abb_apagar(&a);
        return NULL;
    }
    return a;
}
