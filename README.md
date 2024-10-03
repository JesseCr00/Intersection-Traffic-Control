# OpenMPI Adaptation of Distributed Mutual Exclusion for Intersection Traffic Control

## Project Overview
This project presents an adaptation and implementation of the distributed mutual exclusion algorithm for intersection traffic control using OpenMPI. The original algorithm is described in:

Wu, W., Zhang, J., Luo, A., & Cao, J. (2015). Distributed Mutual Exclusion Algorithms for Intersection Traffic Control. IEEE Transactions on Parallel and Distributed Systems, 26(1), 65-74. DOI: 10.1109/TPDS.2013.2297097

While the original implementation focuses on real-life controllers in vehicular ad hoc networks, this project adapts the concepts to a simulated environment using OpenMPI, demonstrating skills in parallel computing and algorithm adaptation.

## Key Features
- Implementation of distributed mutual exclusion for traffic control using OpenMPI
- Adaptation of the original algorithm from vehicular networks to a parallel computing environment
- Simulation of intersection traffic control without reliance on physical traffic light or intersection controller facilities

## Purpose
This repository is part of my personal portfolio, showcasing my skills in:

- Algorithm adaptation and implementation
- Parallel computing using OpenMPI
- Understanding and applying concepts from academic research to practical coding scenarios
  
It is intended for demonstration purposes and as a supplement to my resume.

## Original Paper
W. Wu, J. Zhang, A. Luo and J. Cao, "Distributed Mutual Exclusion Algorithms for Intersection Traffic Control," in IEEE Transactions on Parallel and Distributed Systems, vol. 26, no. 1, pp. 65-74, Jan. 2015, doi: 10.1109/TPDS.2013.2297097. 
Abstract: Traffic control at intersections is a key issue and hot research topic in intelligent transportation systems. Existing approaches, including traffic light scheduling and trajectory maneuver, are either inaccurate and inflexible or complicated and costly. More importantly, due to the dynamics of traffic, it is really difficult to obtain the optimal solution in a real-time way. Inspired by the emergence of vehicular ad hoc network, we propose a novel approach to traffic control at intersections. Via vehicle to vehicle or vehicle to infrastructure communications, vehicles can compete for the privilege of passing the intersection, i.e., traffic is controlled via coordination among vehicles. Such an approach is flexible and efficient. To realize the coordination among vehicles, we first model the problem as a new variant of the classic mutual exclusion problem, and then design algorithms to solve new problem. Both centralized and distributed algorithms are. We conduct extensive simulations to evaluate the performance of our proposed algorithms. The results show that, our approach is efficient and outperforms a reference algorithm based on optimal traffic light scheduling. Moreover, our approach does not rely on traffic light or intersection controller facilities, which makes it flexible and applicable to various kinds of intersections. 
keywords: {Vehicles;Algorithm design and analysis;Trajectory;Traffic control;Real-time systems;Heuristic algorithms;Concurrent computing;Mutual exclusion;intersection traffic control;intelligent transportation system;vehicular ad hoc network;distributed algorithm}, URL: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6747396&isnumber=6980150

## Adaptation Notes
This implementation translates the concepts of vehicle-to-vehicle communication in the original paper to inter-process communication using OpenMPI. The simulation environment replaces physical vehicles with processes, demonstrating the algorithm's principles in a parallel computing context.

## Usage
[TODO: Instructions on how to compile and run]

## Requirements
- OpenMPI
- [TODO: Any other libraries or tools required]

## Results and Performance
[TODO]

## Future Work
[TODO]

## Contact
Jesse Cruickshank
[LinkedIn](www.linkedin.com/in/jesse-cruickshank)

## Acknowledgement
This project is based on research by Wu et al. The original paper can be found at: https://ieeexplore.ieee.org/stamp/stamp.jsp?tp=&arnumber=6747396&isnumber=6980150

## Usage Notice
This project is an adaptation of published research and is intended for educational and demonstration purposes only. It is not licensed for commercial use or redistribution. For information on using or implementing the original algorithm, please refer to the original paper and contact its authors.
