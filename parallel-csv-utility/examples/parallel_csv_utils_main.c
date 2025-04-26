#include "parallel_csv_utils.h"
#include <time.h>
#include <omp.h>
#include <ctype.h>
#include <string.h>

typedef struct {
    int chunk_id;
    double start_time;
    double end_time;
    double duration;
} ChunkTiming;

ChunkTiming* chunk_timings = NULL;
int total_chunks = 0;

void trim_whitespace(char* str) {
    char* end;
    while(isspace((unsigned char)*str)) str++;
    if(*str == 0) return;
    end = str + strlen(str) - 1;
    while(end > str && isspace((unsigned char)*end)) end--;
    *(end+1) = 0;
}

int get_positive_integer(const char* prompt) {
    int value;
    char input[50];
    while(1) {
        printf("%s", prompt);
        fgets(input, sizeof(input), stdin);
        if(sscanf(input, "%d", &value) == 1 && value > 0) {
            return value;
        }
        printf("Invalid input. Please enter a positive integer.\n");
    }
}

void get_string_input(const char* prompt, char* buffer, size_t buffer_size) {
    printf("%s", prompt);
    fgets(buffer, buffer_size, stdin);
    trim_whitespace(buffer);
}

void remove_extension(char* filename) {
    char* dot = strrchr(filename, '.');
    if(dot != NULL) *dot = '\0';
}

void print_timings(double total_elapsed, long fsize) {
    printf("\n================= TIMING RESULTS =================\n");
    printf("Total processing time: %.4f seconds\n", total_elapsed);
    
    if (fsize > 0) {
        printf("Throughput: %.2f MB/s\n", 
              fsize / (1024.0 * 1024.0) / total_elapsed);
    }
    
    if (chunk_timings != NULL && total_chunks > 0) {
        printf("\nPer-chunk processing times:\n");
        printf("Chunk ID\tStart (s)\tEnd (s)\tDuration (ms)\n");
        
        double min_time = chunk_timings[0].duration;
        double max_time = chunk_timings[0].duration;
        double total_time = 0;
        
        for (int i = 0; i < total_chunks; i++) {
            printf("%8d\t%9.4f\t%9.4f\t%12.2f\n", 
                  chunk_timings[i].chunk_id,
                  chunk_timings[i].start_time,
                  chunk_timings[i].end_time,
                  chunk_timings[i].duration * 1000);
            
            if (chunk_timings[i].duration < min_time) min_time = chunk_timings[i].duration;
            if (chunk_timings[i].duration > max_time) max_time = chunk_timings[i].duration;
            total_time += chunk_timings[i].duration;
        }
        
        printf("\nStatistics:\n");
        printf("Average chunk time: %.2f ms\n", (total_time/total_chunks)*1000);
        printf("Minimum chunk time: %.2f ms\n", min_time*1000);
        printf("Maximum chunk time: %.2f ms\n", max_time*1000);
    }
}

int main(int argc, char *argv[]) {
    init_csv_utils();
    
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    char fname[256] = {0};
    char outputfname[256] = {0};
    char ext[32] = ".csv";
    char delim[32] = ",";
    unsigned chunk_size = 20;
    int num_threads;
    int max_threads = omp_get_max_threads();

    if(argc > 1) {
        strncpy(fname, argv[1], sizeof(fname)-1);
        if(argc > 2) strncpy(outputfname, argv[2], sizeof(outputfname)-1);
        if(argc > 3) strncpy(ext, argv[3], sizeof(ext)-1);
        if(argc > 4) strncpy(delim, argv[4], sizeof(delim)-1);
        if(argc > 5) chunk_size = atoi(argv[5]);
        if(argc > 6) num_threads = atoi(argv[6]);
        
        if(strlen(outputfname) == 0) {
            strncpy(outputfname, fname, sizeof(outputfname)-1);
            remove_extension(outputfname);
            strncat(outputfname, "_chunk", sizeof(outputfname)-strlen(outputfname)-1);
        }
    } else {
        printf("CSV Processor Configuration\n");
        printf("--------------------------\n");
        
        get_string_input("Input file name: ", fname, sizeof(fname));
        
        char suggested_output[256];
        strncpy(suggested_output, fname, sizeof(suggested_output)-1);
        remove_extension(suggested_output);
        strncat(suggested_output, "_chunk", sizeof(suggested_output)-strlen(suggested_output)-1);
        
        printf("Suggested output base name: %s\n", suggested_output);
        get_string_input("Output base name (press enter to use suggestion): ", outputfname, sizeof(outputfname));
        if(strlen(outputfname) == 0) {
            strncpy(outputfname, suggested_output, sizeof(outputfname)-1);
        }
        
        get_string_input("File extension [defualt '.csv']: ", ext, sizeof(ext));
        get_string_input("Delimiter [defualt ',']: ", delim, sizeof(delim));
        
        printf("Default chunk size: 30,000 lines/file\n");
        chunk_size = get_positive_integer("Chunk size (lines per chunk): ");
        
        printf("Available threads: %d\n", max_threads);
        num_threads = get_positive_integer("Number of threads to use: ");
        if(num_threads > max_threads) {
            printf("Warning: Using %d threads (requested %d)\n", max_threads, num_threads);
            num_threads = max_threads;
        }
    }

    if(strlen(fname) == 0) {
        printf("Error: Input file name cannot be empty\n");
        return 1;
    }
    if(strlen(ext) == 0) strcpy(ext, ".csv");
    if(strlen(delim) == 0) strcpy(delim, ",");
    if(chunk_size <= 0) chunk_size = 20;
    if(num_threads <= 0) num_threads = max_threads;

    if(ext[0] != '.') {
        char temp[32] = ".";
        strncat(temp, ext, sizeof(temp)-2);
        strncpy(ext, temp, sizeof(ext)-1);
    }

    printf("\nCSV Processor Starting with Configuration:\n");
    printf("----------------------------------------\n");
    printf("Input file: %s\n", fname);
    printf("Output base: %s\n", outputfname);
    printf("Extension: %s\n", ext);
    printf("Delimiter: '%s'\n", delim);
    printf("Chunk size: %d lines\n", chunk_size);
    printf("Threads: %d\n", num_threads);
    printf("Max available threads: %d\n", max_threads);
    
    int result = parallel_process_csv(fname, outputfname, ext, delim, chunk_size, num_threads);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    
    long fsize = get_file_size(fname);
    
    // Print all timing information
    print_timings(elapsed, fsize);
    
    if (chunk_timings != NULL) {
        free(chunk_timings);
    }
    
    cleanup_csv_utils();
    return 0;
}