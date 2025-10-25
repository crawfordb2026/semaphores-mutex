# Producer-Consumer Problem Implementation

## Overview
This project implements the Producer-Consumer synchronization problem using shared memory, semaphores, and threads in C. The producer program creates multiple producer threads that generate items and place them on a shared table (buffer), while the consumer program creates multiple consumer threads that pick up and process these items. The table can hold exactly 2 items at any time.


## Requirements
- Buffer Size: The shared table can hold exactly 2 items
- Synchronization: Uses semaphores to coordinate producer and consumer
- Mutual Exclusion: Ensures thread-safe access to shared resources
- Process Communication: Uses shared memory for inter-process communication
- Thread-based: Each program uses multiple threads within separate processes

Basically, the producer makes items and puts them on the table, and the consumer takes them off. The table can only hold 2 items at once, so they have to coordinate using semaphores.

## Files


### producer.c
- Creates and initializes shared memory
- Sets up semaphores
- Creates 3 producer threads that generate items
- Total of 10 items produced across all threads

### consumer.c
- Attaches to existing shared memory
- Uses existing semaphores
- Creates 2 consumer threads that consume items
- Total of 10 items consumed across all threads

The producer runs first to set up the shared memory and semaphores, then the consumer attaches to them.

## How to Compile and Run

### Step 1: Compile the programs
```bash 
#commands to run
gcc producer.c -pthread -lrt -o producer
gcc consumer.c -pthread -lrt -o consumer
./producer & ./consumer &
```

### Step 3: Check the output
You should see the producer and consumer threads working together, producing and consuming items.

### Environment Setup
- Linux/Unix system (or macOS for testing)
- GCC compiler with pthread support
- POSIX semaphores and shared memory support

## Expected Output
```
Producer process started (PID: 12345)
Creating 3 producer threads
Producer thread 1 started (TID: 0x700007e9d000)
Producer thread 2 started (TID: 0x700007f20000)
Producer thread 3 started (TID: 0x700007fa3000)
Consumer process started (PID: 12346)
Creating 2 consumer threads
Consumer thread 1 started (TID: 0x7000010d5000)
Consumer thread 2 started (TID: 0x700001158000)
Producer thread 1 produced item 1 at position 0
Producer thread 2 produced item 1 at position 1
Consumer thread 1 consumed item 1 from position 0
Consumer thread 2 consumed item 1 from position 1
Producer finished producing 10 items total
Consumer finished consuming 10 items total
Producer process finished
Consumer process finished


## How It Works

The producer threads wait for empty slots in the buffer, then add items. The consumer threads wait for items to be available, then remove them. Semaphores make sure only one thread can access the buffer at a time and coordinate when to wait and when to proceed.

I used three semaphores: one for empty slots, one for full slots, and one for mutual exclusion.