A sloppy counter represents one logical counter as a single shared central counter and a set of per-core counts of spare references. The sloppy counter works by representing... 
- A single logical counter via numerous local physical counters
- one per CPU core
- A single global counter 
There are locks: 
- One for each local counter and one for the global counter

Run the program by first compiling the executable with make, then calling ./sloppySim. This will run it with default values

Defaults:
N_Threads - The number of threads to use, defaults to 2
Sloppiness - How many events before updating global counter, defaults to 10
work_time - The avg work time in milliseconds, defaults 10
work_iterations - The number of iterations per thread, defaults to 100
CPU_BOUND - Make thread work for "work time" on a CPU, defaults to false
Do_Logging - Print current settings and get logging info, defaults to false