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
#define NUM_PRODUCER_THREADS 3

// struct shared memory
typedef struct {
    int buffer[BUFFER_SIZE];
    int in;
    int out;
    int count;
} shared_data_t;

// global variables
int shmid;
int semid;
shared_data_t *shared_data;
pthread_t producer_threads[NUM_PRODUCER_THREADS];
int total_produced = 0;

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

// Producer thread function
void* producer_thread(void* arg) {


    int thread_id = *(int*)arg;
    int item = 1;
    int produced = 0;
    printf("Producer thread %d started (TID: %p)\n", thread_id, (void*)pthread_self());
    
    // keep producing until we hit the limit
    while (total_produced < MAX_ITEMS) {
        // wait for empty slot and get mutex
        sem_wait(semid, 0);
        sem_wait(semid, 2);
        
        // add item to buffer if there's space
        if (shared_data->count < BUFFER_SIZE && total_produced < MAX_ITEMS) {
            shared_data->buffer[shared_data->in] = item;
            printf("Producer thread %d produced item %d at position %d\n", 
                   thread_id, item, shared_data->in);
            shared_data->in = (shared_data->in + 1) % BUFFER_SIZE;
            shared_data->count++;
            item++;
            produced++;
            total_produced++;
        }
        sem_signal(semid, 2);
        sem_signal(semid, 1);
        
        // production time
        usleep(500000);
    }
    
    printf("Producer thread %d finished producing %d items\n", thread_id, produced);
    return NULL;
}

int main() {
    key_t key = 1234;
    int thread_ids[NUM_PRODUCER_THREADS];
    int i;
    
    printf("Producer process started (PID: %d)\n", getpid());
    
    // Create or get shared memory
    shmid = shmget(key, sizeof(shared_data_t), IPC_CREAT | 0666);
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
    
    // Initialize shared data
    shared_data->in = 0;
    shared_data->out = 0;
    shared_data->count = 0;
    memset(shared_data->buffer, 0, sizeof(shared_data->buffer));
    
    // create semaphores
    semid = semget(key, 3, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget failed");
        exit(1);
    }
    
    // init semaphores
    semctl(semid, 0, SETVAL, BUFFER_SIZE);
    semctl(semid, 1, SETVAL, 0);
    semctl(semid, 2, SETVAL, 1);
    
    printf("Creating %d producer threads\n", NUM_PRODUCER_THREADS);
    
    // create threads
    for (i = 0; i < NUM_PRODUCER_THREADS; i++) {
        thread_ids[i] = i + 1;
        pthread_create(&producer_threads[i], NULL, producer_thread, &thread_ids[i]);
    }
    
    // wait for threads
    for (i = 0; i < NUM_PRODUCER_THREADS; i++) {
        pthread_join(producer_threads[i], NULL);
    }
    
    printf("Producer finished producing %d items total\n", total_produced);
    printf("Producer process finished\n");
    
    // Detach shared memory
    shmdt(shared_data);
    
    return 0;
}
