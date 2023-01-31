g++ main.cpp -o MIPSsim
./MIPSsim sample_test.txt simulation.txt
diff -cb simulation.txt simulation_test.txt >diff.txt
diff -cb disassembly.txt disassembly_test.txt >diff2.txt