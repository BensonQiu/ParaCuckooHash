#ifndef BENCHMARK_UNORDERED_MAP_H
#define BENCHMARK_UNORDERED_MAP_H

template <typename T>
class BenchmarkUnorderedMap {

	public:
		BenchmarkUnorderedMap(
			int num_ops=2*1000*1000
		);
		~BenchmarkUnorderedMap();

		void benchmark_read_only();
		void benchmark_write_only();
		void run_all();

	private:
		int m_num_ops;
		std::string* m_random_keys;

};

#include "benchmark_unordered_map.cpp"

#endif
