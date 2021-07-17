#include <iostream>

#include <openblas/cblas.h>

int main() {
    std::cout << "OPENBLAS_L1_DATA_SIZE: " << OPENBLAS_L1_DATA_SIZE << std::endl;
    std::cout << "openblas_get_num_threads: " << openblas_get_num_threads() << std::endl;
    return 0;
}