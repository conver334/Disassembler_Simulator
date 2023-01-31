g++ main.cpp -o main
./main sample_test.txt
diff -cb simulation.txt simulation_test.txt >diff.txt
diff -cb disassembly.txt disassembly_test.txt >diff2.txt