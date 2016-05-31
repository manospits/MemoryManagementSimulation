#OPERATING SYSTEMS PROJECT
#Simulation of process memory placement
#Algorithms used best-fit, next-fit, buddy

Simulation runs by creating virtual process of size in range [lo,hi] using
uniform distribution and each process has to stay in memory for T cycles 2k+1
times where k range is [2,20]. The creation event of each  VP happens in t cycles
from the last creation event where t is the result of poisson distribution.

Compile with make and run with

./mem_sim --lo vp_lowest_size --hi vp_highest_size -t creation_time -T calculations_time -S memory_size --time cycles -a algorithm

algorithm can be 'b' for best-fit 'u' for buddy 'n' for next-fit

example execution:

./mem_sim --lo 32 --hi 512 -T 10 -t 1 -S 1024 --time 1000 -a u

Call with values

low: 32 high: 512 t for events: 1 T for calculations: 10 algorithm : u

-----Memory management simulation finished-----

-Mean of free usable spaces       : 84.9144

-Variance of free usable spaces   : 3053.55

-Average request size             : 77

-Number of requests               : 881

-Average residence stay           : 10.0092

-Time - memory Product efficiency : 0.664739


