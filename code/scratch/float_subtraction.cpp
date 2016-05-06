#include <iostream>

int main() {

    std::cout << 1.0f - 0.0f << std::endl;

    float percent_reads = 0.0f;
    float percent_writes = 1.0f - percent_reads;

    if (percent_reads == 0.0f)
        std::cout << "a" << std::endl;

    if (percent_writes == 1.0f)
        std::cout << "b" << std::endl;
    else
	std::cout << "c" << percent_writes << std::endl;

    return 0;
}

