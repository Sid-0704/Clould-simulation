#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "simulation_core.h"

#ifdef _WIN32
#include <windows.h>
#endif

// -----------------------------
// Global Variables
// -----------------------------
VM vms[MAX_VMS];
int vm_count = MAX_VMS;
static SchedulingAlgorithm g_algorithm = ALG_SJF;

#ifdef _WIN32
static StepResult g_step_result;
static int g_step_index = 0;
static int g_finished = 0;
static int g_simulation_started = 0;
static int g_paused = 0;
static int g_history_steps = 0;
static int g_accepted_history[SIMULATION_TIME];
static int g_rejected_history[SIMULATION_TIME];
static HWND g_sjf_button = NULL;
static HWND g_priority_button = NULL;
static HWND g_start_pause_button = NULL;
static HWND g_reset_button = NULL;

enum {
    ID_BUTTON_SJF = 1001,
    ID_BUTTON_PRIORITY = 1002,
    ID_BUTTON_START_PAUSE = 1003,
    ID_BUTTON_RESET = 1004
};

static void clear_history() {
    g_history_steps = 0;
    for (int i = 0; i < SIMULATION_TIME; i++) {
        g_accepted_history[i] = 0;
        g_rejected_history[i] = 0;
    }
}

static void record_step_history(const StepResult *result) {
    if (result == NULL) {
        return;
    }

    if (result->step < 0 || result->step >= SIMULATION_TIME) {
        return;
    }

    g_accepted_history[result->step] = result->accepted_tasks;
    g_rejected_history[result->step] = result->rejected_tasks;

    if (result->step + 1 > g_history_steps) {
        g_history_steps = result->step + 1;
    }
}

static void start_simulation(HWND hwnd) {
    init_vms();
    clear_history();
    g_step_index = 0;
    g_finished = 0;
    g_paused = 0;
    g_simulation_started = 1;
    simulate_step(g_step_index, g_algorithm, &g_step_result);
    record_step_history(&g_step_result);
    if (g_start_pause_button != NULL) {
        SetWindowTextA(g_start_pause_button, "Pause");
    }
    SetTimer(hwnd, 1, 1200, NULL);
    InvalidateRect(hwnd, NULL, TRUE);
}

static void pause_simulation(HWND hwnd) {
    if (!g_simulation_started || g_finished) {
        return;
    }

    if (!g_paused) {
        g_paused = 1;
        KillTimer(hwnd, 1);
        if (g_start_pause_button != NULL) {
            SetWindowTextA(g_start_pause_button, "Resume");
        }
    } else {
        g_paused = 0;
        if (g_start_pause_button != NULL) {
            SetWindowTextA(g_start_pause_button, "Pause");
        }
        SetTimer(hwnd, 1, 1200, NULL);
    }

    InvalidateRect(hwnd, NULL, TRUE);
}

static void reset_simulation(HWND hwnd) {
    KillTimer(hwnd, 1);
    init_vms();
    clear_history();
    g_step_index = 0;
    g_finished = 0;
    g_paused = 0;
    g_simulation_started = 0;
    if (g_start_pause_button != NULL) {
        SetWindowTextA(g_start_pause_button, "Start / Pause");
    }
    InvalidateRect(hwnd, NULL, TRUE);
}
#endif

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

void draw_performance_graph(HDC hdc, int x, int y, int w, int h) {
    int max_points = g_history_steps;
    int max_value = 1;
    char label[128];

    Rectangle(hdc, x, y, x + w, y + h);
    TextOutA(hdc, x + 8, y + 6, "Performance Graph (Accepted vs Rejected)", 40);

    for (int i = 0; i < max_points; i++) {
        if (g_accepted_history[i] > max_value) {
            max_value = g_accepted_history[i];
        }
        if (g_rejected_history[i] > max_value) {
            max_value = g_rejected_history[i];
        }
    }

    if (max_points > 0) {
        HPEN accepted_pen = CreatePen(PS_SOLID, 2, RGB(30, 170, 80));
        HPEN rejected_pen = CreatePen(PS_SOLID, 2, RGB(210, 70, 60));
        HPEN old_pen = (HPEN)SelectObject(hdc, accepted_pen);
        int graph_left = x + 10;
        int graph_top = y + 30;
        int graph_width = w - 20;
        int graph_height = h - 44;

        for (int i = 0; i < max_points; i++) {
            int px = graph_left + (i * graph_width) / ((max_points > 1) ? (max_points - 1) : 1);
            int py = graph_top + graph_height - (g_accepted_history[i] * graph_height) / max_value;
            if (i == 0) {
                MoveToEx(hdc, px, py, NULL);
            } else {
                LineTo(hdc, px, py);
            }
        }

        SelectObject(hdc, rejected_pen);
        for (int i = 0; i < max_points; i++) {
            int px = graph_left + (i * graph_width) / ((max_points > 1) ? (max_points - 1) : 1);
            int py = graph_top + graph_height - (g_rejected_history[i] * graph_height) / max_value;
            if (i == 0) {
                MoveToEx(hdc, px, py, NULL);
            } else {
                LineTo(hdc, px, py);
            }
        }

        SelectObject(hdc, old_pen);
        DeleteObject(accepted_pen);
        DeleteObject(rejected_pen);
    } else {
        TextOutA(hdc, x + 10, y + 32, "Start simulation to generate graph data.", 40);
    }

    TextOutA(hdc, x + 12, y + h - 16, "Green: Accepted   Red: Rejected", 31);
    snprintf(label, sizeof(label), "Steps recorded: %d", g_history_steps);
    TextOutA(hdc, x + w - 130, y + h - 16, label, (int)strlen(label));
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_CREATE:
            clear_history();
            g_simulation_started = 0;
            g_finished = 0;
            g_paused = 0;
            g_sjf_button = CreateWindowA("BUTTON", "SJF", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                         12, 620, 110, 34, hwnd, (HMENU)ID_BUTTON_SJF, GetModuleHandle(NULL), NULL);
            g_priority_button = CreateWindowA("BUTTON", "Priority", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                               132, 620, 110, 34, hwnd, (HMENU)ID_BUTTON_PRIORITY, GetModuleHandle(NULL), NULL);
            g_start_pause_button = CreateWindowA("BUTTON", "Start / Pause", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                                 252, 620, 150, 34, hwnd, (HMENU)ID_BUTTON_START_PAUSE, GetModuleHandle(NULL), NULL);
            g_reset_button = CreateWindowA("BUTTON", "Reset", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                                           412, 620, 110, 34, hwnd, (HMENU)ID_BUTTON_RESET, GetModuleHandle(NULL), NULL);
            return 0;

        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                case ID_BUTTON_SJF:
                    g_algorithm = ALG_SJF;
                    InvalidateRect(hwnd, NULL, TRUE);
                    return 0;
                case ID_BUTTON_PRIORITY:
                    g_algorithm = ALG_PRIORITY;
                    InvalidateRect(hwnd, NULL, TRUE);
                    return 0;
                case ID_BUTTON_START_PAUSE:
                    if (!g_simulation_started || g_finished) {
                        start_simulation(hwnd);
                    } else {
                        pause_simulation(hwnd);
                    }
                    return 0;
                case ID_BUTTON_RESET:
                    reset_simulation(hwnd);
                    return 0;
            }
            break;

        case WM_TIMER:
            if (!g_finished && !g_paused) {
                g_step_index++;
                if (g_step_index < SIMULATION_TIME) {
                    simulate_step(g_step_index, g_algorithm, &g_step_result);
                    record_step_history(&g_step_result);
                } else {
                    g_finished = 1;
                    KillTimer(hwnd, 1);
                    if (g_start_pause_button != NULL) {
                        SetWindowTextA(g_start_pause_button, "Start / Pause");
                    }
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

            if (!g_simulation_started) {
                TextOutA(hdc, 12, y, "Choose a scheduling algorithm to start the simulation.", 55);
                y += 22;
                if (g_algorithm == ALG_SJF)
                    TextOutA(hdc, 12, y, "Selected: SJF", 14);
                else
                    TextOutA(hdc, 12, y, "Selected: Priority", 19);
                y += 24;
            } else if (g_finished) {
                TextOutA(hdc, 12, y, "Simulation Finished", 19);
                y += 24;
            } else if (g_paused) {
                snprintf(line, sizeof(line), "Paused at step %d", g_step_result.step + 1);
                TextOutA(hdc, 12, y, line, (int)strlen(line));
                y += 20;

                if (g_step_result.algorithm == 0)
                    TextOutA(hdc, 12, y, "Algorithm: SJF", 14);
                else
                    TextOutA(hdc, 12, y, "Algorithm: Priority", 19);

                y += 24;
                TextOutA(hdc, 12, y, "Simulation is paused. Click Resume to continue or Reset to restart.", 70);
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
            events_rect.right = 420;
            events_rect.bottom = 590;
            DrawTextA(hdc, g_step_result.events, -1, &events_rect, DT_LEFT | DT_TOP | DT_WORDBREAK);

            draw_performance_graph(hdc, 440, 250, 390, 340);

            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_DESTROY:
            KillTimer(hwnd, 1);
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
        CW_USEDEFAULT, CW_USEDEFAULT, 860, 700,
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
    int use_console = 0;
    int gui_algorithm_selected = 0;

    srand(time(NULL));
    init_vms();

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--console") == 0) {
            use_console = 1;
        } else if (strcmp(argv[i], "--sjf") == 0) {
            g_algorithm = ALG_SJF;
            gui_algorithm_selected = 1;
        } else if (strcmp(argv[i], "--priority") == 0) {
            g_algorithm = ALG_PRIORITY;
            gui_algorithm_selected = 1;
        }
    }

#ifdef _WIN32
    if (use_console) {
        run_console_simulation(g_algorithm);
        return 0;
    }

    if (!gui_algorithm_selected) {
        g_algorithm = ALG_SJF;
    }

    return run_gui_simulation(GetModuleHandle(NULL), SW_SHOWDEFAULT);
#else
    (void)argc;
    (void)argv;
    run_console_simulation(g_algorithm);
    return 0;
#endif
}