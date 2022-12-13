g++ main.cpp -o MIPSsim
./MIPSsim sample2.txt simulation.txt
diff -cb simulation.txt simulation2-ori.txt >diff.txt