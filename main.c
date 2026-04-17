#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <stdarg.h>

#ifdef _WIN32
#include <windows.h>
#endif

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

typedef struct {
    int step;
    int algorithm; /* 0 = SJF, 1 = Priority */
    Task tasks[MAX_TASKS];
    int task_count;
    char events[2048];
} StepResult;

// -----------------------------
// Global Variables
// -----------------------------
VM vms[MAX_VMS];
int vm_count = MAX_VMS;

#ifdef _WIN32
static StepResult g_step_result;
static int g_step_index = 0;
static int g_finished = 0;
#endif

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

#ifdef _WIN32
// -----------------------------
// GUI Drawing Helpers
// -----------------------------
void draw_bar(HDC hdc, int x, int y, int w, int h, int used, int total, COLORREF color) {
    RECT frame = {x, y, x + w, y + h};
    RECT fill;
    HBRUSH fill_brush;

    Rectangle(hdc, frame.left, frame.top, frame.right, frame.bottom);

    if (total <= 0) {
        return;
    }

    fill = frame;
    fill.right = x + (w * used) / total;
    if (fill.right < fill.left) {
        fill.right = fill.left;
    }

    fill_brush = CreateSolidBrush(color);
    FillRect(hdc, &fill, fill_brush);
    DeleteObject(fill_brush);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            g_step_index = 0;
            g_finished = 0;
            simulate_step(g_step_index, &g_step_result);
            SetTimer(hwnd, 1, 1200, NULL);
            return 0;

        case WM_TIMER:
            if (!g_finished) {
                g_step_index++;
                if (g_step_index < SIMULATION_TIME) {
                    simulate_step(g_step_index, &g_step_result);
                } else {
                    g_finished = 1;
                    KillTimer(hwnd, 1);
                }
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;

        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            char line[256];
            RECT events_rect;
            int y = 12;

            SetBkMode(hdc, TRANSPARENT);
            TextOutA(hdc, 12, y, "Cloud Scheduling Simulation (GUI)", 33);
            y += 26;

            if (g_finished) {
                TextOutA(hdc, 12, y, "Simulation Finished", 19);
                y += 24;
            } else {
                snprintf(line, sizeof(line), "Time Step: %d / %d", g_step_result.step + 1, SIMULATION_TIME);
                TextOutA(hdc, 12, y, line, (int)strlen(line));
                y += 20;

                if (g_step_result.algorithm == 0)
                    TextOutA(hdc, 12, y, "Algorithm: SJF", 14);
                else
                    TextOutA(hdc, 12, y, "Algorithm: Priority", 19);

                y += 24;
                TextOutA(hdc, 12, y, "Generated Tasks:", 16);
                y += 20;

                for (int i = 0; i < g_step_result.task_count; i++) {
                    snprintf(line, sizeof(line), "Task %d -> CPU:%d MEM:%d Priority:%d",
                             g_step_result.tasks[i].id,
                             g_step_result.tasks[i].cpu_req,
                             g_step_result.tasks[i].mem_req,
                             g_step_result.tasks[i].priority);
                    TextOutA(hdc, 20, y, line, (int)strlen(line));
                    y += 18;
                }
                y += 8;
            }

            TextOutA(hdc, 12, y, "VM Utilization:", 15);
            y += 20;

            for (int i = 0; i < vm_count; i++) {
                snprintf(line, sizeof(line), "VM %d CPU %d/%d", vms[i].id, vms[i].used_cpu, vms[i].total_cpu);
                TextOutA(hdc, 20, y, line, (int)strlen(line));
                draw_bar(hdc, 170, y - 2, 180, 14, vms[i].used_cpu, vms[i].total_cpu, RGB(80, 160, 220));
                y += 20;

                snprintf(line, sizeof(line), "VM %d MEM %d/%d", vms[i].id, vms[i].used_mem, vms[i].total_mem);
                TextOutA(hdc, 20, y, line, (int)strlen(line));
                draw_bar(hdc, 170, y - 2, 180, 14, vms[i].used_mem, vms[i].total_mem, RGB(90, 190, 110));
                y += 26;
            }

            y += 6;
            TextOutA(hdc, 12, y, "Last Events:", 12);
            y += 18;
            events_rect.left = 12;
            events_rect.top = y;
            events_rect.right = 760;
            events_rect.bottom = 560;
            DrawTextA(hdc, g_step_result.events, -1, &events_rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int run_gui_simulation(HINSTANCE hInstance, int nCmdShow) {
    const char CLASS_NAME[] = "CloudSimWindowClass";
    WNDCLASSA wc = {0};
    HWND hwnd;
    MSG msg;

    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassA(&wc);

    hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Cloud Simulation GUI",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 820, 640,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 1;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return (int)msg.wParam;
}
#endif

// -----------------------------
// Main
// -----------------------------
int main(int argc, char *argv[]) {
    srand(time(NULL));
    init_vms();

#ifdef _WIN32
    if (argc > 1 && strcmp(argv[1], "--console") == 0) {
        run_console_simulation();
        return 0;
    }

    return run_gui_simulation(GetModuleHandle(NULL), SW_SHOWDEFAULT);
#else
    (void)argc;
    (void)argv;
    run_console_simulation();
    return 0;
#endif
}