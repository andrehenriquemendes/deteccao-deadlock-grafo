#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <dlfcn.h>
#include <execinfo.h>
#include <unistd.h>
#include <pthread.h>


#define FALSE 0
#define TRUE 1

sem_t MUTEX_GRAPH; // controla acesso ao grafo
int INIT_GRAPH = FALSE; 

int (*_sem_init)(sem_t *, int, unsigned int) = NULL;
int (*_sem_wait)(sem_t *) = NULL;
int (*_sem_post)(sem_t *) = NULL;


//------------------------ GRAFO --------------------------
typedef struct vizinho {
     void *id_vizinho;
    struct vizinho *prox;
} TVizinho;

typedef struct grafo {
    void *id_vertice;
    TVizinho *prim_vizinho;
    struct grafo *prox;
} TGrafo;

TGrafo *init_graph() {
    return NULL;
}


TGrafo* busca_vertice(TGrafo* g, void *x){
    while((g != NULL) && (g->id_vertice != x)) {
        g = g->prox;
    }
    return g;
}

TVizinho* busca_aresta(TGrafo *g, void *v1, void *v2){
    TGrafo *pv1 = busca_vertice(g,v1);
    TGrafo *pv2 = busca_vertice(g,v2);
    TVizinho *resp = NULL;
    //checa se ambos os vértices existem
    if((pv1 != NULL) && (pv2 != NULL)) {
        //percorre a lista de vizinhos de v1 procurando por v2
        resp = pv1->prim_vizinho;
        while ((resp != NULL) && (resp->id_vizinho != v2)) {
        resp = resp->prox;
        }
    }
    return resp;
}

TGrafo *insere_vertice(TGrafo *g, void *x){
    TGrafo *p = busca_vertice(g, x);
    if(p == NULL){
        p = (TGrafo*) malloc(sizeof(TGrafo));
        p->id_vertice = x;
        p->prox = g;
        p->prim_vizinho = NULL;
        g = p;
    }
    return g;
}

void insere_um_sentido(TGrafo *g, void *v1, void *v2){
    TGrafo *p = busca_vertice(g, v1);
    TVizinho *nova = (TVizinho *) malloc(sizeof(TVizinho));
    nova->id_vizinho = v2;
    nova->prox = p->prim_vizinho;
    p->prim_vizinho = nova;
} 

void insere_aresta_digrafo(TGrafo *g, void *v1, void *v2){
    TVizinho *v = busca_aresta(g, v1, v2);
    if(v == NULL) {
        insere_um_sentido(g, v1, v2);
    }
}

void retira_um_sentido(TGrafo *g, void *v1, void *v2){
    TGrafo *p = busca_vertice(g, v1);
    if(p != NULL) {
        TVizinho *ant = NULL;
        TVizinho *atual = p->prim_vizinho;
        while ((atual) && (atual->id_vizinho != v2)) {
            ant = atual;
            atual = atual->prox;
        }
        if (ant == NULL) //v2 era o primeiro nó da lista
            p->prim_vizinho = atual->prox;
        else
            ant->prox = atual->prox;
        free(atual);
    }
}

void retira_aresta_digrafo(TGrafo *g , void *v1, void *v2){
    TVizinho* v = busca_aresta(g,v1,v2);
    if(v != NULL) {
        retira_um_sentido(g, v1, v2);
    }
}

int tem_ciclo(TGrafo *g, void *x) {
    TGrafo *p, *node;
    p = busca_vertice(g, x);
    node = p; // node eh o no para comparacao
    TVizinho *v = p->prim_vizinho;
    while(v != NULL) {
        if(v)
            p = busca_vertice(g, v->id_vizinho);
        if(p->id_vertice == node->id_vertice)
            return 1;
        v = p->prim_vizinho;
        
    }
    return 0;
}
//------------------------ FIM DA IMPLEMENTACAO DO GRAFO ----------------------------------




// acessar e bloquear o grafo
void lock_graph() {
    _sem_wait(&MUTEX_GRAPH);
}

// liberar o grafo
void unlock_graph() {
    _sem_post(&MUTEX_GRAPH);
}


TGrafo *GRAPH = NULL; // Inicializa o grafo

int sem_init(sem_t *sem, int pshared, unsigned int value) {
    int r;

    if(!_sem_init) {
        // funcoes originais
        _sem_init = dlsym(RTLD_NEXT, "sem_init");
        _sem_wait = dlsym(RTLD_NEXT, "sem_wait");
        _sem_post = dlsym(RTLD_NEXT, "sem_post");
    }
    
    if(!INIT_GRAPH) {
        INIT_GRAPH = TRUE;
        _sem_init(&MUTEX_GRAPH, 0, 1); // cria o mutex para o grafo
        lock_graph(); // acessa o grafo
        GRAPH = init_graph();
        unlock_graph(); // libera o grafo
    }

    lock_graph();
    GRAPH = insere_vertice(GRAPH, sem); // insere o recurso no grafo
    unlock_graph();

    r = _sem_init(sem, pshared, value);
    return(r);
}

int sem_wait(sem_t *sem) {
    int r;
    
    if(!_sem_wait) {
        _sem_wait = dlsym(RTLD_NEXT, "sem_wait"); // sem_wait original 
    }
    
    pthread_t idThread = pthread_self(); // obtem o id da thread que esta executando

    lock_graph();
    GRAPH = insere_vertice(GRAPH, &idThread); // cria o no da thread no grafo
    insere_aresta_digrafo(GRAPH, &idThread, sem); // cria aresta na direcao processo->recurso
    if(tem_ciclo(GRAPH, sem)) { // deadlock!
        retira_aresta_digrafo(GRAPH, &idThread, sem);
        printf("--DEADLOCK--\n");
        fflush(stdout);
        unlock_graph();
        exit(-1);
        return -1;
    }

    unlock_graph();
    r = _sem_wait(sem);

    lock_graph();
    // inverte a aresta para recurso->processo
    retira_aresta_digrafo(GRAPH, &idThread, sem);
    insere_aresta_digrafo(GRAPH, sem, &idThread);

    unlock_graph();
    return(r);
}

int sem_post(sem_t *sem) {
    int r;
    
    if(!_sem_post) {
        _sem_post = dlsym(RTLD_NEXT, "sem_post"); // sem_post original 
    }
    

    pthread_t idThread = pthread_self();

    lock_graph();
    retira_aresta_digrafo(GRAPH, sem, &idThread); // remove do grafo a aresta recurso->processo
    unlock_graph();

    r = _sem_post(sem);

    return(r);
}
