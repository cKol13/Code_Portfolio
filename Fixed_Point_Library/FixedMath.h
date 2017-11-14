#ifndef FIXEDMATH_H
#define FIXEDMATH_H

#include "FixedPoint.h"
#include <iostream>

const static Fixed<2, 61> FIXED_PI(3.14159265358979323846264);
const static Fixed<2, 61> FIXED_PIDIV2 = FIXED_PI / 2;

// Taylor Series expansion of e^x method
template<fastu16 INT, fastu16 FRAC>
Fixed<INT, FRAC> Fixed_exp(Fixed<INT, FRAC> x, fastu16 precision = 0){
    Fixed<INT, FRAC> sum(1 + x), iteration(x);
    fastu16 n = 2;

    if(precision > 0){
        for(; n < precision && iteration.isNonZero(); n++){
            iteration *= x / n;
            sum += iteration;
        }
    }
    else{
        for(; n < 15 && iteration.isNonZero(); n++){
            iteration *= x / n;
            sum += iteration;
        }
    }
    return sum;
}
/*
// http://www.claysturner.com/dsp/BinaryLogarithm.pdf
template<fastu16 INT, fastu16 FRAC>
Fixed<INT, FRAC> Fixed_Log2(Fixed<INT, FRAC> x, fastu16 precision = 0){
    Fixed<INT, FRAC> y(0), b(0.5);

    while(x < 1){
        x *= 2;
        y -= 1;
    }

    while(x >= 2){
        x /= 2;
        y += 1;
    }

    if(precision > 0){
        if(precision > FRAC) precision = FRAC;
        for(fastu16 i = 0; i < precision; i++){
            x = x * x;
            if(x >= 2){
                x /= 2;
                y += b;
            }
            b /= 2;
        }
    }
    else{
        for(fastu16 i = 0; i < FRAC; i++){
            x = x * x;
            if(x >= 2){
                x /= 2;
                y += b;
            }
            b /= 2;
        }
    }

    return y;
}

// http://lab.polygonal.de/2007/07/18/
//    fast-and-accurate-sinecosine-approximation/
// Quadratic approximation of a sine wave
// Min/Max Err: +-0.0010946, Avg Err: 0.00050539 rads
template<fastu16 INT, fastu16 FRAC>
Fixed<INT, FRAC> Fixed_sin(Fixed<INT, FRAC> x){
    const static Fixed<INT, FRAC>
        FIX_PI(FIXED_PI.fit<INT, FRAC>()),
        FIX_2PI(2 * FIX_PI),
        CONST1(1.27323954),
        CONST2(0.405284735),
        CONST3(0.225),
        ONE(1);

    while(x < -FIX_PI) x += FIX_2PI;
    while(x > FIX_PI)  x -= FIX_2PI;

    if(x.isNegative()){
        x *= CONST1 + (CONST2 * x);
        x *= (-CONST3 * (x + ONE)) + ONE;
    }
    else{
        x *= CONST1 - (CONST2 * x);
        x *= (CONST3 * (x - ONE)) + ONE;
    }

    return x;
}

template<fastu16 INT, fastu16 FRAC>
Fixed<INT, FRAC> Fixed_cos(const Fixed<INT, FRAC> &x){
    const static Fixed<INT, FRAC> FIX_PIDIV2 = FIXED_PIDIV2.fit<INT, FRAC>();
    return Fixed_sin<INT, FRAC>(x + FIX_PIDIV2);
}

template<fastu16 INT, fastu16 FRAC>
Fixed<INT, FRAC> Fixed_tan(const Fixed<INT, FRAC> &x){
    const static Fixed<INT, FRAC> FIX_PIDIV2 = FIXED_PIDIV2.fit<INT, FRAC>();
    Fixed<INT, FRAC> cosVal = Fixed_sin(x + FIX_PIDIV2);
    if(cosVal.isZero()) return cosVal.getMaxValue(); // Saturate
    return Fixed_sin<INT, FRAC>(x) / cosVal;
}

/*
    https://www.dsprelated.com/showarticle/1052.php
    Operates in range [-1, 1]
*/
/*
template<fastu16 INT, fastu16 FRAC>
Fixed<INT, FRAC> Fixed_atan(const Fixed<INT, FRAC> &x){
    static const Fixed<INT, FRAC> CONST1(0.97239411), CONST2(-0.19194795);

    return (CONST1 + (CONST2 * x * x)) * x;
}

template<fastu16 INT, fastu16 FRAC>
Fixed<INT, FRAC> Fixed_atan2(const Fixed<INT, FRAC> &y,
                             const Fixed<INT, FRAC> &x){
    const static Fixed<INT, FRAC>
        FIX_PI(FIXED_PI.fit<INT, FRAC>()),
        FIX_PIDIV2(FIXED_PIDIV2.fit<INT, FRAC>());

    Fixed<INT, FRAC> ret(0);

    if(x.isZero()){
        if(y.isPositive())      ret = FIX_PIDIV2;
        else if(y.isNegative()) ret = -FIX_PIDIV2;
        return ret;
    }

    if(x.abs() > y.abs()){
        if(x.isPositive())
            ret =  Fixed_atan<INT, FRAC>(y / x);

        else if(!y.isNegative())
            ret = Fixed_atan<INT, FRAC>(y / x) + FIX_PI;

        else
            ret = Fixed_atan<INT, FRAC>(y / x) - FIX_PI;
    }
    else{
        if(y.isPositive())
            ret = FIX_PIDIV2 - Fixed_atan<INT, FRAC>(x / y);
        else
            ret = -FIX_PIDIV2 - Fixed_atan<INT, FRAC>(x / y);
    }

    // Convert to degrees if necessary, and return
    return ret;
}
*/
#endif // FIXEDMATH_H
