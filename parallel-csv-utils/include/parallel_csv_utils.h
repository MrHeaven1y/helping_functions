#ifndef PARALLEL_CSV_UTILS_H
#define PARALLEL_CSV_UTILS_H

#ifdef _WIN32
    #define DLL_EXPORT __declspec(dllexport)
    #include <windows.h>
#else
    #define DLL_EXPORT
#endif

#include <stdio.h>
#include <stddef.h>
#include <sys/stat.h>
#include <errno.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <omp.h>
#include <pthread.h>


#define BUFFER_SIZE 4096
#define SIGNIFICANT_DIGITS 16
#define MAX_THREADS 16
#define MIN_CHUNK_FOR_PARALLEL 1000


typedef struct {
    FILE* src;
    FILE* dest;
    char* buffer;
    size_t buffer_size;
    int chunk_id;
    int lines_to_copy;
    const char* delimiter;
    int* lines_copied;
    pthread_mutex_t* mutex;
} CopyThreadArgs;

// Global header buffer with mutex protection
extern DLL_EXPORT char Header[BUFFER_SIZE];
extern DLL_EXPORT omp_lock_t header_lock;

// Function declarations
DLL_EXPORT int tokenizer(char **tok, 
    const char *line, 
    const char *DELIMITER, 
    int lineNo);

DLL_EXPORT int getChunk(FILE* fp, 
    char** lines, 
    int chunkSize);

DLL_EXPORT int getChunk_wrapper(const char* fname, 
    char** lines, 
    int chunkSize);

DLL_EXPORT int parseChunk(double **instanceList, 
    char **lines, 
    int CHUNK_SIZE, 
    int numCols, 
    const char *DELIMITER);

DLL_EXPORT int parallel_parseChunk(double **instanceList, 
    char **lines, 
    int chunk_size, 
    int numCols, 
    const char *DELIMITER, 
    int num_threads);

DLL_EXPORT int writeChunk(const char *targetFile, 
    double **data, 
    const char *HEADER, 
    int numCols, 
    int chunkSize, 
    int chunkNo);

DLL_EXPORT int parallel_writeChunk(const char *targetFile, 
    double **data, 
    const char *HEADER, 
    int numCols, 
    int chunkSize, 
    int chunkNo, 
    int num_threads);

DLL_EXPORT int setHeader(FILE *fp, 
    const char *DELIMITER);

DLL_EXPORT int setHeader_wrapper(const char* fname, 
    const char* DELIMITER);

DLL_EXPORT const char* getHeader();

DLL_EXPORT int parallel_process_csv(const char* input_file, 
    const char* output_base, 
    const char* ext, 
    const char* DELIMITER, 
    int chunk, 
    int num_threads);

DLL_EXPORT int process_csv(const char* input_file, 
    const char* output_base, 
    const char* ext, 
    const char* DELIMITER, 
    int chunk);


    
DLL_EXPORT int copyData(const char* source_file, 
        const char* dest_base, 
        const char* ext, 
        const char* delimiter, 
        int chunk_size);
        
DLL_EXPORT long get_file_size(const char* filename);

DLL_EXPORT void process_chunk(int chunk_id, 
    const char* data, 
    size_t size);

DLL_EXPORT void init_csv_utils();

DLL_EXPORT void cleanup_csv_utils();

DLL_EXPORT int parallel_copy_data(const char* source_file, const char* dest_base, const char* ext, 
    const char* delimiter, int chunk_size, int num_threads);
#ifdef __cplusplus
extern "C" {
#endif

// Utility macro for freeing 2D arrays
#define free_2d_array(type, arr, rows) \
    do { \
        if ((arr) != NULL) { \
            for (int i = 0; i < (rows); i++) { \
                free((type*)(arr)[i]); \
                (arr)[i] = NULL; \
            } \
            free((type*)(arr)); \
            (arr) = NULL; \
        } \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif