#include "rubro_negra.h"
#include "persistencia_privado.h"

#include <stdint.h>
#include <stdlib.h>

typedef enum { PRETO, VERMELHO } COR;
typedef struct no NO;

struct no {
    CHAVE_ARVORE chave;
    LISTA_OFFSETS offsets;
    COR cor; /* Cor do elo que liga o pai a este no. */
    NO *esquerda;
    NO *direita;
};

struct rubro_negra {
    NO *raiz;
    size_t tamanho;
    METRICAS_ARVORE metricas;
};

static bool vermelho(const NO *no) {
    return no != NULL && no->cor == VERMELHO;
}

static void definir_cor(RUBRO_NEGRA *arvore, NO *no, COR cor) {
    if (no != NULL && no->cor != cor) {
        no->cor = cor;
        ++arvore->metricas.alteracoes_cor;
    }
}

static NO *rotacao_esquerda(RUBRO_NEGRA *arvore, NO *h) {
    NO *x = h->direita;
    h->direita = x->esquerda;
    x->esquerda = h;
    definir_cor(arvore, x, h->cor);
    definir_cor(arvore, h, VERMELHO);
    ++arvore->metricas.rotacoes;
    return x;
}

static NO *rotacao_direita(RUBRO_NEGRA *arvore, NO *h) {
    NO *x = h->esquerda;
    h->esquerda = x->direita;
    x->direita = h;
    definir_cor(arvore, x, h->cor);
    definir_cor(arvore, h, VERMELHO);
    ++arvore->metricas.rotacoes;
    return x;
}

static void inverter_cores(RUBRO_NEGRA *arvore, NO *h) {
    definir_cor(arvore, h, h->cor == VERMELHO ? PRETO : VERMELHO);
    definir_cor(arvore, h->esquerda,
                h->esquerda->cor == VERMELHO ? PRETO : VERMELHO);
    definir_cor(arvore, h->direita,
                h->direita->cor == VERMELHO ? PRETO : VERMELHO);
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
    no->cor = VERMELHO;
    return no;
}

static NO *inserir_no(RUBRO_NEGRA *arvore, NO *h, const CAMPO *chave,
                      off_t offset, bool *sucesso, bool *novo_no) {
    if (h == NULL) {
        NO *novo = no_criar(chave, offset);
        *sucesso = novo != NULL;
        *novo_no = *sucesso;
        return novo;
    }

    ++arvore->metricas.comparacoes_insercao;
    int comparacao = chave_arvore_comparar(&h->chave, chave);
    if (comparacao == 0) {
        *sucesso = offsets_adicionar(&h->offsets, offset);
        *novo_no = false;
        return h;
    }

    if (comparacao > 0)
        h->esquerda = inserir_no(arvore, h->esquerda, chave, offset,
                                 sucesso, novo_no);
    else
        h->direita = inserir_no(arvore, h->direita, chave, offset,
                                sucesso, novo_no);

    if (!*sucesso || !*novo_no) return h;
    if (vermelho(h->direita) && !vermelho(h->esquerda))
        h = rotacao_esquerda(arvore, h);
    if (vermelho(h->esquerda) && vermelho(h->esquerda->esquerda))
        h = rotacao_direita(arvore, h);
    if (vermelho(h->esquerda) && vermelho(h->direita))
        inverter_cores(arvore, h);
    return h;
}

static size_t altura_no(const NO *no) {
    if (no == NULL) return 0;
    size_t esquerda = altura_no(no->esquerda);
    size_t direita = altura_no(no->direita);
    return 1 + (esquerda > direita ? esquerda : direita);
}

static void apagar_no(NO *no) {
    if (no == NULL) return;
    apagar_no(no->esquerda);
    apagar_no(no->direita);
    chave_arvore_apagar(&no->chave);
    offsets_apagar(&no->offsets);
    free(no);
}

RUBRO_NEGRA *rubro_negra_criar(void) {
    return calloc(1, sizeof(RUBRO_NEGRA));
}

bool rubro_negra_inserir(RUBRO_NEGRA *arvore, const CAMPO *chave,
                         off_t offset) {
    if (arvore == NULL || !chave_arvore_valida(chave) || offset < 0 ||
        arvore->tamanho == SIZE_MAX ||
        (arvore->raiz != NULL && arvore->raiz->chave.campo.tipo != chave->tipo))
        return false;

    bool sucesso = false, novo_no = false;
    arvore->raiz = inserir_no(arvore, arvore->raiz, chave, offset,
                              &sucesso, &novo_no);
    if (!sucesso) return false;
    if (novo_no) ++arvore->tamanho;
    definir_cor(arvore, arvore->raiz, PRETO);
    return true;
}

const off_t *rubro_negra_buscar(RUBRO_NEGRA *arvore, const CAMPO *chave,
                                size_t *quantidade) {
    if (quantidade != NULL) *quantidade = 0;
    if (arvore == NULL || !chave_arvore_valida(chave)) return NULL;
    NO *atual = arvore->raiz;
    while (atual != NULL) {
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
    return arvore == NULL ? 0 : altura_no(arvore->raiz);
}

const METRICAS_ARVORE *rubro_negra_metricas(const RUBRO_NEGRA *arvore) {
    return arvore == NULL ? NULL : &arvore->metricas;
}

void rubro_negra_apagar(RUBRO_NEGRA **arvore) {
    if (arvore == NULL || *arvore == NULL) return;
    apagar_no((*arvore)->raiz);
    free(*arvore);
    *arvore = NULL;
}

static bool salvar_no(FILE *arquivo, const NO *no) {
    uint8_t presente = no != NULL;
    if (!gravar_u8(arquivo, presente) || !presente)
        return presente == 0;
    CHAVE_ARVORE chave = no->chave;
    LISTA_OFFSETS offsets = no->offsets;
    uint8_t cor = (uint8_t)no->cor;
    return gravar_chave(arquivo, &chave) &&
           gravar_offsets(arquivo, &offsets) &&
           gravar_u8(arquivo, cor) &&
           salvar_no(arquivo, no->esquerda) &&
           salvar_no(arquivo, no->direita);
}

static NO *carregar_no(FILE *arquivo, RUBRO_NEGRA *arvore,
                       TIPO_CAMPO tipo, bool *ok) {
    uint8_t presente, cor;
    if (!ler_u8(arquivo, &presente) || presente > 1) {
        *ok = false;
        return NULL;
    }
    if (!presente) return NULL;

    NO *no = calloc(1, sizeof(*no));
    if (no == NULL) {
        *ok = false;
        return NULL;
    }
    if (!ler_chave(arquivo, &no->chave, tipo) ||
        !ler_offsets(arquivo, &no->offsets) ||
        !ler_u8(arquivo, &cor) || cor > VERMELHO) {
        *ok = false;
        goto falha;
    }
    no->cor = (COR)cor;
    no->esquerda = carregar_no(arquivo, arvore, tipo, ok);
    if (!*ok) goto falha;
    no->direita = carregar_no(arquivo, arvore, tipo, ok);
    if (!*ok) goto falha;
    if (arvore->tamanho == SIZE_MAX) {
        *ok = false;
        goto falha;
    }
    ++arvore->tamanho;
    if (vermelho(no->direita) ||
        (vermelho(no) && (vermelho(no->esquerda) || vermelho(no->direita)))) {
        *ok = false;
        goto falha;
    }
    return no;

falha:
    apagar_no(no->esquerda);
    apagar_no(no->direita);
    chave_arvore_apagar(&no->chave);
    offsets_apagar(&no->offsets);
    free(no);
    return NULL;
}

static bool validar_llrb(const NO *no, const CAMPO *minimo,
                         const CAMPO *limite_superior, TIPO_CAMPO tipo,
                         uint64_t tamanho_arquivo) {
    if (no == NULL) return true;
    if (no->chave.campo.tipo != tipo ||
        !offsets_no_arquivo(&no->offsets, tamanho_arquivo) ||
        vermelho(no->direita) ||
        (vermelho(no) && vermelho(no->esquerda)) ||
        (minimo && chave_arvore_comparar(&no->chave, minimo) <= 0) ||
        (limite_superior &&
         chave_arvore_comparar(&no->chave, limite_superior) >= 0))
        return false;
    return validar_llrb(no->esquerda, minimo, &no->chave.campo, tipo,
                        tamanho_arquivo) &&
           validar_llrb(no->direita, &no->chave.campo, limite_superior, tipo,
                        tamanho_arquivo);
}

static size_t altura_negra(const NO *no, bool *ok) {
    if (no == NULL) return 1;
    size_t esquerda = altura_negra(no->esquerda, ok);
    size_t direita = altura_negra(no->direita, ok);
    if (esquerda != direita) *ok = false;
    return esquerda + (no->cor == PRETO);
}

bool rubro_negra_salvar(const RUBRO_NEGRA *arvore, const char *caminho,
                        const char *coluna, TIPO_CAMPO tipo,
                        const char *arquivo_dados) {
    bool valido = true;
    uint64_t tamanho;
    if (arvore == NULL || caminho == NULL || coluna == NULL ||
        !tipo_campo_valido(tipo) ||
        !persist_caminhos_distintos(caminho, arquivo_dados) ||
        !persist_tamanho_arquivo(arquivo_dados, &tamanho) ||
        (arvore->raiz != NULL && arvore->raiz->cor != PRETO) ||
        !validar_llrb(arvore->raiz, NULL, NULL, tipo, tamanho)) return false;
    (void)altura_negra(arvore->raiz, &valido);
    if (!valido) return false;
    FILE *arquivo = fopen(caminho, "wb");
    if (arquivo == NULL) return false;
    bool ok = persist_cabecalho(arquivo, "rubro_negra_llrb", coluna, tipo,
                                arquivo_dados, true) &&
              salvar_no(arquivo, arvore->raiz);
    if (fclose(arquivo) != 0) ok = false;
    return ok;
}

RUBRO_NEGRA *rubro_negra_carregar(const char *caminho, const char *coluna,
                                  TIPO_CAMPO tipo,
                                  const char *arquivo_dados) {
    uint64_t tamanho;
    if (caminho == NULL || !persist_tamanho_arquivo(arquivo_dados, &tamanho))
        return NULL;
    FILE *arquivo = fopen(caminho, "rb");
    if (arquivo == NULL) return NULL;
    bool ok = persist_cabecalho(arquivo, "rubro_negra_llrb", coluna, tipo,
                                arquivo_dados, false);
    RUBRO_NEGRA *arvore = ok ? rubro_negra_criar() : NULL;
    if (ok && arvore == NULL) ok = false;
    if (ok) arvore->raiz = carregar_no(arquivo, arvore, tipo, &ok);
    if (ok && arvore->raiz != NULL && arvore->raiz->cor != PRETO) ok = false;
    if (ok) (void)altura_negra(arvore->raiz, &ok);
    if (ok) ok = validar_llrb(arvore->raiz, NULL, NULL, tipo, tamanho);
    if (ok && (fgetc(arquivo) != EOF || ferror(arquivo))) ok = false;
    fclose(arquivo);
    if (!ok) {
        rubro_negra_apagar(&arvore);
        return NULL;
    }
    return arvore;
}
