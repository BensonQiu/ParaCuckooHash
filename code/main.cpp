#include <iostream>
#include <unordered_map>
#include <pthread.h>

#include "hash_map.h"
#include "common/hash.h"
#include "coarse_hash_map.h"
#include "cuckoo_hash_map.h"
#include "better_cuckoo_hash_map.h"
#include "common/CycleTimer.h"

#define NUM_THREADS 24
#define NUM_BUCKETS 5 * 1000 * 1000
#define NUM_OPS 10 * 1000 * 1000

// #define NUM_BUCKETS 2
// #define NUM_OPS 10

struct WorkerArgs {
    CoarseHashMap<std::string,std::string> *my_map;
    int thread_id;
    std::string member_function;
};


void *thread_send_requests(void *threadArgs) {
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);
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
    std::cout << "Built-in put: " << best_put_time << "\n";
    std::cout << "Built-in get: " << best_get_time << "\n";

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

    // RW 12 threads, 10M/20M: 6 put, 3.3 get
    // RW 24 threads, 10M/20M: 4 put, 2.5 get
    // Bucket 12 threads, 10M/20M: 6.4 put, 2.3 get
    // Bucket 24 threads, 10M/20M: 4 put, 1.4 get

    std::cout << "\nBenchmarking coarse hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        CoarseHashMap<std::string, std::string> my_map(NUM_BUCKETS);

        // PUT
        start_time = CycleTimer::currentSeconds();

        pthread_t workers[NUM_THREADS];
        WorkerArgs args[NUM_THREADS];

        for (int i = 0; i < NUM_THREADS; i++) {
            args[i].my_map = &my_map;
            args[i].thread_id = (long int)i;
            args[i].member_function = "put";
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_create(&workers[i], NULL, thread_send_requests, &args[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(workers[i], NULL);
        }
        end_time = CycleTimer::currentSeconds();
        best_put_time = std::min(best_put_time, end_time-start_time);


        // GET
        start_time = CycleTimer::currentSeconds();

        for (int i = 0; i < NUM_THREADS; i++) {
            args[i].my_map = &my_map;
            args[i].thread_id = (long int)i;
            args[i].member_function = "get";
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_create(&workers[i], NULL, thread_send_requests, &args[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(workers[i], NULL);
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
    std::cout << "Cuckoo put: " << best_put_time << "\n";
    std::cout << "Cuckoo get: " << best_get_time << "\n";
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


int main() {

    std::cout << "Starting benchmark with NUM_BUCKETS: " << NUM_BUCKETS
              << " and NUM_OPS: " << NUM_OPS << std::endl;

    benchmark_builtin_unorderedmap();
    benchmark_hashmap();
    benchmark_coarse_hashmap();
    benchmark_cuckoo_hashmap();
    benchmark_better_cuckoo_hashmap();
    benchmark_mutex_types();

    return 0;
}
