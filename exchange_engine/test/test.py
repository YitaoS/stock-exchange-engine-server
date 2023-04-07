import time
from multiprocessing import Pool
from myxml import XMLTestGenerator, test


def send_data(num):
    gen = XMLTestGenerator()
    test(gen.setup())
    for i in range(500):
        print("sent transc " + str(i), end="; ", flush=True)
        test(gen.random_order())


if __name__ == "__main__":
    MAX_PROCESSES = 4 
    pool = Pool(processes=MAX_PROCESSES)
    start_time = time.time()
    pool.map(send_data, range(MAX_PROCESSES))
    pool.close()
    pool.join()
    end_time = time.time()
    elapsed_time = end_time - start_time
    print(f"Elapsed time: {elapsed_time:.2f} seconds")