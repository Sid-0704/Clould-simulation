#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VMS 4
#define MAX_TASKS 10
#define SIMULATION_TIME 15

// -----------------------------
// Structures
// -----------------------------
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

// -----------------------------
// Global Variables
// -----------------------------
VM vms[MAX_VMS];
int vm_count = MAX_VMS;

// -----------------------------
// Initialize VMs
// -----------------------------
void init_vms() {
    for (int i = 0; i < vm_count; i++) {
        vms[i].id = i;
        vms[i].total_cpu = 4 + rand() % 3;   // 4–6 CPU
        vms[i].total_mem = 8 + rand() % 5;   // 8–12 GB
        vms[i].used_cpu = 0;
        vms[i].used_mem = 0;
    }
}

// -----------------------------
// Generate Random Task
// -----------------------------
Task generate_task(int id) {
    Task t;
    t.id = id;
    t.cpu_req = 1 + rand() % 3;
    t.mem_req = 1 + rand() % 3;
    t.priority = rand() % 10; // 0–9 (higher = more priority)
    return t;
}

// -----------------------------
// Check Allocation
// -----------------------------
int can_allocate(VM *vm, Task t) {
    return (vm->used_cpu + t.cpu_req <= vm->total_cpu) &&
           (vm->used_mem + t.mem_req <= vm->total_mem);
}

// -----------------------------
// Allocate Task
// -----------------------------
void allocate_task(VM *vm, Task t) {
    vm->used_cpu += t.cpu_req;
    vm->used_mem += t.mem_req;

    printf("Task %d allocated to VM %d\n", t.id, vm->id);
}

// -----------------------------
// Least Loaded VM (Helper)
// -----------------------------
int find_best_vm(Task t) {
    int best = -1;
    int min_load = 100000;

    for (int i = 0; i < vm_count; i++) {
        int load = vms[i].used_cpu + vms[i].used_mem;

        if (can_allocate(&vms[i], t) && load < min_load) {
            min_load = load;
            best = i;
        }
    }

    return best;
}

// -----------------------------
// Shortest Job First (SJF)
// -----------------------------
int task_size(Task t) {
    return t.cpu_req + t.mem_req;
}

void sjf_schedule(Task tasks[], int n) {
    // Sort tasks by smallest size
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (task_size(tasks[j]) > task_size(tasks[j + 1])) {
                Task temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    printf("\n--- SJF Scheduling ---\n");

    for (int i = 0; i < n; i++) {
        Task t = tasks[i];
        int vm_index = find_best_vm(t);

        if (vm_index != -1)
            allocate_task(&vms[vm_index], t);
        else
            printf("Task %d rejected (SJF)\n", t.id);
    }
}

// -----------------------------
// Priority Scheduling
// -----------------------------
void priority_schedule(Task tasks[], int n) {
    // Sort by highest priority first
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (tasks[j].priority < tasks[j + 1].priority) {
                Task temp = tasks[j];
                tasks[j] = tasks[j + 1];
                tasks[j + 1] = temp;
            }
        }
    }

    printf("\n--- Priority Scheduling ---\n");

    for (int i = 0; i < n; i++) {
        Task t = tasks[i];
        int vm_index = find_best_vm(t);

        if (vm_index != -1)
            allocate_task(&vms[vm_index], t);
        else
            printf("Task %d rejected (Priority)\n", t.id);
    }
}

// -----------------------------
// Release Resources
// -----------------------------
void release_resources() {
    for (int i = 0; i < vm_count; i++) {
        if (vms[i].used_cpu > 0)
            vms[i].used_cpu -= rand() % 2;

        if (vms[i].used_mem > 0)
            vms[i].used_mem -= rand() % 2;

        if (vms[i].used_cpu < 0) vms[i].used_cpu = 0;
        if (vms[i].used_mem < 0) vms[i].used_mem = 0;
    }
}

// -----------------------------
// Print Status
// -----------------------------
void print_status() {
    printf("\nVM STATUS:\n");
    for (int i = 0; i < vm_count; i++) {
        printf("VM %d -> CPU: %d/%d | MEM: %d/%d\n",
               vms[i].id,
               vms[i].used_cpu, vms[i].total_cpu,
               vms[i].used_mem, vms[i].total_mem);
    }
}

// -----------------------------
// Main
// -----------------------------
int main() {
    srand(time(NULL));
    init_vms();

    printf("=== Cloud Scheduling Simulation (SJF + Priority) ===\n");

    for (int t = 0; t < SIMULATION_TIME; t++) {
        printf("\n=============================\n");
        printf("Time Step %d\n", t);

        int task_count = rand() % 5 + 1;
        Task tasks[MAX_TASKS];

        // Generate tasks
        for (int i = 0; i < task_count; i++) {
            tasks[i] = generate_task(t * 10 + i);
            printf("Task %d -> CPU:%d MEM:%d Priority:%d\n",
                   tasks[i].id,
                   tasks[i].cpu_req,
                   tasks[i].mem_req,
                   tasks[i].priority);
        }

        // Choose algorithm
        if (t % 2 == 0)
            sjf_schedule(tasks, task_count);
        else
            priority_schedule(tasks, task_count);

        release_resources();
        print_status();
    }

    printf("\nSimulation Finished\n");
    return 0;
}