#ifndef AVL_H
#define AVL_H

typedef struct avl_ AVL;

AVL *avl_criar(void);
int avl_inserir(AVL *avl, int chave);
void avl_imprimir(AVL *avl);
int avl_esta_balanceada(AVL *avl);
void avl_apagar(AVL **avl);

#endif
