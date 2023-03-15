#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

//Constants and macros used in the code.
#define MATRICES_NUMBER 3
#define INPUT1_NAME "a"
#define INPUT2_NAME "b"
#define OUTPUT_NAME "c"
#define MATRIX_NAME_EXT "_per_matrix"
#define ROW_NAME_EXT "_per_row"
#define ELEMENT_NAME_EXT "_per_element"

/*
 * This is a struct that defines a matrix.
 * The row and col variables store the dimensions of the matrix.
 * m is a pointer to the matrix data.
 */
struct matrixStruct {
    int row, col;
    int (*m)[];
};

/*
 * This is a struct that defines the data necessary for matrix multiplication.
 * row and col store the dimensions of the result matrix>
 * pad is the shared dimension of the two input matrices, and A and B are pointers to the two input matrices.
 */
struct multiplicationStruct {
    int row, col, pad;
    int (*A)[];
    int (*B)[];
};

/*
 * This is a struct that defines the data necessary for each thread of matrix multiplication.
 * cur_row and cur_col store the current row and column of the output matrix being computed, C is a pointer to the output matrix.
 * ms is a pointer to the struct containing the data necessary for matrix multiplication.
 */
struct functionStruct {
    int cur_row, cur_col;
    int (*C)[];
    struct multiplicationStruct *ms;
};

/*
 * This is a struct that defines the heap of matrices used in the program.
 * A and B are pointers to the two input matrices, C is an array of pointers to the output matrices,
 * D is a pointer to the struct containing the data necessary for matrix multiplication.
 */
struct heapStruct {
    struct matrixStruct *A;
    struct matrixStruct *B;
    struct matrixStruct *C[MATRICES_NUMBER];
    struct multiplicationStruct *D;
};

/*
 * This function reads a matrix from a file.
 * file_name is the name of the file
 */
int read_file(char *file_name, struct matrixStruct *matrix){
    FILE *input_file;
    char * line = NULL, full_name[256];
    size_t len = 0;
    int row, col;

    strcpy(full_name, file_name);
    strcat(full_name, ".txt");
    input_file = fopen(full_name, "r");
    if (input_file == NULL)
        return -1;

    getline(&line, &len, input_file);
    strtok(line, "=");
    row = atoi(strtok(NULL, " "));
    strtok(NULL, "=");
    col = atoi(strtok(NULL, "\0"));

    matrix->row = row;
    matrix->col = col;

    int (*array)[col] = malloc(row * col * sizeof(int));
    int row_c = 0, col_c = 0;
    while (getline(&line, &len, input_file) != -1) {
        char *num = strtok(line, "\t");

        while (col_c < col && num != NULL) {
            array[row_c][col_c] = atoi(num);
            num = strtok(NULL, "\t");
            col_c++;
        }

        row_c++;
        col_c = 0;
    }

    fclose(input_file);
    if (line)
        free(line);

    matrix->m = array;
    return 0;
}

/*
 * This function writes a matrix to a file.
 * Takes a file name, an extension, and a matrix as input, and outputs the matrix to a text file.
 * The matrix is written row by row, with each row on a separate line.
 */
void write_file(char *file_name, char *extension, struct matrixStruct*matrix){
    FILE *output_file;
    char full_name[256];

    strcpy(full_name, file_name);
    strcat(full_name, extension);
    strcat(full_name, ".txt");
    output_file = fopen(full_name, "w");

    if (output_file == NULL)
        return;

    fprintf(output_file, "row=%d col=%d\n", matrix->row, matrix->col);
    for (int i = 0; i < matrix->row; ++i) {
        for (int j = 0; j < matrix->col; ++j) {
            fprintf(output_file, "%d ", ((int (*)[matrix->col])matrix->m)[i][j]);
        }
        fprintf(output_file, "\n");
    }

    fclose(output_file);
}

/*
 * This function is called by main_matrix_multiplication and performs matrix multiplication on the whole matrix using multi-threading.
 * Takes a structure that contains the matrices A, B, and C, as well as the number of rows, columns, and padding.
 * Performs the multiplication using nested loops.
 */
void *thread_matrix_multiplication(void *args){
    struct functionStruct *data = args;
    for (int i = 0; i < data->ms->row; ++i) {
        for (int j = 0; j < data->ms->col; ++j) {
            ((int (*)[data->ms->col]) data->C)[i][j] = 0;
            for (int k = 0; k < data->ms->pad; ++k) {
                ((int (*)[data->ms->col]) data->C)[i][j] += ((int (*)[data->ms->pad]) data->ms->A)[i][k] * ((int (*)[data->ms->col]) data->ms->B)[k][j];
            }
        }
    }
    free(data);
    pthread_exit(0);
}

/*
 * This function is called by main_row_multiplication and performs matrix multiplication row by row using multi-threading.
 * Takes a structure that contains the matrices A, B, and C, as well as the number of rows, columns, and padding.
 * Performs the multiplication on one row at a time using nested loops.
 */
void *thread_row_multiplication(void *args){
    struct functionStruct *data = args;
    for (int i = 0; i < data->ms->col; ++i) {
        ((int (*)[data->ms->col])data->C)[data->cur_row][i] = 0;
        for (int j = 0; j < data->ms->pad; ++j) {
            ((int (*)[data->ms->col])data->C)[data->cur_row][i] += ((int (*)[data->ms->pad])data->ms->A)[data->cur_row][j] * ((int (*)[data->ms->col])data->ms->B)[j][i];
        }
    }
    free(data);
    pthread_exit(0);
}

/*
 * This function is called by main_element_multiplication and performs matrix multiplication element by element using multi-threading.
 * Takes a structure that contains the matrices A, B, and C, as well as the number of rows, columns, and padding.
 * Performs the multiplication on one element at a time using nested loops.
 */
void *thread_element_multiplication(void *args){
    struct functionStruct *data = args;
    ((int (*)[data->ms->col])data->C)[data->cur_row][data->cur_col] = 0;
    for (int i = 0; i < data->ms->pad; ++i) {
        ((int (*)[data->ms->col])data->C)[data->cur_row][data->cur_col] += ((int (*)[data->ms->pad])data->ms->A)[data->cur_row][i] * ((int (*)[data->ms->col])data->ms->B)[i][data->cur_col];
    }
    free(data);
    pthread_exit(0);
}

/*
 * This function is called by the main function and performs matrix multiplication on the whole matrix using a single thread.
 * Takes the matrices A and B, as well as the number of rows, columns, and padding.
 * Calls thread_matrix_multiplication to perform the multiplication.
 */
void main_matrix_multiplication(struct multiplicationStruct *common_data, struct matrixStruct *output_matrix){
    pthread_t thread;
    struct functionStruct *special_data = malloc(sizeof(struct functionStruct));
    special_data->C = output_matrix->m;
    special_data->ms = common_data;
    pthread_create(&thread, NULL, thread_matrix_multiplication, special_data);
    pthread_join(thread, NULL);
}

/*
 * This function is called by the main function and performs matrix multiplication row by row using multi-threading.
 * Takes the matrices A and B, as well as the number of rows, columns, and padding.
 * Calls thread_row_multiplication to perform the multiplication.
 */
void main_row_multiplication(struct multiplicationStruct *common_data, struct matrixStruct *output_matrix){
    int threads_num = common_data->row;
    pthread_t threads[threads_num];

    for (int i = 0; i < common_data->row; ++i) {
        struct functionStruct *special_data = malloc(sizeof(struct functionStruct));
        special_data->cur_row = i;
        special_data->C = output_matrix->m;
        special_data->ms = common_data;
        pthread_create(&threads[i], NULL, thread_row_multiplication, special_data);
    }

    for (int i = 0; i < threads_num; ++i) {
        pthread_join(threads[i], NULL);
    }
}

/*
 * This function is called by the main function and performs matrix multiplication element by element using multi-threading.
 * Takes the matrices A and B, as well as the number of rows, columns, and padding.
 * Calls thread_element_multiplication to perform the multiplication.
 */
void main_element_multiplication(struct multiplicationStruct *common_data, struct matrixStruct *output_matrix){
    int threads_num = common_data->row * common_data->col;
    pthread_t threads[threads_num];

    for (int i = 0; i < common_data->row; ++i) {
        for (int j = 0; j < common_data->col; ++j) {
            struct functionStruct *special_data = malloc(sizeof(struct functionStruct));
            special_data->cur_row = i;
            special_data->cur_col = j;
            special_data->C = output_matrix->m;
            special_data->ms = common_data;
            pthread_create(&threads[i*common_data->col+j], NULL, thread_element_multiplication, special_data);
        }
    }

    for (int i = 0; i < threads_num; ++i) {
        pthread_join(threads[i], NULL);
    }
}

/*
 * This function frees all the memory allocated on the heap by the program.
 * Takes a structure that contains pointers to matrices A, B, and C, as well as the number of matrices.
 * Frees the memory allocated for each matrix.
 */
void free_heap(struct heapStruct *heap){
    free(heap->A->m);
    free(heap->A);
    free(heap->B->m);
    free(heap->B);
    for (int i = 0; i < MATRICES_NUMBER; ++i) {
        free(heap->C[i]->m);
        free(heap->C[i]);
    }
    free(heap->D);
    free(heap);
}

/*
 * This function prompts the user to input the number of rows and columns for the matrices.
 * Calls the corresponding functions to perform the multiplication and writes the resulting matrices to corresponding files.
 * Calls free_heap to free the memory allocated on the heap by the program.
 */
int main(int argc, char *argv[]) {

    struct timeval stop, start;
    char *input1 = INPUT1_NAME;
    char *input2 = INPUT2_NAME;
    char *output = OUTPUT_NAME;

    struct matrixStruct *A = malloc(sizeof(struct matrixStruct)), *B = malloc(sizeof(struct matrixStruct));
    void (*funcs[3])(struct multiplicationStruct *, struct matrixStruct *) = {main_matrix_multiplication, main_row_multiplication, main_element_multiplication};

    if (argc == MATRICES_NUMBER+1){
        input1 = argv[1];
        input2 = argv[2];
        output = argv[3];
    }

    if (read_file(input1, A) || read_file(input2, B)) {
        printf("Error reading files");
        free(A);
        free(B);
        return 0;
    }

    struct matrixStruct *C[MATRICES_NUMBER];
    struct multiplicationStruct *data = malloc(sizeof(struct multiplicationStruct));
    struct heapStruct *heap = malloc(sizeof(struct heapStruct));

    heap->A = A;
    heap->B = B;

    for (int i = 0; i < MATRICES_NUMBER; ++i) {
        C[i] = malloc(sizeof(struct matrixStruct));
        C[i]->row = A->row;
        C[i]->col = B->col;
        C[i]->m = malloc(C[i]->row * C[i]->col * sizeof(int));
        heap->C[i] = C[i];
    }

    data->row = A->row;
    data->col = B->col;
    data->pad = A->col;
    data->A = A->m;
    data->B = B->m;
    heap->D = data;

    if (A->col != B->row){
        printf("Sizes of matrices are incompatible");
        free_heap(heap);
        return 0;
    }

    for (int i = 0; i < MATRICES_NUMBER; ++i) {
        gettimeofday(&start, NULL);
        funcs[i](data, C[i]);
        gettimeofday(&stop, NULL);
        printf("Seconds taken %lu\n", stop.tv_sec - start.tv_sec);
        printf("Microseconds taken: %lu\n", stop.tv_usec - start.tv_usec);
    }

    write_file(output, MATRIX_NAME_EXT, C[0]);
    write_file(output, ROW_NAME_EXT, C[1]);
    write_file(output, ELEMENT_NAME_EXT, C[2]);

    free_heap(heap);
    return 0;
}
