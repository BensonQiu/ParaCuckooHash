#ifndef BENCHMARK_OPT_CUCKOO_TAG_LOCKLATER_H
#define BENCHMARK_OPT_CUCKOO_TAG_LOCKLATER_H

#include "../optimistic_cuckoo_tag_better_lock_hash_map.h"

template <typename T>
class BenchmarkOptCuckooTagLockLaterHashMap {

	public:
		const int NUM_READERS = 24;
		const int NUM_WRITERS = 6;

		BenchmarkOptCuckooTagLockLaterHashMap(
			int num_ops=2*1000*1000,
			float space_efficiency=0.9f,
			int slots_per_bucket=4
		);
		~BenchmarkOptCuckooTagLockLaterHashMap();

		void benchmark_write_only();
		void run_all();

	private:
		int m_num_ops;
		float m_space_efficiency;
		int m_slots_per_bucket;
		int m_num_buckets;
		std::string* m_random_keys;

};

#include "benchmark_opt_cuckoo_tag_locklater.cpp"


#endif
