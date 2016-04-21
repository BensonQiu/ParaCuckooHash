#include <iostream>
#include <unordered_map>
#include <cassert>
#include <stdlib.h>
#include <pthread.h>

#include "hash_map.h"
#include "common/hash.h"
#include "coarse_hash_map.h"
#include "segment_hash_map.h"
#include "cuckoo_hash_map.h"
#include "better_cuckoo_hash_map.h"
#include "optimistic_cuckoo_hash_map.h"
#include "circular_queue.h"
#include "common/CycleTimer.h"

#define NUM_THREADS 48
#define NUM_BUCKETS 10 * 1000 * 1000
// #define NUM_OPS 20 * 1000 * 1000

// #define NUM_BUCKETS 2 * 1000 * 1000
#define NUM_OPS 20 * 1000 * 1000


struct CoarseWorkerArgs {
    CoarseHashMap<std::string,std::string> *my_map;
    int thread_id;
    std::string member_function;
};

struct OptimisticWorkerArgs {
    OptimisticCuckooHashMap<std::string> *my_map;
    int thread_id;
    std::string member_function;
};

struct SegmentWorkerArgs {
    SegmentHashMap<std::string> *my_map;
    int thread_id;
    std::string member_function;
};

void *coarse_thread_send_requests(void *threadArgs) {
    CoarseWorkerArgs* args = static_cast<CoarseWorkerArgs*>(threadArgs);
    CoarseHashMap<std::string,std::string> *my_map = args->my_map;
    int thread_ID = args->thread_id;
    std::string member_function = args->member_function;

    if (member_function.compare("put") == 0) {
        for (int i = thread_ID; i < NUM_OPS; i += NUM_THREADS) {
            my_map->put(std::to_string(i), "value" + std::to_string(i));
        }
    } else if (member_function.compare("get") == 0) {
       for (int i = thread_ID; i < NUM_OPS; i += NUM_THREADS) {
            my_map->get(std::to_string(i));
        }
    }
}

void *optimistic_thread_send_requests(void *threadArgs) {
    OptimisticWorkerArgs* args = static_cast<OptimisticWorkerArgs*>(threadArgs);
    OptimisticCuckooHashMap<std::string> *my_map = args->my_map;
    int thread_ID = args->thread_id;
    std::string member_function = args->member_function;

    if (member_function.compare("put") == 0) {
        for (int i = thread_ID; i < NUM_OPS; i += NUM_THREADS) {
            my_map->put(std::to_string(i), "value" + std::to_string(i));
        }
    } else if (member_function.compare("get") == 0) {
       for (int i = thread_ID; i < NUM_OPS; i += NUM_THREADS) {
            my_map->get(std::to_string(i));
        }
    }
}

void *segment_thread_send_requests(void *threadArgs) {
    SegmentWorkerArgs* args = static_cast<SegmentWorkerArgs*>(threadArgs);
    SegmentHashMap<std::string> *my_map = args->my_map;
    int thread_ID = args->thread_id;
    std::string member_function = args->member_function;

    if (member_function.compare("put") == 0) {
        for (int i = thread_ID; i < NUM_OPS; i += NUM_THREADS) {
            my_map->put(std::to_string(i), "value" + std::to_string(i));
        }
    } else if (member_function.compare("get") == 0) {
       for (int i = thread_ID; i < NUM_OPS; i += NUM_THREADS) {
            my_map->get(std::to_string(i));
        }
    }
}


void benchmark_builtin_unorderedmap() {

    std::cout << "\nBenchmarking built-in unordered map..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        std::unordered_map<std::string,std::string> ref_map;

        // INSERT
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            ref_map.insert(std::pair<std::string,std::string>(std::to_string(j), "value" + std::to_string(j)));
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);

        // GET
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            ref_map.find(std::to_string(j));
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);

    }
    std::cout << "Built-in put: " << best_put_time << std::endl;
    std::cout << "Built-in get: " << best_get_time << std::endl;

}


void benchmark_hashmap() {

    std::cout << "\nBenchmarking sequential hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    // Time my sequential implementation. Output the best of three times.
    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        HashMap<std::string, std::string> my_map(NUM_BUCKETS);

        // PUT
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.put(std::to_string(j), "value" + std::to_string(j));
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);


        // GET
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.get(std::to_string(j));
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);
    }
    std::cout << "Sequential put: " << best_put_time << std::endl;
    std::cout << "Sequential get: " << best_get_time << std::endl;

}


void benchmark_coarse_hashmap() {

    std::cout << "\nBenchmarking coarse hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        CoarseHashMap<std::string, std::string> my_map(NUM_BUCKETS);

        // PUT
        start_time = CycleTimer::currentSeconds();

        pthread_t workers[NUM_THREADS];
        CoarseWorkerArgs args[NUM_THREADS];

        for (int j = 0; j < NUM_THREADS; j++) {
            args[j].my_map = &my_map;
            args[j].thread_id = (long int)j;
            args[j].member_function = "put";
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_create(&workers[j], NULL, coarse_thread_send_requests, &args[j]);
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_join(workers[j], NULL);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);


        // GET
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_THREADS; j++) {
            args[j].my_map = &my_map;
            args[j].thread_id = (long int)j;
            args[j].member_function = "get";
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_create(&workers[j], NULL, coarse_thread_send_requests, &args[j]);
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_join(workers[j], NULL);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);

        // std::cout << best_put_time << std::endl;
        // std::cout << best_get_time << std::endl << std::endl;
    }
    std::cout << "Coarse (" << NUM_THREADS << " threads): put: " << best_put_time << std::endl;
    std::cout << "Coarse (" << NUM_THREADS << " threads): get: " << best_get_time << std::endl;
}


void benchmark_cuckoo_hashmap() {

    std::cout << "\nBenchmarking cuckoo hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        CuckooHashMap<std::string> my_map(20 * NUM_BUCKETS);

        // PUT
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            std::string key = std::to_string(j);
            my_map.put(key, "value" + key);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);


        // GET
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            std::string key = std::to_string(j);
            my_map.get(key);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);
    }
    std::cout << "Cuckoo put: " << best_put_time << std::endl;
    std::cout << "Cuckoo get: " << best_get_time << std::endl;
}


void benchmark_better_cuckoo_hashmap() {

    std::cout << "\nBenchmarking better cuckoo hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        BetterCuckooHashMap<std::string> my_map(20 * NUM_BUCKETS);

        // PUT
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            std::string key = std::to_string(j);
            my_map.put(key, "value" + key);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);


        // GET
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            std::string key = std::to_string(j);
            my_map.get(key);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);
    }
    std::cout << "BetterCuckoo put: " << best_put_time << std::endl;
    std::cout << "BetterCuckoo get: " << best_get_time << std::endl;
}


void benchmark_optimistic_cuckoo_hashmap() {

    std::cout << "\nBenchmarking optimistic cuckoo hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        OptimisticCuckooHashMap<std::string> my_map(20*NUM_BUCKETS);

        // PUT
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            std::string key = std::to_string(j);
            my_map.put(key, "value" + key);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);


        // GET
        start_time = CycleTimer::currentSeconds();

        pthread_t workers[NUM_THREADS];
        OptimisticWorkerArgs args[NUM_THREADS];

        for (int j = 0; j < NUM_THREADS; j++) {
            args[j].my_map = &my_map;
            args[j].thread_id = (long int)j;
            args[j].member_function = "get";
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_create(&workers[j], NULL, optimistic_thread_send_requests, &args[j]);
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_join(workers[j], NULL);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);

        // std::cout << best_put_time << std::endl;
        // std::cout << best_get_time << std::endl << std::endl;
    }
    std::cout << "Optimistic Cuckoo (" << NUM_THREADS << " threads): put: " << best_put_time << std::endl;
    std::cout << "Optimistic Cuckoo (" << NUM_THREADS << " threads): get: " << best_get_time << std::endl;
}


void benchmark_segment_hashmap() {

    std::cout << "\nBenchmarking segment hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        SegmentHashMap<std::string> my_map(20*NUM_BUCKETS, 32);

        pthread_t workers[NUM_THREADS];
        SegmentWorkerArgs args[NUM_THREADS];

        // PUT
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            std::string key = std::to_string(j);
            my_map.put(key, "value" + key);
        }
        // for (int j = 0; j < NUM_THREADS; j++) {
        //     args[j].my_map = &my_map;
        //     args[j].thread_id = (long int)j;
        //     args[j].member_function = "put";
        // }
        // for (int j = 0; j < NUM_THREADS; j++) {
        //     pthread_create(&workers[j], NULL, segment_thread_send_requests, &args[j]);
        // }
        // for (int j = 0; j < NUM_THREADS; j++) {
        //     pthread_join(workers[j], NULL);
        // }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);


        // GET
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_THREADS; j++) {
            args[j].my_map = &my_map;
            args[j].thread_id = (long int)j;
            args[j].member_function = "get";
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_create(&workers[j], NULL, segment_thread_send_requests, &args[j]);
        }
        for (int j = 0; j < NUM_THREADS; j++) {
            pthread_join(workers[j], NULL);
        }
        end_time = CycleTimer::currentSeconds();
        best_get_time = std::min(best_get_time, end_time-start_time);

        std::cout << best_put_time << std::endl;
        std::cout << best_get_time << std::endl << std::endl;
    }
    std::cout << "Segment Cuckoo (" << NUM_THREADS << " threads): put: " << best_put_time << std::endl;
    std::cout << "Segment Cuckoo (" << NUM_THREADS << " threads): get: " << best_get_time << std::endl;
}


/*
void benchmark_mutex_types() {

    std::cout << "\nBenchmarking mutexes..." << std::endl;

    double start_time, end_time, best_time;

    best_time = 1e30;

    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        boost::shared_mutex mutex;
        for (int j = 0; j < NUM_OPS; j++) {
            boost::shared_lock<boost::shared_mutex> lock(mutex);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "Shared mutex time: " << best_time << std::endl;

    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        std::mutex mutex;
        for (int j = 0; j < NUM_OPS; j++) {
            mutex.lock();
            mutex.unlock();
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "Reg mutex time: " << best_time << std::endl;
}
*/

void benchmark_atomic_operations() {

    std::cout << "\nBenchmarking atomic operations..." << std::endl;

    double start_time, end_time, best_time;

    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        int x = 0;
        for (int j = 0; j < NUM_OPS; j++) {
            __sync_fetch_and_add(&x, 0);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "__sync_fetch_and_add time: " << best_time << std::endl;
}


void benchmark_mod() {

    std::cout << "\nBenchmarking mod..." << std::endl;

    double start_time, end_time, best_time;

    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            int x = 4234324 % -2351;
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "default mod time: " << best_time << std::endl;

    best_time = 1e30;
    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            int x = 4234324 % -2351;
            x = abs(x);
            x = NUM_BUCKETS - x;
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "nonnegative mod time: " << best_time << std::endl;

}

inline int fastrand() {
  int g_seed = (214013*g_seed+2531011);
  return (g_seed>>16)&0x7FFF;
}

void benchmark_random() {

    std::cout << "\nBenchmarking rand..." << std::endl;

    double start_time, end_time, best_rand_time, best_fast_rand_time;

    best_rand_time = 1e30;
    best_fast_rand_time = 1e30;
    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            int x = rand();
        }
        end_time = CycleTimer::currentSeconds();
        best_rand_time = std::min(best_rand_time, end_time-start_time);

        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            int x = fastrand();
        }
        end_time = CycleTimer::currentSeconds();
        best_fast_rand_time = std::min(best_fast_rand_time, end_time-start_time);

    }

    std::cout << "rand() time: " << best_rand_time << std::endl;
    std::cout << "fast_rand() time: " << best_fast_rand_time << std::endl;

}


int main() {

    std::cout << "Starting benchmark with NUM_BUCKETS: " << NUM_BUCKETS
              << " and NUM_OPS: " << NUM_OPS << std::endl;

    // benchmark_builtin_unorderedmap();
    // benchmark_hashmap();
    // benchmark_coarse_hashmap();
    // benchmark_cuckoo_hashmap();
    // benchmark_better_cuckoo_hashmap();
    // benchmark_optimistic_cuckoo_hashmap();
    benchmark_segment_hashmap();

    // benchmark_mutex_types();
    // benchmark_atomic_operations();
    // benchmark_mod();
    // benchmark_random();

    SegmentHashMap<std::string> my_map();

    return 0;
}
