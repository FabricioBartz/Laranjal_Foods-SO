#include <pthread.h> 
#include <stdio.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <time.h> 

#define NUM_RESTAURANTES 5  // número de restaurantes
#define NUM_ENTREGADORES 10 // número de entregadores

// Estrutura para passar o ID do entregador para as threads
typedef struct {
    int id;
} ContextoEntregador;


// Protótipos das funções dos entregadores
void* entregador_veterano(void* arg);
void* entregador_novato(void* arg);

// Arrays de Mutexes para os restaurantes e pedidos
pthread_mutex_t motos[NUM_RESTAURANTES];
pthread_mutex_t pedidos[NUM_RESTAURANTES];

// Função principal
int main() {
    pthread_t entregadores[NUM_ENTREGADORES];
    srand(time(NULL)); // Garante que a aleatoriedade mude a cada execução

    // Inicializando os mutexes
    for (int i = 0; i < NUM_RESTAURANTES; i++) {
        pthread_mutex_init(&motos[i], NULL);
        pthread_mutex_init(&pedidos[i], NULL);
    }

    printf("=== Simulação do Laranjal Foods ===\n\n");

    // Criando as Threads de Entregadores
    for (int i = 0; i < NUM_ENTREGADORES; i++) {
        ContextoEntregador* ctx = malloc(sizeof(ContextoEntregador));
        ctx->id = i;
        
        if (i % 2 == 0) 
            pthread_create(&entregadores[i], NULL, entregador_veterano, ctx);
        else 
            pthread_create(&entregadores[i], NULL, entregador_novato, ctx);
    }

    // Aguardando as Threads terminarem (neste caso, elas rodarão indefinidamente)
    for (int i = 0; i < NUM_ENTREGADORES; i++) {
        pthread_join(entregadores[i], NULL);
    }

    return 0;
}

// Função para os entregadores veteranos
void* entregador_veterano(void* arg) {
    // O entregador veterano tenta pegar a moto primeiro, depois o lanche
    ContextoEntregador* ctx = (ContextoEntregador*)arg;
    int id_entregador = ctx->id;

    while (1) { // Loop principal: Volta para a fila infinitamente
        int restaurante = rand() % NUM_RESTAURANTES; // Sorteia a cada nova entrega

        pthread_mutex_lock(&motos[restaurante]);// Tenta pegar a moto primeiro
        printf("[Veterano %d]: Peguei a chave da moto do Restaurante %d.\n", id_entregador, restaurante); 

        sleep(1); // Simula o tempo para pegar o lanche

        printf("[Veterano %d]: Aguardando lanche do Restaurante %d...\n", id_entregador, restaurante);
        
        if (pthread_mutex_trylock(&pedidos[restaurante]) == 0) { // Tenta pegar o lanche
            printf("[Veterano %d]: Entrega feita! Liberando Restaurante %d e voltando para a fila.\n", id_entregador, restaurante);
            pthread_mutex_unlock(&pedidos[restaurante]); // Libera o lanche
            pthread_mutex_unlock(&motos[restaurante]); // Libera a moto
            
            usleep(500000); // Pequena pausa antes de pegar o próximo pedido
        } else {
            printf("[Veterano %d]: DEADLOCK! Devolvendo chave da moto do Restaurante %d para destravar.\n", id_entregador, restaurante);
            pthread_mutex_unlock(&motos[restaurante]); // Libera a moto para evitar deadlock
            usleep((rand() % 500 + 500) * 1000); // Pausa aleatória para evitar que o mesmo entregador tente o mesmo restaurante imediatamente
        }
    }
    free(ctx); // Libera a memória alocada 
    return NULL;
}

// Função para os entregadores novatos
void* entregador_novato(void* arg) {
    // O entregador novato tenta pegar o lanche primeiro, depois a moto
    ContextoEntregador* ctx = (ContextoEntregador*)arg;
    int id_entregador = ctx->id;

    while (1) {  // Loop principal: Volta para a fila infinitamente
        int restaurante = rand() % NUM_RESTAURANTES; 

        pthread_mutex_lock(&pedidos[restaurante]); // Tenta pegar o lanche primeiro
        printf("[Novato %d]: Peguei o lanche do Restaurante %d.\n", id_entregador, restaurante);

        sleep(1); // Simula o tempo para pegar a moto

        printf("[Novato %d]: Aguardando moto do Restaurante %d...\n", id_entregador, restaurante);
        
        if (pthread_mutex_trylock(&motos[restaurante]) == 0) { // Tenta pegar a moto
            printf("[Novato %d]: Entrega feita! Liberando Restaurante %d e voltando para a fila.\n", id_entregador, restaurante);
            pthread_mutex_unlock(&motos[restaurante]); // Libera a moto
            pthread_mutex_unlock(&pedidos[restaurante]); // Libera o lanche
            
            usleep(500000); // Pequena pausa antes de pegar o próximo pedido
        } else {
            printf("[Novato %d]: DEADLOCK! Devolvendo lanche do Restaurante %d para destravar.\n", id_entregador, restaurante);
            pthread_mutex_unlock(&pedidos[restaurante]); // Libera o lanche para evitar deadlock
            usleep((rand() % 500 + 500) * 1000); // Pausa aleatória para evitar que o mesmo entregador tente o mesmo restaurante imediatamente
        }
    }
    free(ctx); // Libera a memória alocada
    return NULL; 
}