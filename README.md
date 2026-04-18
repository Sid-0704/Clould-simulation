Cloud Scheduling Simulation

This project simulates CPU and memory allocation across multiple Virtual Machines (VMs) with two scheduling strategies:

- SJF (Shortest Job First)
- Priority Scheduling

The program now supports both:

- GUI mode (Windows window with live VM usage bars and events)
- Console mode (terminal output)

Project Structure

- main.c: GUI window handling and application startup logic
- simulation_core.c: core scheduling and simulation functions (SJF, Priority, VM allocation, resource release)

Build (Windows, MinGW GCC)

```bash
gcc main.c simulation_core.c -o cloud_sim.exe -lgdi32
```

Run

- GUI mode (default):
.\cloud_sim.exe

- Console mode:

```bash
cloud_sim.exe --console
```



    