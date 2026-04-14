#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_VMS 4
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
} Task;

// -----------------------------
// Global Variables
// -----------------------------
VM vms[MAX_VMS];
int vm_count = MAX_VMS;
int current_vm = 0;

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
    t.cpu_req = 1 + rand() % 2;   // 1–2 CPU
    t.mem_req = 1 + rand() % 3;   // 1–3 GB
    return t;
}

// -----------------------------
// Check Allocation Feasibility
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
// Round Robin Scheduling
// -----------------------------
void round_robin(Task t) {
    int attempts = 0;

    while (attempts < vm_count) {
        VM *vm = &vms[current_vm];

        if (can_allocate(vm, t)) {
            allocate_task(vm, t);
            current_vm = (current_vm + 1) % vm_count;
            return;
        }

        current_vm = (current_vm + 1) % vm_count;
        attempts++;
    }

    printf("Task %d rejected (No available VM)\n", t.id);
}

// -----------------------------
// Simulate Resource Release
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
// Print VM Status
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
// Main Function
// -----------------------------
int main() {
    srand(time(NULL));

    init_vms();

    printf("=== Round Robin Cloud Simulation ===\n");

    for (int t = 0; t < SIMULATION_TIME; t++) {
        printf("\n--- Time Step %d ---\n", t);

        int task_count = rand() % 3 + 1;

        for (int i = 0; i < task_count; i++) {
            Task task = generate_task(t * 10 + i);

            printf("Task %d -> CPU:%d MEM:%d\n",
                   task.id, task.cpu_req, task.mem_req);

            round_robin(task);
        }

        release_resources();
        print_status();
    }

    printf("\nSimulation Finished\n");
    return 0;
}