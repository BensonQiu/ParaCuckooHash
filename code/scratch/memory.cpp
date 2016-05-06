// Allocate memory until out of memory

#include <iostream>
#include <stdlib.h>
#include <stdint.h>

int main() {
    
    int64_t counter = 0;
    while (true) {
        counter += 1000*1000*1000;
        std::cout << counter << std::endl;
        calloc(sizeof(int)*1000*1000*1000, 1);
    }
    return 0;
}
