#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <sys/time.h>

#define MATRICES_NUMBER 3
#define INPUT1_NAME "a"
#define INPUT2_NAME "b"
#define OUTPUT_NAME "c"
#define MATRIX_NAME_EXT "_per_matrix"
#define ROW_NAME_EXT "_per_row"
#define ELEMENT_NAME_EXT "_per_element"

struct matrixStruct {
    int row, col;
    int (*m)[];
};
struct multiplicationStruct {
    int row, col, pad;
    int (*A)[];
    int (*B)[];
};
struct functionStruct {
    int cur_row, cur_col;
    int (*C)[];
    struct multiplicationStruct *ms;
};
struct heapStruct {
    struct matrixStruct *A;
    struct matrixStruct *B;
    struct matrixStruct *C[MATRICES_NUMBER];
    struct multiplicationStruct *D;
};

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
void *thread_element_multiplication(void *args){
    struct functionStruct *data = args;
    ((int (*)[data->ms->col])data->C)[data->cur_row][data->cur_col] = 0;
    for (int i = 0; i < data->ms->pad; ++i) {
        ((int (*)[data->ms->col])data->C)[data->cur_row][data->cur_col] += ((int (*)[data->ms->pad])data->ms->A)[data->cur_row][i] * ((int (*)[data->ms->col])data->ms->B)[i][data->cur_col];
    }
    free(data);
    pthread_exit(0);
}

void main_matrix_multiplication(struct multiplicationStruct *common_data, struct matrixStruct *output_matrix){
    pthread_t thread;
    struct functionStruct *special_data = malloc(sizeof(struct functionStruct));
    special_data->C = output_matrix->m;
    special_data->ms = common_data;
    pthread_create(&thread, NULL, thread_matrix_multiplication, special_data);
    pthread_join(thread, NULL);
}
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
