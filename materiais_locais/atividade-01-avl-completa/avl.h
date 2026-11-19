#ifndef AVL_H
#define AVL_H

#include <stdbool.h>
#include "item.h"

typedef struct avl_ AVL;

AVL *avl_criar(void);
void avl_apagar(AVL **avl);

bool avl_inserir(AVL *avl, ITEM *item);
ITEM *avl_buscar(const AVL *avl, int chave);
ITEM *avl_remover(AVL *avl, int chave);

bool avl_vazia(const AVL *avl);
int avl_tamanho(const AVL *avl);
int avl_altura(const AVL *avl);
void avl_imprimir(const AVL *avl);

#endif
