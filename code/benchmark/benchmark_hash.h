#ifndef BENCHMARK_HASH_H
#define BENCHMARK_HASH_H

class BenchmarkHash {

	public:
		BenchmarkHash(
			int num_ops=2*1000*1000
		);
		~BenchmarkHash();

		void benchmark_std_hash();
		void benchmark_boost_hash();
		void benchmark_hashlittle();
		void benchmark_hashlittle2();
		void run_all();

	private:
		int m_num_ops;
		std::string* m_keys;

};

#include "benchmark_hash.cpp"

#endif
