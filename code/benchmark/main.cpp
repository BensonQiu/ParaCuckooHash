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
#include "benchmark_opt_cuckoo.h"

#include "../optimistic_cuckoo_tag_better_lock_hash_map.h"


#define NUM_THREADS 24
#define NUM_READERS 6
// #define NUM_WRITERS 2

//#define NUM_BUCKETS 10 * 1000 * 1000
//#define NUM_OPS 20 * 1000 * 1000

#define NUM_BUCKETS 5* 1000 * 1000
#define NUM_OPS 10 * 1000* 1000


int main() {

    std::cout << "***** Starting benchmark with " << NUM_OPS << " operations *****" << std::endl;

    // BenchmarkFineHashMap<std::string> benchmark_fine(NUM_OPS);
    // benchmark_fine.run_all();

    //BenchmarkCuckooHashMap<std::string> benchmark_cuckoo(NUM_OPS);
    //benchmark_cuckoo.run_all();

    //BenchmarkOptCuckooTagHashMap<std::string> benchmark_opt_cuckoo_tag(NUM_OPS);
    //benchmark_opt_cuckoo_tag.run_all();

    //BenchmarkOptCuckooHashMap<std::string> benchmark_opt_cuckoo(NUM_OPS);
    //benchmark_opt_cuckoo.run_all();



    std::string* keys = new std::string[NUM_OPS];
    for (int i = 0 ; i < NUM_OPS; i++)
        keys[i] = std::to_string(i);

    double start_time, end_time, best_put_time, best_get_time;



    best_put_time = 1e30;
    best_get_time = 1e30;

    /*
    for (int i = 0; i < 3; i++) {
      OptimisticCuckooTagBetterLockHashMap<std::string> my_map(0.6 * NUM_BUCKETS);

        // PUT

        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.put(keys[j], "value" + keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);

        // GET


        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.get(keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);

    }
    */

    for (int i = 0; i < 3; i++) {
      OptimisticCuckooTagBetterLockHashMap<std::string> my_map(0.55 * NUM_BUCKETS);

        // PUT

      /*
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.put(keys[j], "value" + keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);
      */


      pthread_t workers[NUM_THREADS];
      WorkerArgs args[NUM_THREADS];

      start_time = CycleTimer::currentSeconds();

        for (int j = 0; j < NUM_THREADS; j++) {
          args[j].my_map = (void*)&my_map;
          args[j].thread_id = (long int)j;
          args[j].num_threads = NUM_THREADS;
          args[j].num_ops = NUM_OPS;
          args[j].percent_reads = 0.0f;
          args[j].keys = keys;
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_create(&workers[j], NULL, thread_send_requests<OptimisticCuckooTagBetterLockHashMap<std::string>>, &args[j]);
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_join(workers[j], NULL);
        }

        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);

        // GET


        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.get(keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);



        // for (int j = 0; j < NUM_OPS; j++) {
        //     my_map.get(std::to_string(j));
        //     // my_map.get(keys[j]);
        // }

    }


    std::cout << "Cuckoo put: " << best_put_time << std::endl;
    std::cout << "Cuckoo get: " << best_get_time << std::endl;

    return 0;
}
