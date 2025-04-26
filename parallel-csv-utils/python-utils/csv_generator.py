import numpy as np
import csv

def generate_n_columns_csv_numpy_float(filename="output_float.csv", num_columns=5, num_rows=10):
    """
    Generates a CSV file with a specified number of columns and rows
    containing random float numbers using NumPy.

    Args:
        filename (str): The name of the CSV file to create. Defaults to "output_float.csv".
        num_columns (int): The number of columns in the CSV file. Defaults to 5.
        num_rows (int): The number of rows of data in the CSV file. Defaults to 10.
    """
    # Generate random float numbers using NumPy
    data = np.random.rand(num_rows, num_columns)

    with open(filename, 'w', newline='') as csvfile:
        writer = csv.writer(csvfile)

        # Write the header row
        header = [f'Column{i+1}' for i in range(num_columns)]
        writer.writerow(header)

        # Write the data rows
        writer.writerows(data)

if __name__ == "__main__":
    num_columns = int(input("Enter the number of columns: "))
    num_rows = int(input("Enter the number of rows: "))
    filename = input("Enter the filename for the CSV (e.g., float_data.csv): ")
    generate_n_columns_csv_numpy_float(filename=filename, num_columns=num_columns, num_rows=num_rows)
    print(f"CSV file '{filename}' with {num_columns} columns and {num_rows} rows of random floats generated successfully.")