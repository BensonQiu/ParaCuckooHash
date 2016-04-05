#include <iostream>
#include <unordered_map>

#include "hash_map.h"
#include "common/CycleTimer.h"

int main() {

    // ********** HASH MAP BENCHMARKING **********
    double startTime;
    double endTime;

    startTime = CycleTimer::currentSeconds();
    HashMap<int, std::string> my_map;
    for (int i = 0; i < 100 * 1000 * 1000; i++) {
        my_map.put(i, "random_value" + std::to_string(i));
    }

    std::cout << "Size: " << my_map.get_size() << " Load factor: " << my_map.get_load_factor() << std::endl;

    // for (int i = 0; i < 100 * 1000 * 1000; i++) {
    //     my_map.remove(i);
    // }

    endTime = CycleTimer::currentSeconds();
    std::cout << "My implementation time: " << endTime-startTime << "\n";



    startTime = CycleTimer::currentSeconds();
    std::unordered_map<int,std::string> ref_map;

    for (int i = 0; i < 100 * 1000 * 1000; i++) {
        ref_map.insert(std::pair<int,std::string>(i, "random_value" + std::to_string(i)));
    }

    // for (int i = 0; i < 100 * 1000 * 1000; i++) {
    //     ref_map.erase(i);
    // }

    endTime = CycleTimer::currentSeconds();
    std::cout << "Built-in time: " << endTime-startTime << "\n";

    return 0;
}
