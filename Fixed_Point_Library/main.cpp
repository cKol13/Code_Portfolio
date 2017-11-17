#include "FixedPoint.h"
#include <iostream>


using namespace std;

int main()
{
    Fixed<10, 5> a = 123.23, b = 32, sum = a + b;
    cout << a << " + " << b << " = " << sum.toString(5) << endl;

    Fixed<16, 16, UNSIGNED> c = 13.875, d = 32.25, product = c * d;
    cout << c << " * " << d << " = " << product.toString(5) << endl;

    return 0;
}
