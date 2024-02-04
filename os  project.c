
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define N_processes 100
#define q1_size 10
#define q2_size 20
#define q3_size 30
#define q1_Quantum 8
#define q2_Quantum 16
#define q1_cpu_utilization 0.5
#define q2_cpu_utilization 0.3
#define q3_cpu_utilization 0.2
//define process
typedef struct {
    int processid;
    int bursttime;
} process;
typedef struct {
    int head;
    int tail;
    int current_size;
    int max_size;
    int quantum; // quantum  for rr queues only
    float cpu_utilization;
} Queue;
void create_queue(Queue *queue, int max_size, int quantum,float cpu_utilization) {
    queue->head = 0;
    queue->tail = -1;
    queue->current_size = 0;
    queue->max_size = max_size;
    queue->quantum = quantum;
    queue->cpu_utilization=cpu_utilization;
}
int isEmpty(Queue *queue) {
    return queue->current_size == 0;
}

int isFull(Queue *queue) {
    return queue->current_size == queue->max_size;
}

void enqueue(Queue *queue, process *p) {
    if (!isFull(queue)) {
        queue->tail = (queue->tail + 1) % queue->max_size; // to make sure if the index  exceeds the maximum size of the queue it wont be out of boundary
        queue->current_size++;
    }
}

process dequeue(Queue *queue) {
    process p;
    if (!isEmpty(queue)) {
        p.processid = -1;
        p.bursttime = -1;
        queue->head = (queue->head + 1) % queue->max_size;
        queue->current_size--;
    }
    return p;
}
void roundrobin_schedulingalgo(Queue *queue, process *p, int *executing_pid, int *p_current_counter, int quantum) {
    //if the queue is not empty
    if (!isEmpty(queue)) {
            //check status of the process
            //if the process in q is executing so its pid !=-1&if its executing time reached to quantum time of q
            //then it will enter the queue again
        if (*executing_pid != -1 && *p_current_counter == quantum) {
            enqueue(queue, &p[*executing_pid]);
            *executing_pid = -1;
            *p_current_counter = 0;
        }
    //if there is no process that is executing then we will dequeue another process to be executed
    //and store its id & make current timer starts counting from 0

        if (*executing_pid == -1) {
            process temp = dequeue(queue);
            *executing_pid = temp.processid;
            *p_current_counter = 0;
        }

        // dec bursttime of process after execution and increase counter of current time
        p[*executing_pid].bursttime--;
        (*p_current_counter)++;

        // to check if process execution finished then make its id -1 as non executing process&make counter reset to 0
        if (p[*executing_pid].bursttime == 0) {
            *executing_pid = -1;
            *p_current_counter = 0;
        }
    }
}
void firstcomefirstserve_schedulingalgo(Queue *queue, process *p, int *executing_pid) {
    if (!isEmpty(queue)) {
            //if there is no process that is executing then we will dequeue another process to be executed
    //and store its id
        if (*executing_pid == -1) {
            process temp = dequeue(queue);
            *executing_pid = temp.processid;
        }

        // dec bursttime of process after execution
        p[*executing_pid].bursttime--;

         // to check if process execution finished then make its id -1 as non executing process
        if (p[*executing_pid].bursttime == 0) {
            *executing_pid = -1;
        }
    }
}
void enter_processes(Queue queues[], process *p) {
    for (int i = 0; i < N_processes; i++) {
        if (p[i].bursttime > 0) {
            if (p[i].bursttime <= q1_Quantum && !isFull(&queues[0])) {
                enqueue(&queues[0], &p[i]);
            } else if (p[i].bursttime <= q2_Quantum && !isFull(&queues[1])) {
                enqueue(&queues[1], &p[i]);
            } else if (!isFull(&queues[2])) {
                enqueue(&queues[2], &p[i]);
            } else {
                // queue is full, enter the remainder of processes according to high priority queue (have high cpu utilization)
                if (queues[0].cpu_utilization > queues[1].cpu_utilization &&
                    queues[0].cpu_utilization > queues[2].cpu_utilization) {
                    enqueue(&queues[0], &p[i]);
                } else if (queues[1].cpu_utilization > queues[2].cpu_utilization) {
                    enqueue(&queues[1], &p[i]);
                } else {
                    enqueue(&queues[2], &p[i]);
                }
            }

        }
    }
}


int countprocesses(Queue queues[]) {
    int count = 0;
    for (int i = 0; i < 3; i++) {
        count = count+queues[i].current_size;
    }
    return count;
}
void multilevelfeedbackqueue_schedulingalgo(Queue queues[], process *p, int total_processes) {
    int time = 0;
    int current_pid = -1;
    int time_counter = 0;

    while (total_processes > 0) {
        enter_processes(queues, p);
        roundrobin_schedulingalgo(&queues[0], p, &current_pid, &time_counter, q1_Quantum);
        roundrobin_schedulingalgo(&queues[1], p, &current_pid, &time_counter, q2_Quantum);
        firstcomefirstserve_schedulingalgo(&queues[2], p, &current_pid);

        // Iterate through processes to check if any finished during this time unit and print its id
        for (int i = 0; i < N_processes; i++) {
            if (p[i].bursttime == 0 && p[i].bursttime != -1) {
                printf("Process %d finished\n", p[i].processid);
                total_processes--;  // decrease total_processes count
                p[i].bursttime = -1;  // to tell that the process is finished
            } else if (p[i].bursttime > 0) {
                // dec the bursttime for processes that are still executing
                p[i].bursttime--;
            }
        }

        time++;
    }
}



int main() {
    //creating queues with their props
   Queue queues[3];
    create_queue(&queues[0], q1_size, q1_Quantum, q1_cpu_utilization);
    create_queue(&queues[1], q2_size, q2_Quantum, q2_cpu_utilization);
    create_queue(&queues[2], q3_size, -1, q3_cpu_utilization);

    process p[N_processes];
    for (int i = 0; i < N_processes; i++) {
        p[i].processid = i + 1;  //make the processid to start from 1
        p[i].bursttime = rand() % 100 + 1; //make the bursstime random
    }

    multilevelfeedbackqueue_schedulingalgo(queues, p, N_processes);

    return 0;




}
