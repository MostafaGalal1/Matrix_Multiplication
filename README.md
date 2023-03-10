# Matrix_Multiplication


## Description

C program that performs matrix multiplication using three approaches the ordinary one which uses one thread, multiplying for row per thread and multiplying for element per thread. The program reads input matrices from files, multiplies them using a specified number of threads, and writes the resulting matrix to an output file. the program demonstrates how to perform matrix multiplication using multiple threads in C. However, it is worth noting that this implementation may not be the most efficient or optimal for all use cases, and additional optimizations may be necessary depending on the input matrices and system resources.


## Specifications

### Compile and Run
  This is linux based program as it uses some POSIX libraries in reading the files. However, it could be adjusted to work on other operating systems as Windows or macOS.
 Compile and run the c file using an IDE or terminal.
  
  The program accepts arguments for required input and desired output files names in the form of: ./MatMultp In1 In2 Out. If the program runned without arguments then the default names are a and b for input files respectively (make sure of the ordering as matrix multiplication is not commutative) and c for the output file.  

### Data structures

The program uses four data structures: matrixStruct, multiplicationStruct, functionStruct, and heapStruct as follows:

  * `matrixStruct`: structure stores a matrix as a two-dimensional array and its dimensions.
  * `multiplicationStruct`: structure stores the indices of a matrix that a thread should multiply with another matrix.
  * `functionStruct`: structure`: stores information about a thread's starting row and column and the resulting matrix.
  * `heapStruct`: structure stores pointers to the two matrices to multiply and the result matrix.

### Main functions

`main` function creates a struct multiplicationStruct with the input matrices A and B, the size of the matrices, and the padding factor. It then creates a new matrix struct C with the dimensions of the result matrix and passes both structs to the main_multiplication functions, which create new threads that executes the matrix multiplication.

The `main_row_multiplication` function is similar to `main_matrix_multiplication` but creates one thread per row instead of one thread for the entire matrix. Also `main_element_multiplication` function creates one thread per element in the result matrix.

The functions `thread_matrix_multiplication`, `thread_row_multiplication`, and `thread_element_multiplication` are the functions executed by the threads, each corresponds to one approach. They take a pointer to a struct functionStruct that contains the necessary data for the thread to perform its computation.

The `read_file` function reads a matrix from a text file with a given file name and stores it in a matrix struct.

The `write_file` function writes a matrix to a text file with a given file name and extension.

The 'free_heap' function releases resources and data allocated in the heap before program termination to avoid memory leak, It uses `heapStruct` to free the memory in the heap pointed to by this struct.

### Comparison
Comparison table of multiplying matrix using: thread by matrix, thread by row, and thread by element:

| Method | Advantages	| Disadvantages |
| ------ | ---------- | ------------- |
| One thread | Simple implementation | Slow for large matrices |
| Thread by row |	Faster for large matrices compared to one thread approach | Threads may finish at different times leading to waiting | 
| Thread by element |	Fastest approach for large matrices |	Requires careful implementation to avoid race conditions |

  * Note that the performance of these methods can vary depending on the hardware and software environment in which they are run. Additionally, the optimal method may vary depending on the specific characteristics of the matrix being multiplied (e.g. size, sparsity, etc.).


## Links

### Video
  * https://drive.google.com/file/d/1oBE4ZMsqSbuffoR65sC3dNOwKjc7jsFR/view?usp=sharing
  
### Report
  * None yet


## References

  * https://github.com/KhaledElTahan/Operating-Systems/tree/master/Labs/lab2
  * https://dotnettutorials.net/lesson/stack-vs-heap-memory/
  * https://stackoverflow.com/questions/10116368/heap-allocate-a-2d-array-not-array-of-pointers
  * https://www.javatpoint.com/atoi-function-in-c
  * https://www.geeksforgeeks.org/multithreading-in-c/
  * https://stackoverflow.com/questions/11624545/how-to-make-main-thread-wait-for-all-child-threads-finish
