#include "avl.h"
#include <stdio.h>
#include <stdlib.h>

typedef struct no_ NO;
struct no_ { int chave; int altura; NO *esq; NO *dir; };
struct avl_ { NO *raiz; };

static int maximo(int a, int b) { return a > b ? a : b; }
static int altura_no(NO *no) { return no == NULL ? 0 : no->altura; }
static int fb_no(NO *no) { return no == NULL ? 0 : altura_no(no->esq) - altura_no(no->dir); }
static void atualizar_altura(NO *no) { if (no != NULL) no->altura = 1 + maximo(altura_no(no->esq), altura_no(no->dir)); }

static NO *no_criar(int chave) {
    NO *novo = (NO *) malloc(sizeof(NO));
    if (novo == NULL) return NULL;
    novo->chave = chave; novo->altura = 1; novo->esq = NULL; novo->dir = NULL;
    return novo;
}

static NO *rotacao_direita(NO *a) {
    NO *b = a->esq;
    NO *t2 = b->dir;

    printf("    ROTACAO SIMPLES A DIREITA em A=%d\n", a->chave);
    printf("    B=%d sobe. A=%d passa a ser filho direito de B.\n", b->chave, a->chave);
    if (t2 == NULL)
        printf("    T2 e vazia.\n");
    else
        printf("    T2, cuja raiz e %d, passa para a esquerda de A.\n", t2->chave);

    b->dir = a;
    a->esq = t2;
    atualizar_altura(a);
    atualizar_altura(b);
    return b;
}

static NO *inserir_no(NO *raiz, int chave, int *ok) {
    int fator;
    if (raiz == NULL) {
        NO *novo = no_criar(chave);
        *ok = (novo != NULL);
        return novo;
    }

    if (chave < raiz->chave) raiz->esq = inserir_no(raiz->esq, chave, ok);
    else if (chave > raiz->chave) raiz->dir = inserir_no(raiz->dir, chave, ok);
    else { *ok = 0; return raiz; }

    atualizar_altura(raiz);
    fator = fb_no(raiz);
    printf("Retorno em %d: h(esq)=%d h(dir)=%d FB=%+d\n",
           raiz->chave, altura_no(raiz->esq), altura_no(raiz->dir), fator);

    if (fator > 1) {
        printf(">>> GATILHO em %d: FB=%+d. Filho esquerdo %d tem FB=%+d.\n",
               raiz->chave, fator, raiz->esq->chave, fb_no(raiz->esq));
        if (fb_no(raiz->esq) >= 0)
            return rotacao_direita(raiz);
        printf("    Este caso exige rotacao dupla e fica para a Parte 2.\n");
    }

    if (fator < -1)
        printf(">>> GATILHO em %d: a arvore esta pesada para a direita.\n"
               "    A rotacao esquerda ainda nao existe nesta atividade.\n", raiz->chave);

    return raiz;
}

static int balanceada_no(NO *r) {
    int f;
    if (r == NULL) return 1;
    f = fb_no(r);
    return f >= -1 && f <= 1 && balanceada_no(r->esq) && balanceada_no(r->dir);
}
static void imprimir_no(NO *r, int nivel) { int i; if (r == NULL) return; imprimir_no(r->dir,nivel+1); for(i=0;i<nivel;i++) printf("        "); printf("%d [h=%d FB=%+d]\n",r->chave,r->altura,fb_no(r)); imprimir_no(r->esq,nivel+1); }
static void apagar_no(NO *r) { if (r==NULL) return; apagar_no(r->esq); apagar_no(r->dir); free(r); }
AVL *avl_criar(void) { AVL *a=(AVL*)malloc(sizeof(AVL)); if(a!=NULL)a->raiz=NULL; return a; }
int avl_inserir(AVL *a,int chave){int ok=0;if(a==NULL)return 0;a->raiz=inserir_no(a->raiz,chave,&ok);return ok;}
void avl_imprimir(AVL *a){if(a==NULL||a->raiz==NULL){printf("<arvore vazia>\n");return;}imprimir_no(a->raiz,0);}
int avl_esta_balanceada(AVL *a){return a!=NULL && balanceada_no(a->raiz);}
void avl_apagar(AVL **a){if(a==NULL||*a==NULL)return;apagar_no((*a)->raiz);free(*a);*a=NULL;}
