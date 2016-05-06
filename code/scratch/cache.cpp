#include <stdlib.h>
#include <iostream>


/*
int main() {
    
    int size = 35 * 1000 * 1000; // 35MB 
    int* foo = new int[size];
    for (int i = 0; i < size; i++) {
        foo[i] = i;
    }
    
    for (int i = 0; i < size; i++) {
        int bar = foo[rand() % (size)];
    }
    return 0;
}
*/

int main() {
    int size = 10000;
    int** foo = new int*[size]();
    for (int i = size-1; i >=0; i--) {
        foo[i] = new int[size]();
    }
/*
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            foo[j][i] = 1;
        }
    }
*/
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            int bar = foo[j][i];
        }
    }
}
