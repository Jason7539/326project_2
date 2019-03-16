# 326project_2

**To compile deleting the queue (You may need to delete the queue before running the DataHub.):**
- g++ -o delete deletequeue.cpp 

**To run:**
- ./delete

**To compile the DataHub (This must be ran before probes A, B, and C):**
- g++ -o hub main.cpp force_patch64.o

**To run:**
- ./hub

**Probes A, B, and Cshould be able to run in any order and concurrently after the DataHub.**

**To compile probeA:**
- g++ -o a probeA.cpp

**To run:**
- ./a

**To compile probeB:**
- g++ -o b probeB.cpp

**To run:**
- ./b

**To compile probeC:**
- g++ -o c probeC.cpp kill_patch64.o

**To run:**
- ./c
