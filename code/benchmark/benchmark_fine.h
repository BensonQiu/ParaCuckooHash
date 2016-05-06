#ifndef BENCHMARK_FINE_H
#define BENCHMARK_FINE_H


template <typename T>
class BenchmarkFineHashMap {

	public:
		const int NUM_READERS = 24;
		const int NUM_WRITERS = 8;

		BenchmarkFineHashMap(
			int num_ops=2*1000*1000,
			float load_factor=4.0f
		);
		~BenchmarkFineHashMap();

		void benchmark_random_interleaved_read_write();
		void benchmark_read_only();
		void benchmark_write_only();
		void benchmark_read_only_single_bucket();
		void run_all();

	private:
		int m_num_ops;
		float m_load_factor;
		int m_table_size;
		std::string* m_random_keys;
};

#include "benchmark_fine.cpp"


#endif
