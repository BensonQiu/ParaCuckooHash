#include <iostream>
#include <stdlib.h>

#include "../common/CycleTimer.h"
#include "../common/hash.h"

#include <boost/functional/hash.hpp>


BenchmarkHash::BenchmarkHash(int num_ops) {

	m_num_ops = num_ops;

	m_keys = new std::string[m_num_ops];
	for (int i=0; i<m_num_ops; i++) {
        m_keys[i] = std::to_string(i);
	}
}

BenchmarkHash::~BenchmarkHash() {

	delete[] m_keys;
}

void BenchmarkHash::benchmark_std_hash() {

	double start_time, end_time, best_time;
    best_time = 1e30;
    for (int i = 0; i < 10; i++) {
        start_time = CycleTimer::currentSeconds();
        std::hash<std::string> string_hasher;
        for (int j = 0; j < m_num_ops; j++) {
            string_hasher(m_keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "std::hash: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

void BenchmarkHash::benchmark_boost_hash() {

    double start_time, end_time, best_time;
    best_time = 1e30;
    for (int i = 0; i < 10; i++) {
        start_time = CycleTimer::currentSeconds();
        boost::hash<std::string> string_hasher;
        for (int j = 0; j < m_num_ops; j++) {
            string_hasher(m_keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "Boost: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

void BenchmarkHash::benchmark_hashlittle() {

    double start_time, end_time, best_time;
    best_time = 1e30;
    for (int i = 0; i < 10; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
            uint32_t h1 = 0;
            hashlittle(m_keys[i].c_str(), m_keys[i].length(), h1);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "hashlittle: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

void BenchmarkHash::benchmark_hashlittle2() {

    double start_time, end_time, best_time;
    best_time = 1e30;
    for (int i = 0; i < 10; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < m_num_ops; j++) {
            uint32_t h1, h2;
            h1 = h2 = 0;
            hashlittle2(m_keys[i].c_str(), m_keys[i].length(), &h1, &h2);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "\t" << "hashlittle2: " << m_num_ops / best_time / (1000 * 1000) << std::endl;
}

void BenchmarkHash::run_all() {

	std::cout << "Benchmarking Hash Functions, " << m_num_ops << " Operations..." << std::endl;

    benchmark_std_hash();
    benchmark_boost_hash();
    benchmark_hashlittle();
    benchmark_hashlittle2();

    std::cout << std::endl;
}
