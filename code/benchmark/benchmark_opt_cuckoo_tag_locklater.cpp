#include <iostream>
#include <stdlib.h>
#include <pthread.h>

#include "../common/CycleTimer.h"
#include "../common/errors.h"
#include "thread_send_requests.h"

template <typename T>
BenchmarkOptCuckooTagLockLaterHashMap<T>::BenchmarkOptCuckooTagLockLaterHashMap(
		int num_ops, float space_efficiency, int slots_per_bucket) {

	m_num_ops = num_ops;
	m_space_efficiency = space_efficiency;
	m_slots_per_bucket = slots_per_bucket;
	m_num_buckets = (1.0f/m_space_efficiency) * m_num_ops / float(m_slots_per_bucket);

	m_random_keys = new std::string[m_num_ops];
	for (int i=0; i<m_num_ops; i++) {
		m_random_keys[i] = std::to_string(rand());
	}
}

template <typename T>
BenchmarkOptCuckooTagLockLaterHashMap<T>::~BenchmarkOptCuckooTagLockLaterHashMap() {

	delete[] m_random_keys;
}

template <typename T>
void BenchmarkOptCuckooTagLockLaterHashMap<T>::benchmark_write_only() {

	double start_time, end_time, best_time;
	int num_buckets = (1.0f/0.5f) * m_num_ops / float(m_slots_per_bucket);

	for (int num_writers = 2; num_writers <= NUM_WRITERS; num_writers += 1) {
		for (int i = 0; i < 10; i++) {
		    OptimisticCuckooTagBetterLockHashMap<T> my_map(num_buckets);

			pthread_t workers[num_writers];
			WorkerArgs args[num_writers];

			for (int j = 0; j < num_writers; j++) {
				args[j].my_map = (void*)&my_map;
				args[j].thread_id = (long int)j;
				args[j].num_threads = num_writers;
				args[j].num_ops = m_num_ops;
				args[j].percent_reads = 0.0f;
				args[j].keys = m_random_keys;
			}

			best_time = 1e30;
			start_time = CycleTimer::currentSeconds();
			for (int j = 0; j < num_writers; j++) {
			pthread_create(&workers[j], NULL, thread_send_requests<OptimisticCuckooTagBetterLockHashMap<std::string>>, &args[j]);
			}
			for (int j = 0; j < num_writers; j++) {
			pthread_join(workers[j], NULL);
			}
			end_time = CycleTimer::currentSeconds();
			best_time = std::min(best_time, end_time-start_time);
		}
	    std::cout << "\t" << "Write-Only (" << num_writers << " Writer Threads: "
	              << m_num_ops / best_time / (1000 * 1000) << std::endl;
	}
}

template <typename T>
void BenchmarkOptCuckooTagLockLaterHashMap<T>::run_all() {

	std::cout << "Benchmarking Optimistic Cuckoo Tag Lock Later HashMap, " << m_num_ops << " Operations..." << std::endl;

	benchmark_write_only();

    std::cout << std::endl;
}
