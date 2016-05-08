#ifndef COMMON_BENCHMARK_HELPERS_H
#define COMMON_BENCHMARK_HELPERS_H


template <typename T>
double write_only(std::string* keys, int num_ops, int slots_per_bucket);

template <typename T>
double read_only(std::string* keys, int num_ops, int slots_per_bucket,
		int num_buckets, int num_readers,
		float read_percentage=1.0f, float space_efficiency=0.9f);

#include "common_benchmark_helpers.cpp"

#endif
