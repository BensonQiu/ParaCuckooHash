#include <math.h>
#include <iomanip>


#define EPSILON 0.0001f

template<typename T>
void* thread_send_requests(void* threadArgs) {
WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    int thread_ID = args->thread_id;
    int num_threads = args->num_threads;
    int num_ops = args->num_ops;
    float percent_reads = args->percent_reads;
    float percent_writes = 1.0f - percent_reads;
    float ops_percentage = args->ops_percentage; // Percentage of operations
                                                 // that we actually execute.
    std::string* keys = args->keys;

    int chunk_start = thread_ID * num_ops / num_threads;
    int chunk_end = (thread_ID + 1) * num_ops / num_threads;

    if (thread_ID == num_threads - 1) {
        chunk_end = num_ops;
    }

    // if number of ops is not 100, manually adjust the chunk ranges
    if (ops_percentage != 1.0f) {
        int actual_ops =  (chunk_end - chunk_start) * ops_percentage;
        chunk_end = chunk_start + actual_ops;
        if (thread_ID == num_threads - 1) {
          chunk_end = actual_ops;
        }
    }

    T* my_map = (T*)args->my_map;
    if (fabs(percent_reads - 1.0f) <= EPSILON) {
        for (int i = chunk_start; i < chunk_end; i++) {
            my_map->get(keys[i]);
        }
    } else if (fabs(percent_writes - 1.0f) <= EPSILON) {
      for (int i = chunk_start; i < chunk_end; i++) {
        my_map->put(keys[i], keys[i]);
      }
    } else {
        int write_range = percent_writes*100;
        // Randomize write regions for each thread
        int write_begin = rand() % (100-write_range);
        int write_end = write_begin + write_range;

        for (int i = chunk_start ; i < chunk_end; i++){
            int mod_by_100 = i % 100;

            if (write_begin <= mod_by_100 && mod_by_100 <= write_end) {
                my_map->put(keys[i],keys[i]);
            } else {
                my_map->get(keys[i]);
            }
        }
    }
    return NULL;
}
