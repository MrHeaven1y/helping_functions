import os
import shutil
import glob
import threading
import time

def move_to_dir(dirname, file):
    os.makedirs(dirname, exist_ok=True)
    destination_path = os.path.join(dirname, os.path.basename(file))
    shutil.move(file, destination_path)

if __name__ == "__main__":
    dirname = input("enter dirname to move the processed files : ")
    pattern = input(f"enter files pattern to move to the {dirname} : ")
    files = glob.glob(pattern)
    threads = []
    
    start_time = time.time()

    for i, file in enumerate(files):  
        thread = threading.Thread(
            target=move_to_dir, 
            args=(dirname, file), 
            name=f"Thread-{i+1}"
        )
        threads.append(thread)
        thread.start()

    for thread in threads:
        thread.join()

    end_time = time.time()
    print(f"\nAll numbers processed in {end_time - start_time:.2f} seconds.")