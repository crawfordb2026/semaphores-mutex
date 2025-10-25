#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#define BUFFER_SIZE 2
#define MAX_ITEMS 10
#define NUM_CONSUMER_THREADS 2

// Structure for shared memory
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    int count;
} shared_data_t;

// Global variables
int shmid;
int semid;
shared_data_t *shared_data;
pthread_t consumer_threads[NUM_CONSUMER_THREADS];
int total_consumed = 0;

// semaphore operations
void sem_op(int semid, int sem_num, int op) {
    struct sembuf sem_buf;
    sem_buf.sem_num = sem_num;
    sem_buf.sem_op = op;
    sem_buf.sem_flg = 0;
    semop(semid, &sem_buf, 1);
}

// wait on semaphore
void sem_wait(int semid, int sem_num) {
    sem_op(semid, sem_num, -1);
}

// signal semaphore
void sem_signal(int semid, int sem_num) {
    sem_op(semid, sem_num, 1);
}

// Consumer thread function
void* consumer_thread(void* arg) {
    int thread_id = *(int*)arg;
    int item;
    int consumed = 0;
    
    printf("Consumer thread %d started (TID: %p)\n", thread_id, (void*)pthread_self());
    
    // keep consuming until we're done
    while (total_consumed < MAX_ITEMS) {
        // wait for items and get mutex
        sem_wait(semid, 1);
        sem_wait(semid, 2);
        
        // take item from buffer if there's something there
        if (shared_data->count > 0 && total_consumed < MAX_ITEMS) {
            item = shared_data->buffer[shared_data->out];
            printf("Consumer thread %d consumed item %d from position %d\n", 
                   thread_id, item, shared_data->out);
            shared_data->buffer[shared_data->out] = 0;
            shared_data->out = (shared_data->out + 1) % BUFFER_SIZE;
            shared_data->count--;
            consumed++;
            total_consumed++;
        }
        
        // release mutex and signal empty slot
        sem_signal(semid, 2);
        sem_signal(semid, 0);
        
        // consumption time
        usleep(750000);
    }
    
    printf("Consumer thread %d finished consuming %d items\n", thread_id, consumed);
    return NULL;
}

int main() {
    key_t key = 1234;
    int thread_ids[NUM_CONSUMER_THREADS];
    int i;
    
    printf("Consumer process started (PID: %d)\n", getpid());
    
    // Get shared memory
    shmid = shmget(key, sizeof(shared_data_t), 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }
    
    // Attach shared memory
    shared_data = (shared_data_t*)shmat(shmid, NULL, 0);
    if (shared_data == (void*)-1) {
        perror("shmat failed");
        exit(1);
    }
    
    // Get semaphores
    semid = semget(key, 3, 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }
    
    printf("Creating %d consumer threads\n", NUM_CONSUMER_THREADS);
    
    // create threads
    for (i = 0; i < NUM_CONSUMER_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&consumer_threads[i], NULL, consumer_thread, &thread_ids[i]);
    }
    
    // wait for threads
    for (i = 0; i < NUM_CONSUMER_THREADS; i++) {
        pthread_join(consumer_threads[i], NULL);
    }
    
    printf("Consumer finished consuming %d items total\n", total_consumed);
    printf("Consumer process finished\n");
    
    // Detach shared memory
    shmdt(shared_data);
    
    return 0;
}
