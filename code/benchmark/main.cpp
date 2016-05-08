#include <iostream>
#include <unordered_map>
#include <cassert>
#include <stdlib.h>
#include <pthread.h>

#include "../hash_map.h"
#include "../common/CycleTimer.h"

#include "benchmark_unordered_map.h"
#include "benchmark_fine.h"
#include "benchmark_cuckoo.h"
#include "benchmark_fine_cuckoo.h"
#include "benchmark_opt_cuckoo.h"
#include "benchmark_opt_cuckoo_tag.h"
#include "benchmark_opt_cuckoo_tag_locklater.h"
#include "benchmark_hash.h"

#include "../cuckoo_fine_hash_map.h"
// #include "../optimistic_cuckoo_tag_locklater_hash_map.h"

#define NUM_THREADS 24
#define NUM_READERS 6
// #define NUM_WRITERS 2

//#define NUM_BUCKETS 10 * 1000 * 1000
//#define NUM_OPS 20 * 1000 * 1000

// #define NUM_BUCKETS 1 * 1000 * 1000
// #define NUM_OPS 1 * 1000 * 1000

#define NUM_OPS 100

void benchmark_fine_cuckoo() {

    std::string* keys = new std::string[NUM_OPS];
    for (int i = 0 ; i < NUM_OPS; i++) {
        keys[i] = std::to_string(i);
    }

    double start_time, end_time, best_put_time, best_get_time;
    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {

        // PUT
        int num_buckets = (1.0f/0.25f) * NUM_OPS / float(4);
        CuckooFineHashMap<std::string> my_map(num_buckets);

        pthread_t workers[NUM_THREADS];
        WorkerArgs args[NUM_THREADS];

        for (int j = 0; j < NUM_THREADS; j++) {
            args[j].my_map = (void*)&my_map;
            args[j].thread_id = (long int)j;
            args[j].num_threads = NUM_THREADS;
            args[j].num_ops = NUM_OPS;
            args[j].percent_reads = 0.0f;
            args[j].keys = keys;
        }

        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_create(&workers[j], NULL, thread_send_requests<CuckooFineHashMap<std::string>>, &args[j]);
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_join(workers[j], NULL);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);
        std::cout << "Put: " << end_time-start_time << std::endl;

        // GET
        for (int j = 0; j < NUM_THREADS; j++) {
            args[j].my_map = (void*)&my_map;
            args[j].thread_id = (long int)j;
            args[j].num_threads = NUM_THREADS;
            args[j].num_ops = NUM_OPS;
            args[j].percent_reads = 1.0f;
            args[j].keys = keys;
        }
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_create(&workers[j], NULL, thread_send_requests<CuckooFineHashMap<std::string>>, &args[j]);
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_join(workers[j], NULL);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);
        std::cout << "Get: " << end_time-start_time << std::endl;
    }

    std::cout << "Cuckoo put (" << NUM_THREADS << " Threads): "
              << NUM_OPS / best_put_time / (1000 * 1000) << std::endl;
    std::cout << "Cuckoo get (" << NUM_THREADS << " Threads): "
              << NUM_OPS / best_get_time / (1000 * 1000) << std::endl;

}

int main() {

    std::cout << "***** Starting benchmark with " << NUM_OPS << " operations *****" << std::endl;

    BenchmarkUnorderedMap<std::string> benchmark_unordered_map(NUM_OPS);
    benchmark_unordered_map.run_all();

    BenchmarkFineHashMap<std::string> benchmark_fine(NUM_OPS);
    benchmark_fine.run_all();

    BenchmarkCuckooHashMap<std::string> benchmark_cuckoo(NUM_OPS);
    benchmark_cuckoo.run_all();

    BenchmarkFineCuckoo<std::string> benchmark_fine_cuckoo(NUM_OPS);
    benchmark_fine_cuckoo.run_all();

    BenchmarkOptCuckooHashMap<std::string> benchmark_opt_cuckoo(NUM_OPS);
    benchmark_opt_cuckoo.run_all();

    BenchmarkOptCuckooTagHashMap<std::string> benchmark_opt_cuckoo_tag(NUM_OPS);
    benchmark_opt_cuckoo_tag.run_all();

    BenchmarkOptCuckooTagLockLaterHashMap<std::string> benchmark_opt_cuckoo_tag_ll(NUM_OPS);
    benchmark_opt_cuckoo_tag_ll.run_all();

    BenchmarkHash benchmark_hash(NUM_OPS);
    benchmark_hash.run_all();
}
