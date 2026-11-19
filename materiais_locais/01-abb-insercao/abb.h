#ifndef ABB_H
#define ABB_H

#include <stdbool.h>

typedef struct no_ NO;
typedef struct abb_ ABB;

ABB *abb_criar(void);
void abb_apagar(ABB **t);
bool abb_inserir(ABB *t, int chave);
void abb_imprimir(const ABB *t);

#endif
