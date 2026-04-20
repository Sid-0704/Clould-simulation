Cloud Scheduling Simulation

This project simulates CPU and memory allocation across multiple Virtual Machines (VMs) with two scheduling strategies:

- SJF (Shortest Job First)
- Priority Scheduling

The program now supports both:

- GUI mode (Windows window with live VM usage bars and events)
- Console mode (terminal output)

The GUI also shows a basic performance graph (Accepted vs Rejected tasks per time step) so you can quickly see how the selected algorithm performed over the run.

You can choose the scheduling algorithm in the GUI with buttons before the simulation starts.
The GUI also includes separate `Start / Pause` and `Reset` buttons, so the control buttons do not overlap the algorithm selection buttons.

You can also choose at startup in console mode:

- SJF (default)
- Priority Scheduling

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

- In the GUI, click `SJF` or `Priority` to select the algorithm, then use `Start / Pause` to run or pause the simulation.
- Use `Reset` to clear the current run and return to the initial state.

- Console mode:

```bash
cloud_sim.exe --console
```

- Console mode with Priority Scheduling:

```bash
cloud_sim.exe --console --priority
```



    