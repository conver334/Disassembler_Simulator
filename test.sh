g++ main.cpp -o main
./main sample_int.txt
diff -cb simulation.txt simulation_base.txt >diff.txt
diff -cb disassembly.txt disassembly_base.txt >diff2.txt