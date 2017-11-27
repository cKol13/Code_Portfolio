#include "FixedPoint.h"
#include "FxPtTests.h"
#include <iostream>



int main()
{
    //run8BitFixedPointTests(std::cout);
    //run16BitFixedPointTests(std::cout);


    Fixed<14, 18, UNSIGNED> x = 541.515625, y = 23.375;
    std::cout << "x = " << x << ", y = " << y << std::endl;
    std::cout << "x + y = " << x + y << std::endl;
    std::cout << "x - y = " << x - y << std::endl;
    std::cout << "x * y = " << x * y << std::endl;
    std::cout << "x / y = " << x / y << std::endl;

    Fixed<30, 33> a = -12352.5, b = 3125.533203125;
    std::cout << "a = " << a << ", b = " << b << std::endl;
    std::cout << "a + b = " << a + b << std::endl;
    std::cout << "a - b = " << a - b << std::endl;
    std::cout << "a * b = " << a * b << std::endl;
    std::cout << "a / b = " << a / b << std::endl;

    return 0;
}
