#include <iostream>
#include <unordered_map>
#include <cassert>
#include <stdlib.h>
#include <pthread.h>

#include "../hash_map.h"
#include "../common/hash.h"
#include "../common/CycleTimer.h"

#include "benchmark_fine.h"
#include "benchmark_cuckoo.h"
#include "benchmark_opt_cuckoo_tag.h"

#define NUM_THREADS 16
#define NUM_READERS 6
// #define NUM_WRITERS 2

// #define NUM_BUCKETS 10 * 1000 * 1000
// #define NUM_OPS 20 * 1000 * 1000

#define NUM_BUCKETS 1 * 1000 * 1000
#define NUM_OPS 2 * 1000 * 1000

int main() {

    std::cout << "***** Starting benchmark with " << NUM_OPS << " operations *****" << std::endl;

    BenchmarkFineHashMap<std::string> benchmark_fine(NUM_OPS);
    // benchmark_fine.run_all();

    BenchmarkCuckooHashMap<std::string> benchmark_cuckoo(NUM_OPS);
    benchmark_cuckoo.run_all();

    BenchmarkOptCuckooTagHashMap<std::string> benchmark_opt_cuckoo_tag(NUM_OPS);
    // benchmark_opt_cuckoo_tag.run_all();

    return 0;
}
