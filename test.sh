g++ main.cpp -o main
./main sample.txt
diff -c simulation.txt simulation-ori.txt >diff.txt
diff -c disassembly.txt disassembly-ori.txt >diff2.txt