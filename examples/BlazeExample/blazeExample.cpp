#include <iostream>
#include <blaze/Math.h>

using blaze::StaticVector;
using blaze::DynamicVector;

int main() {
    std::cout << "Blaze example: \n\n";
    // Instantiation of a static 3D column vector. The vector is directly initialized as
    //    ( 4 -2  5 )
    StaticVector<int, 3UL> a{4, -2, 5};
    std::cout << "StaticVector<int, 3UL> a: \n" << a << "\n";
    // Instantiation of a dynamic 3D column vector. Via the subscript operator the values are set to
    //    ( 2  5 -3 )
    DynamicVector<int> b(3UL);
    b[0] = 2;
    b[1] = 5;
    b[2] = -3;
    std::cout << "DynamicVector<int> b: \n" << a << "\n";

    // Adding the vectors a and b
    DynamicVector<int> c = a + b;

    // Printing the result of the vector addition
    std::cout << "Adding a, b: \n";
    std::cout << "c =\n" << c << "\n";
    return 0;
}