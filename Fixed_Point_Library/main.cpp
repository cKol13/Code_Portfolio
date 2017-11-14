#include "FixedMath.h"
#include <iostream>

#include <math.h>
#include <cmath>

using namespace std;

int main()
{
    Fixed<10, 10> a(2.5);
    Fixed<10, 10> b(3.75);
    cout << a + 3.0 << endl;
    cout << 3 + a << endl;
    cout << a + b + 3.0 << endl;

    //cout << "exp(1.4) = " << Fixed_exp<30, 30>(1.4) << endl;
    return 0;
}
