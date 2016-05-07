#include <iostream>
#include <stdlib.h>
#include <pthread.h>

#include "../common/CycleTimer.h"
#include "../common/errors.h"
#include "thread_send_requests.h"

template <typename T>
BenchmarkCuckooHashMap<T>::BenchmarkCuckooHashMap(
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
BenchmarkCuckooHashMap<T>::~BenchmarkCuckooHashMap() {

	delete[] m_random_keys;
}

template <typename T>
void BenchmarkCuckooHashMap<T>::benchmark_read_only() {

    CuckooHashMap<T> my_map(m_num_buckets);
    double best_time = m_benchmark_reads_helper(&my_map);

    std::cout << "\t" << "Read-Only: " << best_time << std::endl;
}

template <typename T>
void BenchmarkCuckooHashMap<T>::benchmark_write_only() {

	double start_time, end_time, best_time;

    best_time = 1e30;
    for (int i = 0; i < 10; i++) {
	    CuckooHashMap<T> my_map(m_num_buckets);

        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
            my_map.put(m_random_keys[j], m_random_keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "Write-Only: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

template <typename T>
void BenchmarkCuckooHashMap<T>::benchmark_read_only_single_bucket() {

    CuckooHashMap<T> my_map(m_num_buckets);
	for (int i = 0; i < m_num_ops; i++) {
		my_map.put(m_random_keys[i], m_random_keys[i]);
	}

	std::string single_key = m_random_keys[0];
	double start_time, end_time, best_time;

	best_time = 1e30;
	for (int i = 0; i < 10; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
        	my_map.get(single_key);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
	}
    std::cout << "\t" << "Read-Only Single Bucket: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

template <typename T>
void BenchmarkCuckooHashMap<T>::benchmark_space_efficiency() {

	for (float space_efficiency = 0.15f; space_efficiency <= 0.9f; space_efficiency += 0.15f) {
		int num_buckets = (1.0f/space_efficiency) * m_num_ops / float(m_slots_per_bucket);
	    CuckooHashMap<T> my_map(num_buckets);
		double best_time = m_benchmark_reads_helper(&my_map);

	    std::cout << "\t" << 100*space_efficiency << "% Space Efficiency: "
	              << m_num_ops / best_time / (1000 * 1000) << std::endl;
	}
}

template <typename T>
double BenchmarkCuckooHashMap<T>::m_benchmark_reads_helper(CuckooHashMap<T>* my_map) {

	// Warm up the hashmap with sequential insertions.
	for (int i = 0; i < m_num_ops; i++) {
		my_map->put(m_random_keys[i], m_random_keys[i]);
	}

	double start_time, end_time, best_time;

	best_time = 1e30;
	for (int i = 0; i < 10; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
        	my_map->get(m_random_keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
	}
	return best_time;
}

template <typename T>
void BenchmarkCuckooHashMap<T>::run_all() {

	std::cout << "Benchmarking Cuckoo HashMap, " << m_num_ops << " Operations..." << std::endl;

	benchmark_read_only();
	benchmark_write_only();
	benchmark_read_only_single_bucket();
	benchmark_space_efficiency();

	std::cout << std::endl;
}
