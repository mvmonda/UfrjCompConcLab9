#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"

// Argumento para cada thread com início e fim
typedef struct {
   int inicio, fim;
} t_Args;

// Número de threads, tamanho do vetor, vetor de elementos para realizar o quicksort, número de threads
int nthreads, nvetor, *vetor, countThreads = 1;

pthread_mutex_t x_mutex;

int tentaUtilizarUmaThread() {
    pthread_mutex_lock(&x_mutex); 
    if(countThreads == nthreads) {
        pthread_mutex_unlock(&x_mutex);
        return 0;
    } 
    else {
        countThreads++;
        pthread_mutex_unlock(&x_mutex);
        return 1;
    }
}

// Função genérica de particionamento
int particiona(int inicio, int fim, int *vetor) {
    int i = inicio, j = fim;
 
    // Escolhe um pivot aleatório dentro do limite
    int pivot =  inicio;

    int t = vetor[j];
    vetor[j] = vetor[pivot];
    vetor[pivot] = t;
    j--;

    // Particionamento
    while (i <= j) {

        if (vetor[i] <= vetor[fim]) {
            i++;
            continue;
        }

        if (vetor[j] >= vetor[fim]) {
            j--;
            continue;
        }

        t = vetor[j];
        vetor[j] = vetor[i];
        vetor[i] = t;
        j--;
        i++;
    }

    // Corrige posicionamento do pivot
    t = vetor[j + 1];
    vetor[j + 1] = vetor[fim];
    vetor[fim] = t;
    return j + 1;
}

void quickSort(t_Args * args) {
    // Se inicio = fim, chegamos a partição final e podemos retornar
    if(args->inicio >= args->fim) {
        return;   
    }

    // particiona o segmento do array
    int p = particiona(args->inicio, args->fim, vetor);

    t_Args* argsEsquerda = malloc(sizeof(t_Args));
    argsEsquerda->inicio = args->inicio;
    argsEsquerda->fim = p - 1;

    quickSort(argsEsquerda);
    //

    // Executa para segmento à direita.
    t_Args* argsDireita = malloc(sizeof(t_Args));
    argsDireita->inicio = p + 1;
    argsDireita->fim = args->fim;

    quickSort(argsDireita);
}

void * quickSortThread(void * arg) {
    t_Args *args = (t_Args *) arg;

    // Se inicio = fim, chegamos a partição final e podemos retornar
    if(args->inicio >= args->fim) {
        return 0;   
    }

    // particiona o segmento do array
    int p = particiona(args->inicio, args->fim, vetor);
    
    // Cria thread para segmento da "esquerda"
    pthread_t esquerda;

    t_Args* argsEsquerda = malloc(sizeof(t_Args));
    argsEsquerda->inicio = args->inicio;
    argsEsquerda->fim = p - 1;

    // Verifica se já chegamos no limite de threads
    int podeParalelizar = tentaUtilizarUmaThread();
    if(podeParalelizar) {
        printf("\n Entrou numa outra thread!");
        // Se não chegamos, cria thread
        pthread_create(&esquerda, NULL, quickSortThread, (void*) argsEsquerda);
    } else {
        // Se não, executa sequencial
        quickSort(argsEsquerda);
    }
    //

    // Executa para segmento à direita.
    t_Args* argsDireita = malloc(sizeof(t_Args));
    argsDireita->inicio = p + 1;
    argsDireita->fim = args->fim;

    quickSortThread((void *)argsDireita);

    if(podeParalelizar)
        pthread_join(esquerda, NULL);
}

int main(int argc, char *argv[]) 
{  
    if(argc < 2) {
        fprintf(stderr, "Digite: %s <numero de threads> <tamanho vetor>\n", argv[0]);
        return 1;
    }

    nthreads = atoi(argv[1]);

    nvetor = atoi(argv[2]);

    // Preenche vetor
    vetor = malloc(sizeof(int) * nvetor);

    if(vetor==NULL) {
      fprintf(stderr, "ERRO - Número de elementos no vetor muito grande");
      return 1;
    }


    for(int i = 0; i < nvetor; i++) {
        vetor[i] = rand() % 1000000;
    }

    // 

    // Inicia processamento

    t_Args* args = malloc(sizeof(t_Args));
    args->inicio = 0;
    args->fim = nvetor - 1;

    double start, finish;
    GET_TIME(start);
    quickSortThread((void *) args);
    //quickSort(args);
    GET_TIME(finish);
    
    printf("ELAPSED: %lf\n\n\n", finish - start);

    // Verifica e printa o resultado
    for(int i = 0; i < nvetor; i++) {
        if(i != 0 && vetor[i - 1] > vetor[i]) {
            fprintf(stderr, "Erro de validação. Vetor não ordenado!");
            exit(1);
        }
        printf("%d ", vetor[i]);
    }
}