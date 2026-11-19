#ifndef ABB_H
#define ABB_H

#include <stdbool.h>

typedef struct no_ NO;
typedef struct abb_ ABB;

ABB *abb_criar(void);
void abb_apagar(ABB **t);
bool abb_inserir(ABB *t, int chave);
void abb_imprimir(const ABB *t);
bool abb_buscar(const ABB *t, int chave);
void abb_em_ordem(const ABB *t);
void abb_pre_ordem(const ABB *t);
void abb_pos_ordem(const ABB *t);
bool abb_remover(ABB *t, int chave);
#endif
