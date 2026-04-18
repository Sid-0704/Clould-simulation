#ifndef SIMULATION_CORE_H
#define SIMULATION_CORE_H

#include <stddef.h>

#define MAX_VMS 4
#define MAX_TASKS 10
#define SIMULATION_TIME 15

typedef struct {
    int id;
    int total_cpu;
    int total_mem;
    int used_cpu;
    int used_mem;
} VM;

typedef struct {
    int id;
    int cpu_req;
    int mem_req;
    int priority;
} Task;

typedef struct {
    int step;
    int algorithm; /* 0 = SJF, 1 = Priority */
    Task tasks[MAX_TASKS];
    int task_count;
    char events[2048];
} StepResult;

typedef enum {
    ALG_SJF = 0,
    ALG_PRIORITY = 1
} SchedulingAlgorithm;

extern VM vms[MAX_VMS];
extern int vm_count;

void init_vms();
Task generate_task(int id);
int can_allocate(VM *vm, Task t);
void allocate_task(VM *vm, Task t, char *events, size_t events_size);
int find_best_vm(Task t);
int task_size(Task t);
void sjf_schedule(Task tasks[], int n, char *events, size_t events_size);
void priority_schedule(Task tasks[], int n, char *events, size_t events_size);
void release_resources();
void print_status();
void simulate_step(int step, SchedulingAlgorithm algorithm, StepResult *result);
void run_console_simulation(SchedulingAlgorithm algorithm);

#endif
