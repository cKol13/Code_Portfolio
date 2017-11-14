#include "FixedPoint.h"
#include <iostream>

#include <math.h>
#include <cmath>

using namespace std;

int main()
{
    Fixed<10, 10> a(2.5);
    cout << a << endl;
    return 0;
}
