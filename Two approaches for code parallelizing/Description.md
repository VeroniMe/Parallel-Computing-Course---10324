The purpose of this exercise is to implement a simple application with Dynamic and Static Task Pool management.

We asked to implement two approaches to parallelize the code that make some heavy calculations:
- Using Static Task Pool approach to solve the problem
- Implement Dynamic Task Pool Approach for parallel solution

# Parallel Computing Performance Analysis  

This repository contains an analysis of execution times for different **task allocation strategies** in a parallel computing environment. The experiments compare **Static Task Pool** and **Dynamic Task Pool** approaches with varying numbers of **slave processes**.  

## Execution Results  

### Sequential Solution  
- **Execution Time:** 13.5210s  
- This serves as the baseline for comparison, with no parallelism applied.  

### Static Task Pool  
- **2 Slaves:** Execution time **7.88s**  
- **4 Slaves:** Execution time **8.13s**  
- **20 Slaves:** Execution time **8.59s**  

With a **static task pool**, increasing the number of slaves does not necessarily improve performance. This happens because **tasks are not dynamically balanced**, leading to one or more slaves receiving heavier calculations, delaying the overall execution. We can see that one of the slaves got big numbers to compute, slowing down all others.  

### Dynamic Task Pool  
- **2 Slaves:** Execution time **6.88s**  
- **4 Slaves:** Execution time **5.17s**  
- **20 Slaves:** Execution time **5.85s**  

With a **dynamic task pool**, increasing the number of slaves generally leads to improved performance due to **better workload distribution**. However, at **20 slaves**, the execution time increases slightly instead of improving further. This is because the number of slaves exceeds the number of CPU cores, causing excessive **CPU contention and race conditions**, where the processes compete for resources and slow each other down.  

## Key Findings  
- **Static Task Pool suffers from load imbalance**, causing delays in execution.  
- **Dynamic Task Pool improves performance by distributing tasks more efficiently**.  
- **Too many slaves can be counterproductive** due to **CPU contention**.  
- **Optimal performance was achieved with 4 slaves in the dynamic model**, balancing parallel execution and available CPU cores.  

