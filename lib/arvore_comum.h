#ifndef ARVORE_COMUM_H
#define ARVORE_COMUM_H

#include "item.h"

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>

typedef struct {
    size_t comparacoes_insercao;
    size_t comparacoes_busca;
    size_t rotacoes;
    size_t alteracoes_cor;
} METRICAS_ARVORE;

typedef struct {
    CAMPO campo;
    char *texto;
} CHAVE_ARVORE;

typedef struct {
    off_t *dados;
    size_t quantidade;
    size_t capacidade;
} LISTA_OFFSETS;

/* Textos sao copiados. Cada chave pode guardar varios offsets. */
bool chave_arvore_valida(const CAMPO *campo);
bool chave_arvore_copiar(CHAVE_ARVORE *destino, const CAMPO *origem);
int chave_arvore_comparar(const CHAVE_ARVORE *a, const CAMPO *b);
void chave_arvore_apagar(CHAVE_ARVORE *chave);

bool offsets_adicionar(LISTA_OFFSETS *lista, off_t offset);
void offsets_apagar(LISTA_OFFSETS *lista);

#endif
