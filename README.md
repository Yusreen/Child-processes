Algorithm:
The controller (parent) process should create N (ϵ [1, 8]), worker processes using fork() and other appropriate system calls, 
as you see fit. To facilitate the inter-process communication, there should be N pairs of pipes through which the controller assigns
jobs to associated worker processes and receives results back from them when the calculation is completed.
More specifically, the controller process will initially assign the first N pieces to the N worker processes to do their respective 
calculation. Whenever a process has done its calculation of the area of another trapezoid, it will send it back to the controller. 
Upon receiving such a piece, the controller process will update the sum accordingly, and, if there are still un-calculated pieces, 
the controller will also assign the next piece to the just returned worker process.
After the controller has received all the 32 results, it prints out the final answer, which should be within a reasonable range of
0.7850 (i.e., π/4).
