g++ main.cpp -o main
./main sample_int.txt
diff -cb simulation.txt simulation_int.txt >diff.txt
diff -cb disassembly.txt disassembly_int.txt >diff2.txt