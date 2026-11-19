#ifndef AVL_H
#define AVL_H

#include "chave.h"

typedef struct avl AVL;

AVL *avl_criar(void);
bool avl_inserir(AVL *avl, const CAMPO *chave, off_t offset);
const off_t *avl_buscar(AVL *avl, const CAMPO *chave, size_t *quantidade);
size_t avl_tamanho(const AVL *avl);
size_t avl_altura(const AVL *avl);
const METRICAS_ARVORE *avl_metricas(const AVL *avl);
void avl_apagar(AVL **avl);

#endif
