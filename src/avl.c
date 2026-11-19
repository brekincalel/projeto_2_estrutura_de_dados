#include "avl.h"
#include "persistencia_privado.h"

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
        avl->tamanho == SIZE_MAX ||
        (avl->raiz != NULL && avl->raiz->chave.campo.tipo != chave->tipo))
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

static bool salvar_no(FILE *f, const NO *no) {
    uint8_t presente = no != NULL;
    if (fwrite(&presente, 1, 1, f) != 1 || !presente)
        return presente == 0;
    CHAVE_ARVORE chave = no->chave;
    LISTA_OFFSETS offsets = no->offsets;
    return gravar_chave(f, &chave) && gravar_offsets(f, &offsets) &&
           gravar_u32(f, (uint32_t)no->altura) &&
           salvar_no(f, no->esquerda) && salvar_no(f, no->direita);
}
static NO *carregar_no(FILE *f, AVL *avl, TIPO_CAMPO tipo, bool *ok) {
    uint8_t presente;
    uint32_t altura;
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
        !ler_offsets(f, &no->offsets) || !ler_u32(f, &altura) ||
        altura == 0 || altura > INT32_MAX) {
        *ok = false;
        goto falha;
    }
    no->altura = (int)altura;
    no->esquerda = carregar_no(f, avl, tipo, ok);
    if (!*ok) goto falha;
    no->direita = carregar_no(f, avl, tipo, ok);
    if (!*ok) goto falha;
    if (avl->tamanho == SIZE_MAX ||
        abs(altura_no(no->esquerda) - altura_no(no->direita)) > 1 ||
        no->altura != 1 + maximo(altura_no(no->esquerda),
                                 altura_no(no->direita))) {
        *ok = false;
        goto falha;
    }
    ++avl->tamanho;
    return no;

falha:
    apagar_no(no->esquerda);
    apagar_no(no->direita);
    chave_arvore_apagar(&no->chave);
    offsets_apagar(&no->offsets);
    free(no);
    return NULL;
}
static bool validar_arvore(const NO *no, const CAMPO *minimo,
                           const CAMPO *limite_superior, TIPO_CAMPO tipo,
                           uint64_t tamanho_arquivo) {
    if (no == NULL) return true;
    int altura = 1 + maximo(altura_no(no->esquerda), altura_no(no->direita));
    if (no->chave.campo.tipo != tipo || no->altura != altura ||
        !offsets_no_arquivo(&no->offsets, tamanho_arquivo) ||
        abs(altura_no(no->esquerda) - altura_no(no->direita)) > 1 ||
        (minimo && chave_arvore_comparar(&no->chave, minimo) <= 0) ||
        (limite_superior &&
         chave_arvore_comparar(&no->chave, limite_superior) >= 0))
        return false;
    return validar_arvore(no->esquerda, minimo, &no->chave.campo, tipo,
                          tamanho_arquivo) &&
           validar_arvore(no->direita, &no->chave.campo, limite_superior, tipo,
                          tamanho_arquivo);
}

bool avl_salvar(const AVL *avl, const char *caminho, const char *coluna,
                TIPO_CAMPO tipo, const char *dados) {
    uint64_t tamanho;
    if (avl == NULL || coluna == NULL || !tipo_campo_valido(tipo) ||
        !persist_caminhos_distintos(caminho, dados) ||
        !persist_tamanho_arquivo(dados, &tamanho) ||
        !validar_arvore(avl->raiz, NULL, NULL, tipo, tamanho)) return false;
    FILE *f = fopen(caminho, "wb");
    if (f == NULL) return false;
    bool ok = persist_cabecalho(f, "avl", coluna, tipo, dados, true) &&
              salvar_no(f, avl->raiz);
    if (fclose(f) != 0) ok = false;
    return ok;
}

AVL *avl_carregar(const char *caminho, const char *coluna, TIPO_CAMPO tipo,
                  const char *dados) {
    uint64_t tamanho;
    if (caminho == NULL || !persist_tamanho_arquivo(dados, &tamanho))
        return NULL;
    FILE *f = fopen(caminho, "rb");
    if (f == NULL) return NULL;
    bool ok = persist_cabecalho(f, "avl", coluna, tipo, dados, false);
    AVL *avl = ok ? avl_criar() : NULL;
    if (ok && avl == NULL) ok = false;
    if (ok) avl->raiz = carregar_no(f, avl, tipo, &ok);
    if (ok) ok = validar_arvore(avl->raiz, NULL, NULL, tipo, tamanho);
    if (ok && (fgetc(f) != EOF || ferror(f))) ok = false;
    fclose(f);
    if (!ok) {
        avl_apagar(&avl);
        return NULL;
    }
    return avl;
}
