#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "simulation_core.h"

// -----------------------------
// Utility: Append Formatted Text
// -----------------------------
void appendf(char *buffer, size_t buffer_size, const char *fmt, ...) {
    size_t len;
    va_list args;

    if (buffer == NULL || buffer_size == 0) {
        return;
    }

    len = strlen(buffer);
    if (len >= buffer_size - 1) {
        return;
    }

    va_start(args, fmt);
    vsnprintf(buffer + len, buffer_size - len, fmt, args);
    va_end(args);
}

// -----------------------------
// Initialize VMs
// -----------------------------
void init_vms() {
    for (int i = 0; i < vm_count; i++) {
        vms[i].id = i;
        vms[i].total_cpu = 4 + rand() % 3;   // 4-6 CPU
        vms[i].total_mem = 8 + rand() % 5;   // 8-12 GB
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
    t.priority = rand() % 10; // 0-9 (higher = more priority)
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
void allocate_task(VM *vm, Task t, char *events, size_t events_size) {
    vm->used_cpu += t.cpu_req;
    vm->used_mem += t.mem_req;

    appendf(events, events_size, "Task %d allocated to VM %d\n", t.id, vm->id);
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

void sjf_schedule(Task tasks[], int n, char *events, size_t events_size) {
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

    appendf(events, events_size, "--- SJF Scheduling ---\n");

    for (int i = 0; i < n; i++) {
        Task t = tasks[i];
        int vm_index = find_best_vm(t);

        if (vm_index != -1)
            allocate_task(&vms[vm_index], t, events, events_size);
        else
            appendf(events, events_size, "Task %d rejected (SJF)\n", t.id);
    }
}

// -----------------------------
// Priority Scheduling
// -----------------------------
void priority_schedule(Task tasks[], int n, char *events, size_t events_size) {
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

    appendf(events, events_size, "--- Priority Scheduling ---\n");

    for (int i = 0; i < n; i++) {
        Task t = tasks[i];
        int vm_index = find_best_vm(t);

        if (vm_index != -1)
            allocate_task(&vms[vm_index], t, events, events_size);
        else
            appendf(events, events_size, "Task %d rejected (Priority)\n", t.id);
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
// Simulate One Step
// -----------------------------
void simulate_step(int step, StepResult *result) {
    Task sched_tasks[MAX_TASKS];

    result->step = step;
    result->algorithm = (step % 2 == 0) ? 0 : 1;
    result->task_count = rand() % 5 + 1;
    result->events[0] = '\0';

    for (int i = 0; i < result->task_count; i++) {
        result->tasks[i] = generate_task(step * 10 + i);
        sched_tasks[i] = result->tasks[i];
    }

    if (result->algorithm == 0)
        sjf_schedule(sched_tasks, result->task_count, result->events, sizeof(result->events));
    else
        priority_schedule(sched_tasks, result->task_count, result->events, sizeof(result->events));

    release_resources();
}

// -----------------------------
// Console Runner
// -----------------------------
void run_console_simulation() {
    StepResult result;

    printf("=== Cloud Scheduling Simulation (SJF + Priority) ===\n");

    for (int t = 0; t < SIMULATION_TIME; t++) {
        simulate_step(t, &result);

        printf("\n=============================\n");
        printf("Time Step %d\n", t);

        for (int i = 0; i < result.task_count; i++) {
            printf("Task %d -> CPU:%d MEM:%d Priority:%d\n",
                   result.tasks[i].id,
                   result.tasks[i].cpu_req,
                   result.tasks[i].mem_req,
                   result.tasks[i].priority);
        }

        printf("\n%s", result.events);
        print_status();
    }

    printf("\nSimulation Finished\n");
}
