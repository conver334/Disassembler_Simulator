g++ main.cpp -o MIPSsim
./MIPSsim t1.txt simulation.txt
diff -cb simulation.txt t1_sim.txt >diff.txt
diff -cb disassembly.txt t1_dis.txt >diff2.txt