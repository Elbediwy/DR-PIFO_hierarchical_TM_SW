# The DR-PIFO hierarchical Traffic Manager is implemented and deployed in this software programmable networking environment
We provide here an example of a case study applying two-level hierarchy, with Strict Prirority scheduling on the root (the first level), and Deficit Round Robin (DRR) on the leaf nodes (the second level).
The DR-PIFO also supports, by default, First-In-First-Out (FIFO) queues prior to the lowest level, (here, before the DRR level).
This prototype provides two-level hierarchy, with two leaf nodes/queues, and each leaf supports up to 50 different tarffic flows'/users' IDs. 

# To build and run:
1. Download the provided folders in "P4_simulation/" and save them in the main directory of your workspace.

2. Make sure these tools are installed in your workspace : Mininet, P4c, P4runtime and BMv2.
Optional : you can find these tools installed in the provided VM from https://github.com/p4lang/tutorials

3. Copy the files provided in "P4_simulation/BMv2 Files/" to the directory of the simple_switch in your system "behavioral-model/targets/simple_switch" (replace the existed files, if needed)

4. In the directory of BMv2 "behavioral-model/", run these commands : 
```bash
./autogen.sh
./configure
sudo make
sudo make install
sudo ldconfig
```
optional, in "behavioral-model/targets/simple_switch" and "behavioral-model/targets/simple_switch_grpc", you can run these commands:
```bash
sudo make
sudo make install
sudo ldconfig
```
5. In the DR-PIFO utils direcotry "P4_simulation/utils/user_externs_dr_pifo/", it is prefered to run these commands : 
```bash
sudo make clean
sudo make
```

6. In "P4_simulation/program/qos/", run these commands (A P4 program of the adopted case study is defined in the qos.p4 file) :
```bash
sudo make stop
sudo make clean
sudo make
```

7. Then, wait until the simulation is finished. (~ 30 mins)

# Useful information to apply more case studies
* We would like to refer you to the attached "pre-defined_interfaces.pdf" file on this directory, which includes the required input data and control signals to efficiently communicate and control with the deployed TM. (Note: these inputs can alter the behavior of the DR-PIFO TM to express different scheduling/shaping schemes)
* To find the log files of each switch, go to "P4_simulation/utils/program/qos/logs"
* To find the received packets by each receiving host, go to "P4_simulation/utils/program/qos/receiver_h'#host_id'", using the receiver/sending hosts log files, one can calculate the completion time of each flow.
* You can modify the main run file "P4_simulation/utils/run_execrise.py" to apply different workloads, https://github.com/Elbediwy/DR-PIFO_hierarchical_TM_SW/blob/ab058788aa63ed15169f3e3cdbbc90249e4b79d3/P4_simulation/utils/run_exercise.py#L361
* You can change the workload itself, from the workload assignment file "P4_simulation/utils/run_execrise.py" and the workload definitions on "P4_simulation/utils/program/workload"
* If you would like to change the underlying architecture (of the DR-PIFO TM), or extend it, you can modify the source code of our DR-PIFO on the "DR-PIFO_hierarhcical_TM_SW/BMv2 files/TM_buffer_dr_pifo.h" module.
