#include <iostream>
#include <unordered_map>
#include <stdlib.h>

#include "../common/CycleTimer.h"

template <typename T>
BenchmarkUnorderedMap<T>::BenchmarkUnorderedMap(int num_ops) {

	m_num_ops = num_ops;

	m_random_keys = new std::string[m_num_ops];
	for (int i=0; i<m_num_ops; i++) {
	  m_random_keys[i] = std::to_string(rand());
	}
}

template <typename T>
BenchmarkUnorderedMap<T>::~BenchmarkUnorderedMap() {

	delete[] m_random_keys;
}

template <typename T>
void BenchmarkUnorderedMap<T>::benchmark_read_only() {

    std::unordered_map<std::string, std::string> my_map;
	for (int i = 0; i < m_num_ops; i++) {
        my_map.insert(std::pair<std::string,std::string>(m_random_keys[i], m_random_keys[i]));
    }

	double start_time, end_time, best_time;
    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
            my_map.find(m_random_keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "Read-Only: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

template <typename T>
void BenchmarkUnorderedMap<T>::benchmark_write_only() {

	double start_time, end_time, best_time;
    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
	    std::unordered_map<std::string, std::string> my_map;

        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
            my_map.insert(std::pair<std::string,std::string>(m_random_keys[j], m_random_keys[j]));
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }

    std::cout << "\t" << "Write-Only: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

template <typename T>
void BenchmarkUnorderedMap<T>::run_all() {

	std::cout << "Benchmarking Unordered Map, " << m_num_ops << " Operations..." << std::endl;

	benchmark_read_only();
	benchmark_write_only();

    std::cout << std::endl;
}
