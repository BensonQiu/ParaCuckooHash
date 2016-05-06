#ifndef BENCHMARK_OPT_CUCKOO_TAG_H
#define BENCHMARK_OPT_CUCKOO_TAG_H

#include "../optimistic_cuckoo_tag_hash_map.h"

template <typename T>
class BenchmarkOptCuckooTagHashMap {

	public:
		const int NUM_READERS = 24;
		const int NUM_WRITERS = 8;

		BenchmarkOptCuckooTagHashMap(
			int num_ops=2*1000*1000,
			float space_efficiency=0.9f,
			int slots_per_bucket=4
		);
		~BenchmarkOptCuckooTagHashMap();

		void benchmark_random_interleaved_read_write();
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

		double m_benchmark_reads_helper(OptimisticCuckooTagHashMap<T>* my_map);

};

#include "benchmark_opt_cuckoo_tag.cpp"


#endif
