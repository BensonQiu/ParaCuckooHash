#ifndef THREAD_SEND_ARGS_H
#define THREAD_SEND_ARGS_H


struct WorkerArgs {
    void* my_map;
    int thread_id;
    int num_threads;
    int num_ops;
    float percent_reads;
    std::string* keys;
};

template<typename T>
void* thread_send_requests(void* threadArgs);

#include "thread_send_requests.cpp"

#endif