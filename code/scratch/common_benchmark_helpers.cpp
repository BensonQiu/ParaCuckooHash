#include "thread_send_requests.h"

template<typename T>
double write_only(std::string* keys, int num_ops, int slots_per_bucket) {

	int num_buckets = (1.0f/0.25f) * num_ops / float(slots_per_bucket);

	double start_time, end_time, best_time;
    best_time = 1e30;
    for (int i = 0; i < 10; i++) {
	    T my_map(num_buckets);

        start_time = CycleTimer::currentSeconds();
        for (int j = 0; j < num_ops; j++) {
            my_map.put(keys[j], keys[j]);
        }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
    }
    return best_time;
}

template <typename T>
double read_only(std::string* keys, int num_ops, int slots_per_bucket,
		int num_buckets, int num_readers,
		float read_percentage=1.0f, float space_efficiency=0.9f) {

	(void)slots_per_bucket;
	num_buckets = (1.0f/space_efficiency) * num_ops / float(slots_per_bucket);

	T my_map(num_buckets);
	// Warm up the hashmap.
	for (int i = 0; i < num_ops; i++) {
		my_map.put(keys[i], keys[i]);
	}

	// Setup thread args.
    pthread_t workers[num_readers];
    WorkerArgs args[num_readers];

    for (int i = 0; i < num_readers; i++) {
        args[i].my_map = (void*)&my_map;
        args[i].thread_id = (long int)i;
        args[i].num_threads = num_readers;
        args[i].num_ops = num_ops;
        args[i].percent_reads = read_percentage;
        args[i].keys = keys;
    }

	double start_time, end_time, best_time;

    best_time = 1e30;
    for (int i = 0; i < 5; i++) {
        start_time = CycleTimer::currentSeconds();
	    for (int j = 0; j < num_readers; j++) {
	        pthread_create(&workers[j], NULL,
	        	thread_send_requests<T>, &args[j]);
	    }
	    for (int j = 0; j < num_readers; j++) {
	        pthread_join(workers[j], NULL);
	    }
        end_time = CycleTimer::currentSeconds();
        best_time = std::min(best_time, end_time-start_time);
        // std::cout << "\t" << end_time-start_time << std::endl;
    }

	return best_time;
}
