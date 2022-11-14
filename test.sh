g++ main.cpp -o main
./main sample.txt
diff -cb simulation.txt simulation-ori.txt >diff.txt
diff -cb disassembly.txt disassembly-ori.txt >diff2.txt