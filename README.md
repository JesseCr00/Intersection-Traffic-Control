# Proof of Concept of Distributed Mutual Exclusion for Intersection Traffic Control using OpenMPI

## Project Overview
This project presents a proof of concept of the distributed mutual exclusion algorithm for intersection traffic control using OpenMPI. The original algorithm (full citation and abstract is at the bottom of this document) is described in:

Wu, W., Zhang, J., Luo, A., & Cao, J. (2015). Distributed Mutual Exclusion Algorithms for Intersection Traffic Control. IEEE Transactions on Parallel and Distributed Systems, 26(1), 65-74. DOI: 10.1109/TPDS.2013.2297097

The original implementation focuses on real-life controllers in vehicular ad hoc networks, however this project adapts the concepts to a simulated environment using OpenMPI.

## Key Features
- Implementation of distributed mutual exclusion for traffic control, using OpenMPI to simulate cars
- Adaptation of the original algorithm from vehicular networks to a parallel computing environment
- Simulation of intersection traffic control without reliance on physical traffic light or intersection controller facilities

## Adaptation Notes
- Represents vehicle-to-vehicle communication with inter-process communication with OpenMPI, where a process represents a vehicle
- One difference in my implementation is that it will reuse processes as cars. 
- In addition to this, the code order and operations have to be different to accommodate for the differences between real life wireless communicators, and MPI processes running in a loop.

## Purpose
This repository is part of my personal portfolio, showcasing my skills in:

- Algorithm adaptation and implementation
- Parallel computing using OpenMPI
- Understanding and applying concepts from academic research to practical coding scenarios
  
It is intended for demonstration purposes and as a supplement to my resume.



## Usage
To build and run the code, navigate to root directory and use command:

```make```

To clean up build artifacts the the command:

```make clean```

To run without rebuilding, run command:

```mpirun -np 4 ./traffic_control 4```

## Requirements
- OpenMPI
- A C++ compiler with OpenMP and Address Sanitizer support (recent versions of GCC should work)
- Note: This has only been tested on Linux

## Future Work
Refactor/cleanup eventually.

## Contact
Jesse Cruickshank

[LinkedIn](www.linkedin.com/in/jesse-cruickshank)

## Original Paper Citation & Abstract
W. Wu, J. Zhang, A. Luo and J. Cao, "Distributed Mutual Exclusion Algorithms for Intersection Traffic Control," in IEEE Transactions on Parallel and Distributed Systems, vol. 26, no. 1, pp. 65-74, Jan. 2015, doi: 10.1109/TPDS.2013.2297097. 

Abstract: Traffic control at intersections is a key issue and hot research topic in intelligent transportation systems. Existing approaches, including traffic light scheduling and trajectory maneuver, are either inaccurate and inflexible or complicated and costly. More importantly, due to the dynamics of traffic, it is really difficult to obtain the optimal solution in a real-time way. Inspired by the emergence of vehicular ad hoc network, we propose a novel approach to traffic control at intersections. Via vehicle to vehicle or vehicle to infrastructure communications, vehicles can compete for the privilege of passing the intersection, i.e., traffic is controlled via coordination among vehicles. Such an approach is flexible and efficient. To realize the coordination among vehicles, we first model the problem as a new variant of the classic mutual exclusion problem, and then design algorithms to solve new problem. Both centralized and distributed algorithms are. We conduct extensive simulations to evaluate the performance of our proposed algorithms. The results show that, our approach is efficient and outperforms a reference algorithm based on optimal traffic light scheduling. Moreover, our approach does not rely on traffic light or intersection controller facilities, which makes it flexible and applicable to various kinds of intersections. 

keywords: {Vehicles;Algorithm design and analysis;Trajectory;Traffic control;Real-time systems;Heuristic algorithms;Concurrent computing;Mutual exclusion;intersection traffic control;intelligent transportation system;vehicular ad hoc network;distributed algorithm}, URL: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6747396&isnumber=6980150

## Usage Notice
This project is an adaptation of published research and is intended for educational and demonstration purposes only. It is not licensed for commercial use or redistribution. For information on using or implementing the original algorithm, please refer to the original paper and contact its authors.
