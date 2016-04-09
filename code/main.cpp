#include <iostream>
#include <unordered_map>
#include <pthread.h>

#include "hash_map.h"
// #include "common/hash.c"
#include "coarse_hash_map.h"
#include "cuckoo_hash_map.h"
// #include "better_cuckoo_hash_map.h"
#include "common/CycleTimer.h"
#include <boost/thread/locks.hpp>

#define NUM_THREADS 12
#define NUM_BUCKETS 5 * 1000 * 1000
#define NUM_OPS 10 * 1000 * 1000

// #define NUM_BUCKETS 5
// #define NUM_OPS 5

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

    double start_time, end_time, best_put_time, best_get_time;

    // Time my sequential implementation. Output the best of three times.
    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        HashMap<std::string, std::string> my_map(NUM_BUCKETS);

        // PUT
        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < NUM_OPS; j++) {
            my_map.put(std::to_string(j), "random_value" + std::to_string(j));
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
    std::cout << "My implementation put: " << best_put_time << "\n";
    std::cout << "My implementation get: " << best_get_time << "\n";

}

void benchmark_coarse_hashmap() {

    // unix3, 1M buckets, 10M inserts
    //   Without locking (sequential): 5.7 - 6.5 seconds

    std::cout << "***** Benchmarking coarse hashmap..." << std::endl;

    double start_time, end_time, best_time;

    best_time = 1e30;

    for (int i = 0; i < 3; i++) {
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

void benchmark_cuckoo_hashmap() {

    std::cout << "***** Benchmarking cuckoo hashmap..." << std::endl;

    double start_time, end_time, best_put_time, best_get_time;

    best_put_time = 1e30;
    best_get_time = 1e30;
    for (int i = 0; i < 3; i++) {
        CuckooHashMap<std::string> my_map(NUM_BUCKETS*10);

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
    std::cout << "Cuckoo put time: " << best_put_time << "\n";
    std::cout << "Cuckoo get time: " << best_get_time << "\n";
}


// void benchmark_better_cuckoo_hashmap() {

//     double start_time, end_time, best_put_time, best_get_time;

//     best_put_time = 1e30;
//     best_get_time = 1e30;
//     for (int i = 0; i < 3; i++) {
//         BetterCuckooHashMap<std::string> my_map(50000);

//         // PUT
//         start_time = CycleTimer::currentSeconds();

//         for (int j = 0; j < 25000; j++) {
//             std::string key = std::to_string(j);
//             my_map.put(key, "value" + key);
//         }

//         end_time = CycleTimer::currentSeconds();
//         best_put_time = std::min(best_put_time, end_time-start_time);


//         // GET
//         start_time = CycleTimer::currentSeconds();

//         for (int j = 0; j < 25000; j++) {
//             std::string key = std::to_string(j);
//             my_map.get(key);
//         }

//         end_time = CycleTimer::currentSeconds();
//         best_get_time = std::min(best_get_time, end_time-start_time);
//         std::cout << best_put_time << ", " << best_get_time << std::endl;
//     }
//     std::cout << "Cuckoo put time: " << best_put_time << "\n";
//     std::cout << "Cuckoo get time: " << best_get_time << "\n";
// }


int main() {
    
    // uint32_t h1, h2;
    // hashlittle2((void*)"t0by", 4, &h1, &h2);
    // std::cout << h1 << " " << h2 << std::endl;

    // benchmark_hashmap();
    // benchmark_coarse_hashmap();
    benchmark_cuckoo_hashmap();

    // benchmark_better_cuckoo_hashmap();

    // for (int yolo = 0; yolo < 3; yolo++) {
    //     double start_time = CycleTimer::currentSeconds();
    //     for (int i = 0; i < NUM_OPS; i++) {
    //         uint32_t h1, h2;
    //         h1 = 0;
    //         h2 = 0;

    //         std::string curr_key = std::to_string(i);
    //         hashlittle2(curr_key.c_str(), curr_key.length(), &h1, &h2);
    //     }
    //     double end_time = CycleTimer::currentSeconds();
    //     std::cout << end_time-start_time << std::endl;
    // }

    // for (int yolo = 0; yolo < 3; yolo++) {
    //     double start_time = CycleTimer::currentSeconds();
    //     for (int i = 0; i < NUM_OPS; i++) {
    //         std::hash<std::string> hasher;
    //         int bucket = hasher(std::to_string(i));

    //         // std::string curr_key = std::to_string(i);
    //         // hashlittle2(curr_key.c_str(), curr_key.length(), &h1, &h2);
    //     }
    //     double end_time = CycleTimer::currentSeconds();
    //     std::cout << end_time-start_time << std::endl;
    // }

            // uint32_t r =  hv & m_tagmask;
            // return (unsigned char) r + (r == 0);

    return 0;
}
