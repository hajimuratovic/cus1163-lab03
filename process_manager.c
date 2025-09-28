#include "process_manager.h"

int run_basic_demo(void) {
    int pipe_fd[2];
    pid_t producer_pid, consumer_pid;
    int status;
    
    printf("\nParent process (PID: %d) creating children...\n", getpid());
    
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
        return -1;
    }

    producer_pid = fork();
    if (producer_pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }
    if (producer_pid == 0) {
        close(pipe_fd[0]);
        producer_process(pipe_fd[1], 1);
        exit(0);
    } else {
        printf("Created producer child (PID: %d)\n", producer_pid);
    }

    consumer_pid = fork();
    if (consumer_pid == -1) {
        perror("fork");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
        return -1;
    }
    if (consumer_pid == 0) {
        close(pipe_fd[1]);
        consumer_process(pipe_fd[0], 0);
        exit(0);
    } else {
        printf("Created consumer child (PID: %d)\n", consumer_pid);
    }

    close(pipe_fd[0]);
    close(pipe_fd[1]);

    if (waitpid(producer_pid, &status, 0) > 0) {
        printf("Producer child (PID: %d) exited with status %d\n", producer_pid, status);
    }
    if (waitpid(consumer_pid, &status, 0) > 0) {
        printf("Consumer child (PID: %d) exited with status %d\n", consumer_pid, status);
    }

    return 0;
}

int run_multiple_pairs(int num_pairs) {
    pid_t pids[10];
    int pid_count = 0;

    printf("\nParent creating %d producer-consumer pairs...\n", num_pairs);

    for (int i = 0; i < num_pairs; i++) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe");
            return -1;
        }

        printf("\n=== Pair %d ===\n", i + 1);

        pid_t prod = fork();
        if (prod == -1) {
            perror("fork");
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            return -1;
        }
        if (prod == 0) {
            close(pipe_fd[0]);
            int start_num = i * 5 + 1;
            producer_process(pipe_fd[1], start_num);
            exit(0);
        }
        pids[pid_count++] = prod;

        pid_t cons = fork();
        if (cons == -1) {
            perror("fork");
            close(pipe_fd[0]);
            close(pipe_fd[1]);
            return -1;
        }
        if (cons == 0) {
            close(pipe_fd[1]);
            consumer_process(pipe_fd[0], i + 1);
            exit(0);
        }
        pids[pid_count++] = cons;

        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    printf("\nAll pairs completed successfully!\n");
    int status;
    for (int i = 0; i < pid_count; i++) {
        pid_t w = waitpid(pids[i], &status, 0);
        if (w > 0) {
            printf("Child (PID: %d) exited with status %d\n", w, status);
        }
    }

    return 0;
}

void producer_process(int write_fd, int start_num) {
    printf("Producer (PID: %d) starting...\n", getpid());
    for (int i = 0; i < NUM_VALUES; i++) {
        int number = start_num + i;
        if (write(write_fd, &number, sizeof(number)) != sizeof(number)) {
            perror("write");
            exit(1);
        }
        printf("Producer: Sent number %d\n", number);
        usleep(100000);
    }
    printf("Producer: Finished sending %d numbers\n", NUM_VALUES);
    close(write_fd);
    exit(0);
}

void consumer_process(int read_fd, int pair_id) {
    int number;
    int count = 0;
    int sum = 0;
    
    printf("Consumer (PID: %d) starting...\n", getpid());
    while (read(read_fd, &number, sizeof(number)) > 0) {
        count++;
        sum += number;
        printf("Consumer: Received %d, running sum: %d\n", number, sum);
    }
    printf("Consumer: Final sum: %d\n", sum);
    close(read_fd);
    exit(0);
}

