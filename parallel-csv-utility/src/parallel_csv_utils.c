#include "parallel_csv_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <math.h>
#include <omp.h>
#include <ctype.h>
#include <sys/stat.h>

char Header[BUFFER_SIZE] = "";
omp_lock_t header_lock;

// Initialize library
void init_csv_utils() {
    omp_init_lock(&header_lock);
    memset(Header, 0, BUFFER_SIZE);
}

// Cleanup library
void cleanup_csv_utils() {
    omp_destroy_lock(&header_lock);
}



static void precised(double *value) {
    if(isnan(*value) || isinf(*value)) {
        *value = 0.0;
        return;
    }
    
    double factor = pow(10, SIGNIFICANT_DIGITS);
    *value = round(*value * factor) / factor;
}

static const char* next(FILE *fp) {
    static char current_line[BUFFER_SIZE];
    if (fgets(current_line, sizeof(current_line), fp)) {
        return current_line;
    }
    return NULL;
}

int merged(const char *base, int number, const char *ext, char *buffer, size_t bufsize) {
    int digits = snprintf(NULL, 0, "%d", number);
    size_t needed = strlen(base) + digits + strlen(ext) + 1;
    
    if(bufsize < needed) {
        return -1;
    }

    snprintf(buffer, bufsize, "%s%d%s", base, number, ext);
    return strlen(buffer);
}

int setHeader(FILE* fp, const char* DELIMITER) {
    if(!fp) {
        perror("setHeader: error file pointer is empty.");
        return -1;
    }

    const char* first_line = next(fp);
    if(!first_line) {
        perror("setHeader: file is empty");
        return -1;
    }

    char* cp_first_line = strdup(first_line);
    if(!cp_first_line) {
        perror("setHeader: strdup failed");
        return -1;
    }

    char* newline = strchr(cp_first_line, '\n');
    if(newline) *newline = '\0';
    newline = strchr(cp_first_line, '\r');
    if(newline) *newline = '\0';

    omp_set_lock(&header_lock);
    if(strcmp(Header, "") == 0) {
        strncpy(Header, cp_first_line, BUFFER_SIZE-1);
        Header[BUFFER_SIZE-1] = '\0'; 
    }
    omp_unset_lock(&header_lock);

    int numTokens = tokenizer(NULL, cp_first_line, DELIMITER, 0);
    
    free(cp_first_line);
    return numTokens;
}

int setHeader_wrapper(const char* fname, const char* DELIMITER) {
    FILE* fp = fopen(fname, "r");
    if (fp == NULL) {
        perror("setHeader_wrapper: fopen failed");
        return -1;
    }
    int result = setHeader(fp, DELIMITER);
    fclose(fp);
    return result;
}

int getChunk(FILE* fp, char** lines, int chunkSize) {
    if(!fp || !chunkSize) {
        perror("getChunk: the file is empty or the chunk size is zero");
        return -1;
    }
    
    int i;
    for(i = 0; i < chunkSize; i++) {
        const char* line = next(fp);
        if(!line) {
            if(feof(fp)) {
                break;
            }
            perror("getChunk: file read error");
            return -1;
        }

        lines[i] = strdup(line);
        if(!lines[i]) {
            while(i --> 0) {
                free(lines[i]);
                lines[i] = NULL;
            }
            return -1;
        }
    }
    return i;
}

int getChunk_wrapper(const char* fname, char** lines, int chunkSize) {
    FILE* fp = fopen(fname, "r");
    if(!fp) {
        perror("getChunk_wrapper: fopen failed");
        return -1;
    }

    int result = getChunk(fp, lines, chunkSize);
    if(result == -1) {
        perror("getChunk_wrapper: error to get chunk");
    }
    fclose(fp);
    return result;
}

int tokenizer(char **tok, const char *line, const char *DELIMITER, int lineNo) {
    if ((!tok && !line) || !DELIMITER) {
        errno = EINVAL;
        return -1;
    }

    char *cpline = strdup(line);
    if (!cpline) return -1;

    int i = 0;
    char *saveptr = NULL;
    char *token = strtok_r(cpline, DELIMITER, &saveptr);

    while (token != NULL) {
        if (lineNo) {
            char *end = token + strlen(token) - 1;
            while (end > token && isspace((unsigned char)*end)) end--;
            *(end + 1) = '\0';
            
            tok[i] = strdup(token);
            if (!tok[i]) {
                for (int j = 0; j < i; j++) free(tok[j]);
                free(cpline);
                return -1;
            }
        }
        token = strtok_r(NULL, DELIMITER, &saveptr);
        i++;
    }
    free(cpline);
    return i;
}

int parseChunk(double** instanceList, char** lines, int lines_in_chunk, int numCols, const char* DELIMITER) {
    int i, j;
    double value;
    char *endptr = NULL;
    
    char** tokens = (char **)malloc(numCols * sizeof(char *));
    if (!tokens) { 
         perror("Memory allocation failed for tokens");
         return -1;
    }

    for (j = 0; j < numCols; j++) {
         tokens[j] = NULL;
    }

    if (!instanceList) { 
         perror("instanceList is NULL");
         free(tokens);
         return -1;
    }

    for(i = 0; i < lines_in_chunk; i++) {
         if(!lines[i]) continue;

         char* line_copy = strdup(lines[i]);
         if(!line_copy) {
             perror("Failed to duplicate line");
             continue; 
         }

         int token_count = tokenizer(tokens, line_copy, DELIMITER, 1);
         free(line_copy);

         if(token_count < 0) {
             fprintf(stderr, "Tokenizer failed for line %d\n", i);
             continue;
         }

         if (!instanceList[i]) {
            fprintf(stderr, "Error: instanceList[%d] is NULL in parseChunk\n", i);
            for(j = 0; j < token_count; j++) {
               free(tokens[j]);
               tokens[j] = NULL;
            }
            continue;
         }

         for(j = 0; j < numCols && j < token_count; j++){
             if (!tokens[j]) continue;

             errno = 0;
             value = strtod(tokens[j], &endptr);
             precised(&value);

             if(errno == ERANGE){
                 printf("Warning: Overflow/underflow for '%s' on line %d\n", tokens[j], i);
             } else if(endptr == tokens[j] || *endptr != '\0') {
                 printf("Warning: Invalid number '%s' on line %d\n", tokens[j], i);
             } else{
                 instanceList[i][j] = value;
             }

             free(tokens[j]);
             tokens[j] = NULL;
         }

         for (j = 0; j < token_count; j++) {
             if (tokens[j]) {
                 free(tokens[j]);
                 tokens[j] = NULL;
             }
         }
    }

    free(tokens);
    return lines_in_chunk;
}

int parallel_parseChunk(double** instanceList, char** lines, int lines_in_chunk, 
                       int numCols, const char* DELIMITER, int num_threads) {
    if (!instanceList || !lines || !DELIMITER) {
        fprintf(stderr, "parallel_parseChunk: NULL pointer passed\n");
        return -1;
    }

    // If chunk size is small, use serial processing
    if (lines_in_chunk < MIN_CHUNK_FOR_PARALLEL) {
        return parseChunk(instanceList, lines, lines_in_chunk, numCols, DELIMITER);
    }

    // Set number of threads
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    num_threads = (num_threads > MAX_THREADS) ? MAX_THREADS : num_threads;
    omp_set_num_threads(num_threads);

    int error_flag = 0;

    #pragma omp parallel shared(error_flag)
    {
        char** tokens = (char**)malloc(numCols * sizeof(char*));
        if (!tokens) {
            #pragma omp critical
            {
                fprintf(stderr, "Thread %d: Memory allocation failed for tokens\n", omp_get_thread_num());
                error_flag = 1;
            }
        } else {
            // Initialize tokens
            for (int j = 0; j < numCols; j++) {
                tokens[j] = NULL;
            }

            #pragma omp for schedule(dynamic)
            for(int i = 0; i < lines_in_chunk; i++) {
                if(error_flag) continue;
                if(!lines[i]) continue;

                char* line_copy = strdup(lines[i]);
                if(!line_copy) {
                    #pragma omp critical
                    {
                        fprintf(stderr, "Thread %d: Failed to duplicate line %d\n", omp_get_thread_num(), i);
                        error_flag = 1;
                    }
                    continue;
                }

                int token_count = tokenizer(tokens, line_copy, DELIMITER, 1);
                free(line_copy);

                if(token_count < 0) {
                    #pragma omp critical
                    {
                        fprintf(stderr, "Thread %d: Tokenizer failed for line %d\n", omp_get_thread_num(), i);
                        error_flag = 1;
                    }
                    continue;
                }

                if (!instanceList[i]) {
                    #pragma omp critical
                    {
                        fprintf(stderr, "Thread %d: instanceList[%d] is NULL\n", omp_get_thread_num(), i);
                        error_flag = 1;
                    }
                    for(int j = 0; j < token_count; j++) {
                        if(tokens[j]) free(tokens[j]);
                    }
                    continue;
                }

                for(int j = 0; j < numCols && j < token_count; j++) {
                    if (!tokens[j]) continue;

                    char* endptr = NULL;
                    errno = 0;
                    double value = strtod(tokens[j], &endptr);
                    precised(&value);

                    if(errno == ERANGE) {
                        #pragma omp critical
                        {
                            printf("Warning: Overflow/underflow for '%s' on line %d\n", tokens[j], i);
                        }
                    } else if(endptr == tokens[j] || *endptr != '\0') {
                        #pragma omp critical
                        {
                            printf("Warning: Invalid number '%s' on line %d\n", tokens[j], i);
                        }
                    } else {
                        instanceList[i][j] = value;
                    }

                    free(tokens[j]);
                    tokens[j] = NULL;
                }

                for(int j = 0; j < token_count; j++) {
                    if(tokens[j]) {
                        free(tokens[j]);
                        tokens[j] = NULL;
                    }
                }
            }
        }
        
        if(tokens) free(tokens);
    }

    return error_flag ? -1 : lines_in_chunk;
}

int writeChunk(const char* targetFile, double** data, const char* HEADER, int numCols, int chunkSize, int chunkNo) {
    if (!targetFile || !data || !HEADER) {
        fprintf(stderr, "writeChunk: NULL parameter(s)\n");
        return -1;
    }

    FILE *file = fopen(targetFile, "w");
    if(!file) {
        perror("writeChunk: cannot open the file.");
        return -1;
    }

    char *cp_header = strdup(HEADER);
    if(!cp_header) {
        perror("writeChunk: failed to copy the header");
        fclose(file);
        return -1; 
    }

    char *newline = strchr(cp_header, '\n');
    if(!newline) {
        newline = strchr(cp_header, '\0');
        *newline = '\n';
        *(newline+1) = '\0';
    }

    if(fputs(cp_header, file) == EOF) {
        perror("writeChunk: Error writing header to file");
        free(cp_header);
        fclose(file);
        return -1; 
    }

    for(int i = 0; i < chunkSize; i++) {
        if (!data[i]) continue;
        
        fprintf(file, "%.16lf", data[i][0]);
        
        for(int j = 1; j < numCols; j++) {
            fprintf(file, ",%.16lf", data[i][j]);
        }
        fprintf(file, "\n");  
    }

    printf("Successfully created file for chunk: %d\n", chunkNo);
    free(cp_header);
    fclose(file);
    return chunkSize;
}

int parallel_writeChunk(const char* targetFile, double** data, const char* HEADER, 
                      int numCols, int chunkSize, int chunkNo, int num_threads) {
    if (!targetFile || !data || !HEADER) {
        fprintf(stderr, "parallel_writeChunk: NULL parameter(s)\n");
        return -1;
    }

    // For small chunks, use serial writing
    if (chunkSize < MIN_CHUNK_FOR_PARALLEL) {
        return writeChunk(targetFile, data, HEADER, numCols, chunkSize, chunkNo);
    }

    FILE *file = fopen(targetFile, "w");
    if(!file) {
        perror("parallel_writeChunk: cannot open the file.");
        return -1;
    }

    char *cp_header = strdup(HEADER);
    if(!cp_header) {
        perror("parallel_writeChunk: failed to copy the header");
        fclose(file);
        return -1; 
    }

    char *newline = strchr(cp_header, '\n');
    if(!newline) {
        newline = strchr(cp_header, '\0');
        *newline = '\n';
        *(newline+1) = '\0';
    }

    if(fputs(cp_header, file) == EOF) {
        perror("parallel_writeChunk: Error writing header to file");
        free(cp_header);
        fclose(file);
        return -1; 
    }

    // Set number of threads
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    num_threads = (num_threads > MAX_THREADS) ? MAX_THREADS : num_threads;
    omp_set_num_threads(num_threads);

    // Use OpenMP to format lines in parallel
    char** formatted_lines = (char**)malloc(chunkSize * sizeof(char*));
    if (!formatted_lines) {
        perror("parallel_writeChunk: Memory allocation failed");
        free(cp_header);
        fclose(file);
        return -1;
    }

    #pragma omp parallel for
    for(int i = 0; i < chunkSize; i++) {
        if (!data[i]) {
            formatted_lines[i] = NULL;
            continue;
        }
        
        // Allocate memory for the formatted line
        size_t line_size = numCols * 25 + 2; // Conservative estimate
        formatted_lines[i] = (char*)malloc(line_size);
        
        if (!formatted_lines[i]) {
            #pragma omp critical
            {
                fprintf(stderr, "Thread %d: Memory allocation failed for line %d\n", 
                       omp_get_thread_num(), i);
            }
            continue;
        }
        
        // Format the first column
        int written = snprintf(formatted_lines[i], line_size, "%.16lf", data[i][0]);
        
        // Format remaining columns
        for(int j = 1; j < numCols && written > 0 && written < line_size; j++) {
            written += snprintf(formatted_lines[i] + written, line_size - written, 
                              ",%.16lf", data[i][j]);
        }
        
        // Add newline if there's space
        if (written > 0 && written < line_size - 1) {
            formatted_lines[i][written] = '\n';
            formatted_lines[i][written + 1] = '\0';
        }
    }

    // Write all formatted lines to the file (single-threaded for I/O)
    for(int i = 0; i < chunkSize; i++) {
        if (formatted_lines[i]) {
            fputs(formatted_lines[i], file);
            free(formatted_lines[i]);
        }
    }
    
    free(formatted_lines);
    printf("Successfully created file for chunk: %d (using %d threads)\n", chunkNo, num_threads);
    free(cp_header);
    fclose(file);
    return chunkSize;
}

int parallel_process_csv(const char* input_file, const char* output_base, const char* ext, 
                        const char* DELIMITER, int chunk_size, int num_threads) {
    if (!input_file || !output_base || !ext || !DELIMITER) {
        fprintf(stderr, "NULL pointer passed to parallel_process_csv\n");
        return -1;
    }
    
    printf("Processing CSV with %d threads\n", num_threads);
    
    // Initialize header
    omp_set_lock(&header_lock);
    memset(Header, 0, BUFFER_SIZE);
    omp_unset_lock(&header_lock);

    FILE* fp = fopen(input_file, "r");
    if(!fp) {
        perror("parallel_process_csv: Failed to open input file");
        return -1;
    }
    
    int numCols = setHeader(fp, DELIMITER);
    if(numCols <= 0) {
        fclose(fp);
        fprintf(stderr, "parallel_process_csv: Failed to read header\n");
        return -1;
    }
    
    int chunk_count = 0;
    int total_lines = 0;
    
    // Set number of threads
    if (num_threads <= 0) {
        num_threads = omp_get_max_threads();
    }
    num_threads = (num_threads > MAX_THREADS) ? MAX_THREADS : num_threads;
    omp_set_num_threads(num_threads);

    while(1) {
        // Allocate memory for lines and data
        char** lines = (char**)malloc(chunk_size * sizeof(char*));
        double** data = (double**)malloc(chunk_size * sizeof(double*));
    
        if(!lines || !data) {
            perror("parallel_process_csv: Failed to allocate memory");
            if(lines) free(lines);
            if(data) free(data);
            fclose(fp);
            return -1;
        }

        // Initialize data pointers in parallel
        #pragma omp parallel for
        for(int i = 0; i < chunk_size; i++) {
            data[i] = (double*)malloc(numCols * sizeof(double));
            if(!data[i]) {
                #pragma omp critical
                {
                    fprintf(stderr, "Thread %d: Failed to allocate data row %d\n", 
                           omp_get_thread_num(), i);
                }
            }
        }

        // Read chunk (single-threaded for I/O)
        int lines_read = getChunk(fp, lines, chunk_size);

        if(lines_read <= 0) {
            free_2d_array(char*, lines, chunk_size);
            free_2d_array(double*, data, chunk_size);
            break;
        }

        // Parse chunk in parallel
        int parse_result = parallel_parseChunk(data, lines, lines_read, numCols, DELIMITER, num_threads);
        if(parse_result < 0) {
            fprintf(stderr, "parallel_process_csv: Failed to parse chunk %d\n", chunk_count);
            free_2d_array(double*, data, chunk_size);
            free_2d_array(char*, lines, chunk_size);
            continue;
        }

        // Generate output filename
        char output_filename[BUFFER_SIZE];
        if (merged(output_base, chunk_count, ext, output_filename, BUFFER_SIZE) < 0) {
            fprintf(stderr, "parallel_process_csv: Output filename too long\n");
            free_2d_array(double*, data, chunk_size);
            free_2d_array(char*, lines, chunk_size);
            continue;
        }
        
        // Write chunk in parallel
        if (parallel_writeChunk(output_filename, data, Header, numCols, lines_read, chunk_count, num_threads) < 0) {
            fprintf(stderr, "parallel_process_csv: Failed to write chunk %d\n", chunk_count);
        }
        
        total_lines += lines_read;
        free_2d_array(double*, data, chunk_size);
        free_2d_array(char*, lines, chunk_size);
        chunk_count++;
    }

    fclose(fp);
    printf("Processed %d chunks with %d total lines\n", chunk_count, total_lines);
    return chunk_count;
}

int process_csv(const char* input_file, const char* output_base, const char* ext, 
                const char* DELIMITER, int chunk) {
    return parallel_process_csv(input_file, output_base, ext, DELIMITER, chunk, 1);
}

const char* getHeader() {
    omp_set_lock(&header_lock);
    const char* h = Header;
    omp_unset_lock(&header_lock);
    return h;
}

long get_file_size(const char* filename) {
    struct stat st;
    if (stat(filename, &st) == 0) {
        return st.st_size;
    }
    perror("Error getting file size");
    return -1;
}

void* copy_chunk_thread(void* arg) {
    CopyThreadArgs* args = (CopyThreadArgs*)arg;
    int lines_copied = 0;
    char line[BUFFER_SIZE];

    while (lines_copied < args->lines_to_copy && fgets(line, sizeof(line), args->src)) {
        // Remove newline characters
        line[strcspn(line, "\r\n")] = '\0';
        
        pthread_mutex_lock(args->mutex);
        fputs(line, args->dest);
        fputc('\n', args->dest);
        (*(args->lines_copied))++;
        pthread_mutex_unlock(args->mutex);
        
        lines_copied++;
    }

    return NULL;
}

int parallel_copy_data(const char* source_file, const char* dest_base, 
    const char* ext, const char* delimiter, 
    int chunk_size, int num_threads) {
FILE* src = fopen(source_file, "r");
if (!src) {
perror("Error opening source file");
return -1;
}

// Read header once
char header[BUFFER_SIZE];
if (!fgets(header, sizeof(header), src)) {
perror("Error reading header");
fclose(src);
return -1;
}

struct timespec total_start, total_end;
clock_gettime(CLOCK_MONOTONIC, &total_start);

int total_lines = 0;
int chunk_count = 0;
char line[BUFFER_SIZE];

while (!feof(src)) {
char dest_filename[BUFFER_SIZE];
if (merged(dest_base, chunk_count, ext, dest_filename, BUFFER_SIZE) < 0) {
fprintf(stderr, "Filename generation failed\n");
break;
}

FILE* dest = fopen(dest_filename, "w");
if (!dest) {
perror("Error opening destination file");
break;
}

// Write header
fputs(header, dest);

struct timespec chunk_start, chunk_end;
clock_gettime(CLOCK_MONOTONIC, &chunk_start);

int lines_in_chunk = 0;
while (lines_in_chunk < chunk_size && fgets(line, sizeof(line), src)) {
fputs(line, dest);
lines_in_chunk++;
total_lines++;
}

clock_gettime(CLOCK_MONOTONIC, &chunk_end);
double chunk_elapsed = (chunk_end.tv_sec - chunk_start.tv_sec) + 
           (chunk_end.tv_nsec - chunk_start.tv_nsec) / 1e9;

printf("Chunk %d: %d lines", chunk_count, lines_in_chunk);
if (chunk_elapsed >= 0.001) {
printf(" (%.1f lines/sec)\n", lines_in_chunk / chunk_elapsed);
} else {
printf(" (completed quickly)\n");
}

fclose(dest);
chunk_count++;

if (lines_in_chunk == 0) {
remove(dest_filename); // Delete empty last chunk
chunk_count--;
break;
}
}

clock_gettime(CLOCK_MONOTONIC, &total_end);
double total_elapsed = (total_end.tv_sec - total_start.tv_sec) + 
       (total_end.tv_nsec - total_start.tv_nsec) / 1e9;

printf("\nCopy complete! Processed %d lines in %.4f seconds", 
total_lines, total_elapsed);

if (total_elapsed > 0.1) {
printf(" (%.1f lines/sec)\n", total_lines / total_elapsed);
} else {
printf("\n");
}

fclose(src);
return chunk_count;
}
// Keep the original copyData as a wrapper
int copyData(const char* source_file, const char* dest_base, const char* ext, 
             const char* delimiter, int chunk_size) {
    return parallel_copy_data(source_file, dest_base, ext, delimiter, chunk_size, 1);
}