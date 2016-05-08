#include <iostream>
#include <stdlib.h>
#include <pthread.h>

#include "../fine_hash_map.h"
#include "../common/CycleTimer.h"
#include "thread_send_requests.h"

template <typename T>
BenchmarkFineHashMap<T>::BenchmarkFineHashMap(int num_ops, float load_factor) {

	m_num_ops = num_ops;
	m_load_factor = load_factor;
	m_table_size = num_ops / load_factor;

	m_random_keys = new std::string[m_num_ops];
	for (int i=0; i<m_num_ops; i++) {
		m_random_keys[i] = std::to_string(rand());
	}

}

template <typename T>
BenchmarkFineHashMap<T>::~BenchmarkFineHashMap() {

	delete[] m_random_keys;
}

template <typename T>
void BenchmarkFineHashMap<T>::benchmark_random_interleaved_read_write() {

	std::cout << "TODO: Interleaved Reads / Writes" << std::endl;
}

template <typename T>
void BenchmarkFineHashMap<T>::benchmark_read_only() {

	double start_time, end_time, best_time;

	// Warm up the hashmap with sequential insertions.
    FineHashMap<T> my_map(m_table_size);
	for (int i = 0; i < m_num_ops; i++) {
		my_map.put(m_random_keys[i], m_random_keys[i]);
	}

	// Setup thread args.
    pthread_t workers[NUM_READERS];
    WorkerArgs args[NUM_READERS];

    for (int i = 0; i < NUM_READERS; i++) {
        args[i].my_map = (void*)&my_map;
        args[i].thread_id = (long int)i;
        args[i].num_threads = NUM_READERS;
        args[i].num_ops = m_num_ops;
        args[i].percent_reads = 1.0f;
        args[i].keys = m_random_keys;
    }

    // Take the best time of three runs.
    best_time = 1e30;
    for (int i = 0; i < 3; i++) {

        start_time = CycleTimer::currentSeconds();
	    for (int j = 0; j < NUM_READERS; j++) {
	        pthread_create(&workers[j], NULL,
	        	thread_send_requests<FineHashMap<T>>, &args[j]);
	    }
	    for (int j = 0; j < NUM_READERS; j++) {
	        pthread_join(workers[j], NULL);
	    }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "Read-Only (" << NUM_READERS << " Reader Threads): "
              << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

template <typename T>
void BenchmarkFineHashMap<T>::benchmark_write_only() {

	std::cout << "TODO: Write Only" << std::endl;
}

template <typename T>
void BenchmarkFineHashMap<T>::benchmark_read_only_single_bucket() {

	double start_time, end_time, best_time;

	// Warm up the hashmap with sequential insertions.
    FineHashMap<T> my_map(m_table_size);
	for (int i = 0; i < m_num_ops; i++) {
		my_map.put(m_random_keys[i], m_random_keys[i]);
	}

	// Concurrently read the same key.
	std::string* identical_keys = new std::string[m_num_ops];
	for (int i = 0; i < m_num_ops; i++) {
		identical_keys[i] = m_random_keys[0];
	}

	for (int num_readers = 1; num_readers <= NUM_READERS; num_readers+= 4) {

		// Setup thread args.
	    pthread_t workers[num_readers];
	    WorkerArgs args[num_readers];

	    for (int i = 0; i < num_readers; i++) {
	        args[i].my_map = (void*)&my_map;
	        args[i].thread_id = (long int)i;
	        args[i].num_threads = num_readers;
	        args[i].num_ops = m_num_ops;
	        args[i].percent_reads = 1.0f;
	        args[i].keys = identical_keys;
	    }

	    // Take the best time of three runs.
	    best_time = 1e30;
	    for (int i = 0; i < 3; i++) {
	        start_time = CycleTimer::currentSeconds();
		    for (int j = 0; j < num_readers; j++) {
		        pthread_create(&workers[j], NULL,
		        	thread_send_requests<FineHashMap<T>>, &args[j]);
		    }
		    for (int j = 0; j < num_readers; j++) {
		        pthread_join(workers[j], NULL);
		    }
	        end_time = CycleTimer::currentSeconds();
	        best_time = std::min(best_time, end_time-start_time);
	    }
	    std::cout << "\t" << "Read-Only Single Bucket (" << num_readers << " Reader Threads): "
	              << m_num_ops / best_time / (1000 * 1000) << std::endl;
	}
	delete[] identical_keys;
}

template <typename T>
void BenchmarkFineHashMap<T>::run_all() {

	std::cout << "Benchmarking Fine HashMap, " << m_num_ops << " Operations..." << std::endl;

	// benchmark_random_interleaved_read_write();
	// benchmark_read_only();
	// benchmark_write_only();
	benchmark_read_only_single_bucket();

	std::cout << std::endl;
}
