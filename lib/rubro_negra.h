#ifndef RUBRO_NEGRA_H
#define RUBRO_NEGRA_H

#include "arvore_comum.h"

typedef struct rubro_negra RUBRO_NEGRA;

RUBRO_NEGRA *rubro_negra_criar(void);
bool rubro_negra_inserir(RUBRO_NEGRA *arvore, const CAMPO *chave,
                         off_t offset);
const off_t *rubro_negra_buscar(RUBRO_NEGRA *arvore, const CAMPO *chave,
                                size_t *quantidade);
size_t rubro_negra_tamanho(const RUBRO_NEGRA *arvore);
size_t rubro_negra_altura(const RUBRO_NEGRA *arvore);
const METRICAS_ARVORE *rubro_negra_metricas(const RUBRO_NEGRA *arvore);
void rubro_negra_apagar(RUBRO_NEGRA **arvore);

#endif
