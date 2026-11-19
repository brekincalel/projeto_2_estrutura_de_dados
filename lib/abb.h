#ifndef ABB_H
#define ABB_H

#include "chave.h"

typedef struct abb ABB;

ABB *abb_criar(void);
bool abb_inserir(ABB *abb, const CAMPO *chave, off_t offset);
const off_t *abb_buscar(ABB *abb, const CAMPO *chave, size_t *quantidade);
size_t abb_tamanho(const ABB *abb);
size_t abb_altura(const ABB *abb);
const METRICAS_ARVORE *abb_metricas(const ABB *abb);
void abb_apagar(ABB **abb);

/* Salva/carrega a topologia da ABB. O arquivo inclui metadados dos dados. */
bool abb_salvar(const ABB *abb, const char *caminho, const char *coluna,
                TIPO_CAMPO tipo_chave,
                const char *arquivo_dados);
ABB *abb_carregar(const char *caminho, const char *coluna,
                  TIPO_CAMPO tipo_chave,
                  const char *arquivo_dados);

#endif
