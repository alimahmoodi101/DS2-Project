type this in terminal to run each test 

BINARY TEST COMMAND:
g++ binaryTest.cpp src/BinaryHeap.cpp -I src -o test1.exe       
./test1.exe             

FIBHEAP TEST COMMAND:
g++ fibHeapTest.cpp src/FibHeap.cpp -I src -o test2.exe  
./test2.exe   

DIJIKSTRA TEST COMMAND:
g++ -std=c++17 graphTest.cpp src/Graph.cpp src/FibHeap.cpp src/BinaryHeap.cpp -Isrc -o test3.exe
./test3.exe   