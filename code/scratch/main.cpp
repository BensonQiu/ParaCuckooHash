#include <iostream>
#include <mutex>
#include <boost/thread/shared_mutex.hpp>

#include "CycleTimer.h"

#define NUM_OPS 10 * 1000 * 1000

int main() {

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

    std::cout << "Simple mutex time: " << best_time << std::endl;

    return 0;
}

