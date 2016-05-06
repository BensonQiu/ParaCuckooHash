#include <iostream>
#include <stdlib.h>
#include <pthread.h>

#include "../common/CycleTimer.h"
#include "../common/errors.h"
#include "thread_send_requests.h"

template <typename T>
BenchmarkOptCuckooTagHashMap<T>::BenchmarkOptCuckooTagHashMap(
		int num_ops, float space_efficiency, int slots_per_bucket) {

	m_num_ops = num_ops;
	m_space_efficiency = space_efficiency;
	m_slots_per_bucket = slots_per_bucket;
	m_num_buckets = (1.0f/m_space_efficiency) * m_num_ops / float(m_slots_per_bucket);

	m_random_keys = new std::string[m_num_ops];
	for (int i=0; i<m_num_ops; i++) {
		m_random_keys[i] = rand();
	}
}

template <typename T>
BenchmarkOptCuckooTagHashMap<T>::~BenchmarkOptCuckooTagHashMap() {

	delete[] m_random_keys;
}

template <typename T>
void BenchmarkOptCuckooTagHashMap<T>::benchmark_random_interleaved_read_write() {

    OptimisticCuckooTagHashMap<T> my_map(m_num_buckets);
    try {
	    my_map.get("hello");
	} catch(KeyNotFoundError) {}
}

template <typename T>
void BenchmarkOptCuckooTagHashMap<T>::benchmark_read_only() {

    OptimisticCuckooTagHashMap<T> my_map(m_num_buckets);
    double best_time = m_benchmark_reads_helper(&my_map);

    std::cout << "\t" << "Read-Only (" << NUM_READERS << " Reader Threads): " << best_time << std::endl;
}

// TODO: Write with multiple threads if we implement optimistic locking.
template <typename T>
void BenchmarkOptCuckooTagHashMap<T>::benchmark_write_only() {

	double start_time, end_time, best_time;

    // Take the best time of three runs.
    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
	    OptimisticCuckooTagHashMap<T> my_map(m_num_buckets);

        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
            my_map.put(m_random_keys[j], m_random_keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "Write-Only: " << best_time << std::endl;
}

template <typename T>
void BenchmarkOptCuckooTagHashMap<T>::benchmark_read_only_single_bucket() {

	double start_time, end_time, best_time;

	// Warm up the hashmap with sequential insertions.
    OptimisticCuckooTagHashMap<T> my_map(m_num_buckets);
	for (int i = 0; i < m_num_ops; i++) {
		my_map.put(m_random_keys[i], m_random_keys[i]);
	}

	// Concurrently read the same key.
	std::string* identical_keys = new std::string[m_num_ops];
	for (int i = 0; i < m_num_ops; i++) {
		identical_keys[i] = m_random_keys[0];
	}

    for (int num_readers = 1; num_readers <= NUM_READERS; num_readers += 4) {

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
		        	thread_send_requests<OptimisticCuckooTagHashMap<T>>, &args[j]);
		    }
		    for (int j = 0; j < num_readers; j++) {
		        pthread_join(workers[j], NULL);
		    }
	        end_time = CycleTimer::currentSeconds();
	        best_time = std::min(best_time, end_time-start_time);
	    }
	    std::cout << "\t" << "Read-Only Single Bucket (" << num_readers << " Reader Threads): " << best_time << std::endl;
	}
}

template <typename T>
void BenchmarkOptCuckooTagHashMap<T>::benchmark_space_efficiency() {

	for (float space_efficiency = 0.15f; space_efficiency <= 0.9f; space_efficiency += 0.15f) {
		int num_buckets = (1.0f/space_efficiency) * m_num_ops / float(m_slots_per_bucket);
	    OptimisticCuckooTagHashMap<T> my_map(num_buckets);
		double best_time = m_benchmark_reads_helper(&my_map);

	    std::cout << "\t" << 100*space_efficiency << "% Space Efficiency ("
	              << NUM_READERS << " Reader Threads): " << best_time << std::endl;
	}
}

template <typename T>
double BenchmarkOptCuckooTagHashMap<T>::m_benchmark_reads_helper(
		OptimisticCuckooTagHashMap<T>* my_map) {

	// Warm up the hashmap.
	for (int i = 0; i < m_num_ops; i++) {
		my_map->put(m_random_keys[i], m_random_keys[i]);
	}

	// Setup thread args.
    pthread_t workers[NUM_READERS];
    WorkerArgs args[NUM_READERS];

    for (int i = 0; i < NUM_READERS; i++) {
        args[i].my_map = (void*)my_map;
        args[i].thread_id = (long int)i;
        args[i].num_threads = NUM_READERS;
        args[i].num_ops = m_num_ops;
        args[i].percent_reads = 1.0f;
        args[i].keys = m_random_keys;
    }

	double start_time, end_time, best_time;

    // Take the best of 3 runs.
    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
	    for (int j = 0; j < NUM_READERS; j++) {
	        pthread_create(&workers[j], NULL,
	        	thread_send_requests<OptimisticCuckooTagHashMap<T>>, &args[j]);
	    }
	    for (int j = 0; j < NUM_READERS; j++) {
	        pthread_join(workers[j], NULL);
	    }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
        // std::cout << "\t" << end_time-start_time << std::endl;
    }

	return best_time;
}

template <typename T>
void BenchmarkOptCuckooTagHashMap<T>::run_all() {

	std::cout << "Benchmarking Optimistic Cuckoo Tag HashMap, " << m_num_ops << " Operations..." << std::endl;

	benchmark_random_interleaved_read_write();
	benchmark_read_only();
	benchmark_write_only();
	benchmark_read_only_single_bucket();
	benchmark_space_efficiency();

    std::cout << std::endl;
}
