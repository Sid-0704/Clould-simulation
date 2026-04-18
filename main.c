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

#ifdef _WIN32
static StepResult g_step_result;
static int g_step_index = 0;
static int g_finished = 0;
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