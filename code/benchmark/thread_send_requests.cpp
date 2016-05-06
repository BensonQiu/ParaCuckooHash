#include <math.h>

#define EPSILON 0.0001f

template<typename T>
void* thread_send_requests(void* threadArgs) {
    WorkerArgs* args = static_cast<WorkerArgs*>(threadArgs);

    int thread_ID = args->thread_id;
    int num_threads = args->num_threads;
    int num_ops = args->num_ops;
    float percent_reads = args->percent_reads;
    float percent_writes = 1.0f - percent_reads;
    std::string* keys = args->keys;

    // TODO: Calculate start/end for interleaved read/writes.
    int chunk_start = thread_ID * num_ops / num_threads;
    int chunk_end = (thread_ID + 1) * num_ops / num_threads;
    if (thread_ID == num_threads - 1) {
        chunk_end = num_ops;
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
        std::cout << "TODO" << std::endl;
    }

    return NULL;
}

