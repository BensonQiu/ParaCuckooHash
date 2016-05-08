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
    //percentage of the ops that you're actually executing
    float ops_percentage = args->ops_percentage;
    std::string* keys = args->keys;

    // TODO: Calculate start/end for interleaved read/writes.
    int chunk_start = thread_ID * num_ops / num_threads;
    int chunk_end = (thread_ID + 1) * num_ops / num_threads;

    if (thread_ID == num_threads - 1) {
        chunk_end = num_ops;
    }

    // if number of ops is not 100, manually adjust the chunk ranges
    if (ops_percentage != 1.0f){

      int actual_ops =  (chunk_end - chunk_start) * ops_percentage;

      chunk_end = chunk_start + actual_ops;

      if (thread_ID == num_threads - 1){
        chunk_end = actual_ops;
      }
    }



    T* my_map = (T*)args->my_map;
    if (fabs(percent_reads - 1.0f) <= EPSILON) {
        for (int i = chunk_start; i < chunk_end; i++) {
            my_map->get(keys[i]);
        }
    }
    else if (fabs(percent_writes - 1.0f) <= EPSILON) {
      //std::cout << "chunk_start: " << chunk_start << " chunk_end: " << chunk_end << std::endl;
      for (int i = chunk_start; i < chunk_end; i++) {
        //std::cout << "Trying to put " << keys[i] << std::endl;
        my_map->put(keys[i], keys[i]);
      }
    }
    else {
      int write_range = percent_writes*100;
      //int write_begin = 100-write_range;
      // randomize write regions for each thread
      int write_begin = rand() % (100-write_range);
      //int write_begin = 100 - write_range;
      int write_end = write_begin + write_range;

      //std::cout << "Write Begin " << write_begin << " Write End " << write_end << std::endl;

      for (int i = chunk_start ; i < chunk_end; i++){
        int mod_by_100 = i % 100;

        //if write_begin <= mod_by_100 <= 99
        // do writes

        if (write_begin <= mod_by_100 && mod_by_100 <= write_end){
          //std::cout << "Put: " << keys[i] << std::endl;

          my_map->put(keys[i],keys[i]);
        }
        //else just do reads
        else{
          //std::cout << "Get: " << keys[i] << std::endl;

          my_map->get(keys[i]);
        }
      }
      //std::cout << "TODO" << std::endl;
    }

    return NULL;
}
