#include <iostream>
#include <unordered_map>
#include <pthread.h>

#include "hash_map.h"
#include "coarse_hash_map.h"
#include "common/CycleTimer.h"
#include <boost/thread/locks.hpp>

#define NUM_THREADS 12
#define NUM_BUCKETS 35 * 1000 * 1000
#define NUM_OPS 50 * 1000 * 1000

struct WorkerArgs {
    CoarseHashMap<int,std::string> *my_map;
    int thread_id;
    int map_size;
};

void *thread_send_requests(void *threadArgs) {
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);
    CoarseHashMap<int,std::string> *my_map = args->my_map;
    int thread_ID = args->thread_id;
    int map_size = args->map_size;

    for (int i = thread_ID; i < map_size; i += NUM_THREADS) {
        my_map->put(i, "random_value" + std::to_string(i), thread_ID);
    }

}

void benchmark_hashmap() {

    std::cout << "***** Benchmarking sequential hashmap..." << std::endl;

    double start_time, end_time, best_time;

    // Time my sequential implementation. Output the best of three times.
    best_time = 1e30;

    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();
        
        HashMap<int, std::string> my_map(NUM_BUCKETS);
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.put(j, "random_value" + std::to_string(j));
        }

        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "My implementation time: " << best_time << "\n";


    // Time the built-in implementation.
    best_time = 1e30;

    for (int i = 0; i < 3; i++) {
        start_time = CycleTimer::currentSeconds();

        std::unordered_map<int,std::string> ref_map;
        for (int j = 0; j < NUM_OPS; j++) {
            ref_map.insert(std::pair<int,std::string>(j, "random_value" + std::to_string(j)));
        }

        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "Built-in time: " << best_time << "\n";

}

void benchmark_coarse_hashmap() {

    // unix3, 1M buckets, 10M inserts
    //   Without locking (sequential): 5.7 - 6.5 seconds

    std::cout << "***** Benchmarking coarse hashmap..." << std::endl;

    double start_time, end_time, best_time;

    best_time = 1e30;

    for (int i = 0; i < 1; i++) {
        start_time = CycleTimer::currentSeconds();
        
        CoarseHashMap<int, std::string> my_map(NUM_BUCKETS);

        pthread_t workers[NUM_THREADS];
        WorkerArgs args[NUM_THREADS];

        for (int i = 0; i < NUM_THREADS; i++) {
            args[i].my_map = &my_map;
            args[i].thread_id = (long int)i;
            args[i].map_size = NUM_OPS;
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_create(&workers[i], NULL, thread_send_requests, &args[i]);
        }

        for (int i = 0; i < NUM_THREADS; i++) {
            pthread_join(workers[i], NULL);
        }

        // std::cout << "Size: " << my_map.get_size() << " Load factor: " << my_map.get_load_factor() << std::endl;

        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "Coarse hashmap with " << NUM_THREADS << " threads: " << best_time << "\n";

    // Time the built-in implementation.
    best_time = 1e30;

    for (int i = 0; i < 1; i++) {
        start_time = CycleTimer::currentSeconds();

        std::unordered_map<int,std::string> ref_map;
        for (int j = 0; j < NUM_OPS; j++) {
            ref_map.insert(std::pair<int,std::string>(j, "random_value" + std::to_string(j)));
        }

        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    std::cout << "Built-in time: " << best_time << "\n";

}


int main() {

    benchmark_hashmap();
    benchmark_coarse_hashmap();
    
    return 0;
}
