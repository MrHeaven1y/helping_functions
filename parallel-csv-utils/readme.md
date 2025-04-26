# Parallel CSV Processing Library

A high-performance C library for processing large CSV files in parallel using OpenMP. This library provides utilities for reading, parsing, processing, and writing CSV data in chunks, making it efficient for handling datasets that don't fit in memory.

## Key Features

- Parallel processing of CSV files using OpenMP (multi-threading)
- Chunk-based processing to handle large datasets
- Thread-safe operations with proper locking mechanisms
- Configurable number of threads for optimal performance
- High precision numeric handling with customizable significant digits
- Robust error handling and reporting

## Repository Structure
parallel-csv-utils/
├── include/
│ └── parallel_csv_utils.h
├── src/
│ └── parallel_csv_utils.c
├── examples/
│ ├── parallel_csv_utils_main.c # Example for processing CSV data 
│ └── split_csv_main.c # Example for copying CSV data in chunks
├── python_utils/ # Additional Python utilities
│ ├── csv_generator.py # Generate large CSV files for testing
│ └── move_files.py # Split and move CSV chunks to directories
└── README.md


## Usage Examples

The repository includes two main example files:

1. `copy_main.c`: Demonstrates how to efficiently copy large CSV files in chunks
2. `process_main.c`: Shows how to process CSV data with transformation functions

## Extending for Custom Processing

You can easily extend the library to implement custom data processing functions:

```c
// Example of using process_csv to apply custom transformation
#include "parallel_csv_utils.h"

// Custom processing function to be applied to each chunk
void process_data_chunk(double** data, int rows, int cols) {
    #pragma omp parallel for
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            // Apply your transformation here
            data[i][j] = sqrt(data[i][j]); // Example: square root transformation
        }
    }
}

int main() {
    // Initialize the library
    init_csv_utils();
    
    // Process CSV in chunks, then apply custom processing
    const char* input = "large_dataset.csv";
    const char* output_base = "processed_chunk_";
    const char* ext = ".csv";
    const char* delimiter = ",";
    int chunk_size = 10000;
    int num_threads = 8;
    
    // Process the CSV file
    int chunks = parallel_process_csv(input, output_base, ext, delimiter, chunk_size, num_threads);
    
    // Cleanup
    cleanup_csv_utils();
    return 0;
}
```c

Python Utilities
The repository includes Python scripts to help with CSV processing:

csv_generator.py: Generate large CSV files for testing

csv_splitter.py: Split CSV files and move chunks to specific directories (uses Python's threading for parallel operations)

Building and Installation

# Clone the repository
git clone https://github.com/yourusername/parallel-csv-utils.git

# Build the library and examples
cd parallel-csv-utils
make

# Run the examples
./copy_example large_file.csv output_ .csv 10000
./process_example large_file.csv processed_ .csv 10000 8

# Run Python utilities
python3 python_utils/csv_generator.py
python3 python_utils/csv_splitter.py

Performance
The library is optimized for multi-core systems and can achieve near-linear scaling with the number of cores for CPU-bound operations. Memory usage is controlled by the chunk size parameter.

For C operations: Uses OpenMP for parallel processing (multi-threading)

For Python utilities: Uses Python's threading module for parallel file operations