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
static NO *no_criar(int chave) { NO *n=(NO*)malloc(sizeof(NO)); if(n==NULL)return NULL; n->chave=chave;n->altura=1;n->esq=NULL;n->dir=NULL;return n; }

static NO *rotacao_direita(NO *a) {
    NO *b=a->esq, *t2=b->dir;
    printf("    ROTACAO SIMPLES A DIREITA em A=%d; B=%d sobe.\n",a->chave,b->chave);
    b->dir=a; a->esq=t2; atualizar_altura(a); atualizar_altura(b); return b;
}

static NO *rotacao_esquerda(NO *a) {
    NO *b = a->dir;
    NO *t2 = b->esq;

    printf("    ROTACAO SIMPLES A ESQUERDA em A=%d\n", a->chave);
    printf("    B=%d sobe. A=%d passa a ser filho esquerdo de B.\n", b->chave, a->chave);
    if (t2 == NULL)
        printf("    T2 e vazia.\n");
    else
        printf("    T2, cuja raiz e %d, passa para a direita de A.\n", t2->chave);

    b->esq = a;
    a->dir = t2;
    atualizar_altura(a);
    atualizar_altura(b);
    return b;
}

static NO *rebalancear(NO *r) {
    int f;
    atualizar_altura(r);
    f=fb_no(r);
    printf("Retorno em %d: h(esq)=%d h(dir)=%d FB=%+d\n",r->chave,altura_no(r->esq),altura_no(r->dir),f);
    if(f>1){printf(">>> GATILHO em %d: FB=%+d; filho esquerdo FB=%+d.\n",r->chave,f,fb_no(r->esq));if(fb_no(r->esq)>=0)return rotacao_direita(r);printf("    Caso duplo: Parte 2.\n");}
    if(f<-1){printf(">>> GATILHO em %d: FB=%+d; filho direito FB=%+d.\n",r->chave,f,fb_no(r->dir));if(fb_no(r->dir)<=0)return rotacao_esquerda(r);printf("    Caso duplo: Parte 2.\n");}
    return r;
}

static NO *inserir_no(NO *r,int chave,int *ok){if(r==NULL){NO*n=no_criar(chave);*ok=(n!=NULL);return n;}if(chave<r->chave)r->esq=inserir_no(r->esq,chave,ok);else if(chave>r->chave)r->dir=inserir_no(r->dir,chave,ok);else{*ok=0;return r;}return rebalancear(r);}
static int balanceada_no(NO *r){int f;if(r==NULL)return 1;f=fb_no(r);return f>=-1&&f<=1&&balanceada_no(r->esq)&&balanceada_no(r->dir);}
static void imprimir_no(NO *r,int nivel){int i;if(r==NULL)return;imprimir_no(r->dir,nivel+1);for(i=0;i<nivel;i++)printf("        ");printf("%d [h=%d FB=%+d]\n",r->chave,r->altura,fb_no(r));imprimir_no(r->esq,nivel+1);}
static void apagar_no(NO*r){if(r==NULL)return;apagar_no(r->esq);apagar_no(r->dir);free(r);}
AVL *avl_criar(void){AVL*a=(AVL*)malloc(sizeof(AVL));if(a!=NULL)a->raiz=NULL;return a;}
int avl_inserir(AVL*a,int chave){int ok=0;if(a==NULL)return 0;a->raiz=inserir_no(a->raiz,chave,&ok);return ok;}
void avl_imprimir(AVL*a){if(a==NULL||a->raiz==NULL){printf("<arvore vazia>\n");return;}imprimir_no(a->raiz,0);}
int avl_esta_balanceada(AVL*a){return a!=NULL&&balanceada_no(a->raiz);}
void avl_apagar(AVL**a){if(a==NULL||*a==NULL)return;apagar_no((*a)->raiz);free(*a);*a=NULL;}
