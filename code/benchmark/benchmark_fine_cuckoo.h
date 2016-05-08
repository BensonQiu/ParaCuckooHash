#ifndef BENCHMARK_FINE_CUCKOO
#define BENCHMARK_FINE_CUCKOO

#include "../cuckoo_fine_hash_map.h"

template <typename T>
class BenchmarkFineCuckoo {

	public:
		const int NUM_READERS = 24;
		const int NUM_WRITERS = 8;

		BenchmarkFineCuckoo(
			int num_ops=2*1000*1000,
			float space_efficiency=0.9f,
			int slots_per_bucket=4
		);
		~BenchmarkFineCuckoo();

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

		double m_benchmark_reads_helper(CuckooFineHashMap<T>* my_map, float read_percentage=1.0f, float opts_percentage=1.0f);

};

#include "benchmark_fine_cuckoo.cpp"

#endif
