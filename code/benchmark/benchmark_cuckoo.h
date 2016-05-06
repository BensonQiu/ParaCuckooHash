#ifndef BENCHMARK_CUCKOO_H
#define BENCHMARK_CUCKOO_H

#include "../cuckoo_hash_map.h"

template <typename T>
class BenchmarkCuckooHashMap {

	public:
		BenchmarkCuckooHashMap(
			int num_ops=2*1000*1000,
			float space_efficiency=0.9f,
			int slots_per_bucket=4
		);
		~BenchmarkCuckooHashMap();

		void benchmark_read_only();
		void benchmark_write_only();
		void benchmark_read_only_single_bucket();
		void benchmark_space_efficiency();
		void run_all();

	private:
		int m_num_ops;
		float m_space_efficiency;
		int m_slots_per_bucket;
		int m_num_buckets;
		std::string* m_random_keys;

		double m_benchmark_reads_helper(CuckooHashMap<T>* my_map);

};

#include "benchmark_cuckoo.cpp"


#endif
