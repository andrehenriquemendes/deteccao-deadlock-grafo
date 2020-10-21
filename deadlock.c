#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define A 0
#define B 1

sem_t semA, semB;

void recurso(int rec) {

    pthread_t idThread = pthread_self();
    if(rec == 0) {
        printf("\t%lu usando o recurso A\n", idThread);
        fflush(stdout);
    }
    else {
        printf("\t%lu usando o recurso B\n", idThread);
        fflush(stdout);
    }
}

void *threadA(void *args) {
    
    while(1) {
        pthread_t idThread = pthread_self();
        printf("Thread %lu solicita acesso ao recurso <B>\n", idThread);
        sem_wait(&semB); // solicita o recurso B
        printf("Thread %lu ganha acesso ao recurso <B>\n", idThread);
        printf("Thread %lu solicita acesso ao recurso <A>\n", idThread);
        sem_wait(&semA); // solicita o recurso A
        printf("Thread %lu ganha acesso ao recurso <A>\n", idThread);

        // regioes criticas
        recurso(A); // acessa o recurso A
        recurso(B); // acessa o recurso B

        sem_post(&semA); // libera o recurso A
        printf("Thread %lu libera o recurso <A>\n", idThread);
        sem_post(&semB); // libera o recurso B
        printf("Thread %lu libera o recurso <B>\n", idThread);

        printf("%lu terminou sua execução\n\n", pthread_self()); 
        fflush(stdout);
    }
    pthread_exit(NULL);
}

void *threadB(void *args) {
    
    while(1) {
        pthread_t idThread = pthread_self();
        printf("Thread %lu solicita acesso ao recurso <A>\n", idThread);
        sem_wait(&semA); // solicita o recurso A
        printf("Thread %lu ganha acesso ao recurso <A>\n", idThread);
        printf("Thread %lu solicita acesso ao recurso <B>\n", idThread);
        sem_wait(&semB); // solicita o recurso B
        printf("Thread %lu ganha acesso ao recurso <B>\n", idThread);

        // regioes criticas 
        recurso(A); // acessa o recurso B
        recurso(B); // acessa o recurso A

        sem_post(&semA); // libera o recurso A
        printf("Thread %lu libera o recurso <A>\n", idThread);
        sem_post(&semB); // libera o recurso B
        printf("Thread %lu libera o recurso <B>\n", idThread);

        printf("%lu terminou sua execução\n", pthread_self()); 
        fflush(stdout);
    }
    pthread_exit(NULL);
}

int main() {

    sem_init(&semA, 0, 1);
    sem_init(&semB, 0, 1);

    pthread_t tA, tB;

    pthread_create(&tA, NULL, threadA, NULL);
    pthread_create(&tB, NULL, threadB, NULL);

    pthread_join(tA, NULL);
    pthread_join(tB, NULL);

    return 0;
}