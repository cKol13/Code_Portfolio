/*
 *  @file    FixedPoint.h
 *  @author  Chris Kolegraff
 *  @date    11/18/2017
 *
 *  @brief A template fixed point library
 *
 *  DESCRIPTION
 *
 * The point behind this fixed point library is to allow the user to use
 * the smallest data type possible to try to speed up calculations, reduce
 * the amount of storage needed, increase readability in the code to prevent
 * bugs, and/or to increase portability between systems.
 *
 * The sum of INT and FRAC represent the number of data bits. Each Fixed
 * number needs at least 1 INT bit or 1 FRAC bit (for now). This class will
 * automatically choose the smallest data type depending on what numbers the
 * user chooses. A Fixed<3, 4> will use an 8 bit integer, while a Fixed<32, 31>
 * will use a 64 bit integer.
 *
 * Fixed point numbers can be signed or unsigned. You can set the sign value
 * by either using true for signed and false for unsigned, or use the
 * SIGNED and UNSIGNED defines when declaring a fixed variable.
 * Fixed numbers will default to signed.

 * The user will need to choose numbers such that the data bits add up to
 * either 7, 15, 31, or 63 for signed numbers, or 8, 16, 32, or 64 for
 * unsigned. This is done for clarity in knowing the true data size of the
 * fixed point variable.
 *
 * Fixed<5, 2, true> is an 8 bit signed value, with 2 fractional bits.
 * Fixed<10, 5> is a 16 bit signed value, with 5 fractional bits.
 * Fixed<16, 15, SIGNED> is a 32 bit signed value, with 15 fractional bits.
 * Fixed<20, 44, UNSIGNED> is a 64 bit unsigned value, with 44 fractional bits.
 *
 *
 *  MODIFICATIONS
 *
 * There are some defines that the user can toggle to enable/disable support
 * for larger data types. If your system does not support 32 or 64 bit numbers,
 * you can set ENABLE_32_BIT_USE or ENABLE_64_BIT_USE to 0 or 1. 1 will enable
 * and 0 will disable use for that data size. If ENABLE_64_BIT_USE is set to 1,
 * and ENABLE_32_BIT_USE is set to 0, the library will still correctly
 * compile and use 32 bit Fixed variables.
 *
 * If you are running on a system which does not have support for double the
 * size of the largest supported data type (e.g. 128 bit data types on a 64 bit
 * system) then you can toggle ENABLE_DW_BIT_MATH to 1/0. 1 will enable the
 * library to cast values to the doubled width, and 0 will disable that
 * functionality. If ENABLE_DW_BIT_MATH is 0, then the library will use slower
 * algorithms that do not cast the variables used to their doubled sizes. This
 * only applies to the largest supported data width.
 *
 * Enable 64-bit  Enable 32-bit  Enable DW Math  Max size  Max used for Math
 *        0              0              0           16            16
 *        0              0              1           16            32
 *        0              1              0           32            32
 *        0              1              1           32            64
 *        1              X              0           64            64
 *        1              X              1           64            128
 *
 *
 *  KNOWN BUGS
 *
 * 1. toString() does not function properly if variables with the
 * majority of bits being fractional bits (Fixed<2, 30, UNSIGNED> for instance),
 * the variable fracPart will overflow after being multiplied by 10 if the
 * variable is not able to use the next double width int type.
 *      ENABLE_32_BIT_USE = 0 and ENABLE_64_BIT_USE = 0, 16-bit tests fail.
 *            "Fixed16_Test_toString_testB1: test FAILED.
 *              Expected: 6.50390625, result was: 6.503)0625."
 *            "Fixed16_Test_toString_testF1: test FAILED.
 *              Expected: -0.9998779296875, result was: -0.1110//1.1.0/1." etc
 *      Behavior stops when ENABLE_32_BIT_USE or ENABLE_64_BIT_USE is set to 1
 */




#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#define CAST static_cast
#define TO_DFIX CAST<dfixSize>
#define TO_UFIX CAST<ufixSize>
#define TO_SFIX CAST<fixSize>

#define SIGNED true
#define UNSIGNED false

#define ENABLE_32_BIT_USE 1 // 1 for enable, 0 for disable
#define ENABLE_64_BIT_USE 1 // If 1, will override 32 bit enable define effects

// 1 if compiler supports double max width bit integers, 0 otherwise
// If 0, multiplication and division will use the slower
// algorithms, but will not use doubled integer width variables.
#define ENABLE_DW_BIT_MATH 1

#include <cassert>
#include <cstdint>
#include <string>
#include <ostream>
#include <type_traits>

typedef std::uint8_t  fastu8;
typedef std::int8_t  fast8;

typedef std::uint16_t fastu16;
typedef std::int16_t fast16;


#if ENABLE_64_BIT_USE == 1
    typedef std::uint32_t fastu32;
    typedef std::int32_t fast32;
    typedef std::int64_t fast64;
    typedef std::uint64_t fastu64;
    #define BIT_THRESHOLD 62
#elif ENABLE_32_BIT_USE == 1
    typedef std::uint32_t fastu32;
    typedef std::int32_t fast32;
    typedef std::int32_t fast64;
    typedef std::uint32_t fastu64;
    #define BIT_THRESHOLD 30
#else
    typedef std::uint16_t fastu32;
    typedef std::int16_t fast32;
    typedef std::int16_t fast64;
    typedef std::uint16_t fastu64;
    #define BIT_THRESHOLD 14
#endif // ENABLE_64_BIT_USE


#if ENABLE_DW_BIT_MATH == 1
    #if ENABLE_64_BIT_USE == 1
        typedef __int128 fast128;
    #elif ENABLE_32_BIT_USE == 1
        typedef int64_t fast128;
    #else
        typedef int32_t fast128;
    #endif // ENABLE_32_BIT_USE
#else
    typedef fast64 fast128;
#endif // ENABLE_DW_BIT_MATH



static_assert(ENABLE_32_BIT_USE == 0 || ENABLE_32_BIT_USE == 1,
              "Invalid value for ENABLE_32_BIT_USE");
static_assert(ENABLE_64_BIT_USE == 0 || ENABLE_64_BIT_USE == 1,
              "Invalid value for ENABLE_64_BIT_USE");
static_assert(ENABLE_DW_BIT_MATH == 0 || ENABLE_DW_BIT_MATH == 1,
              "Invalid value for ENABLE_DW_BIT_MATH");


static_assert(sizeof(fast8) == sizeof(fastu8),
              "FixedPoint.h: fast8 and fastu8 not the same size");
static_assert(sizeof(fast16) == sizeof(fastu16),
              "FixedPoint.h: fast16 and fastu16 not the same size");
static_assert(sizeof(fast32) == sizeof(fastu32),
              "FixedPoint.h: fast32 and fastu32 not the same size");
static_assert(sizeof(fast64) == sizeof(fastu64),
              "FixedPoint.h: fast64 and fastu64 not the same size");

static_assert(sizeof(fast8) >= 1, "FixedPoint.h: fast8 size too small");
static_assert(sizeof(fast16) >= 2, "FixedPoint.h: fast16 size too small");

#if ENABLE_32_BIT_USE == 1
    static_assert(sizeof(fast32) == 4,
                  "FixedPoint.h: fast32 needs to be 32 bits");
#endif // ENABLE_32_BIT_USE

#if ENABLE_64_BIT_USE == 1
    static_assert(sizeof(fast64) == 8,
                  "FixedPoint.h: fast64 needs to be 64 bits");
#endif // ENABLE_64_BIT_USE

#if ENABLE_64_BIT_USE == 1
    #if ENABLE_DW_BIT_MATH == 1
        static_assert(sizeof(fast128) == 16,
              "FixedPoint.h: fast128 needs to be 128 bits");
    #endif // ENABLE_DW_BIT_MATH
#endif // ENABLE_64_BIT_USE

static_assert((sizeof(fast8) <= sizeof(fast16)) && (sizeof(fast16) <= 4),
              "FixedPoint.h: Typedefs need to be in correct order");



template<fastu16 _INT, fastu16 _FRAC, bool _SIGN = SIGNED> class Fixed{

    static_assert(_INT > 0, "FixedPoint.h: Not enough INT bits");
    static_assert(_INT < 64, "FixedPoint.h: Too many INT bits");
    static_assert(_FRAC > 0, "FixedPoint.h: Not enough FRAC bits");
    static_assert(_FRAC < 64, "FixedPoint.h: Too many INT bits");

    const static fastu16 BITSUM = _INT + _FRAC;

    static_assert(
    ( _SIGN && (BITSUM == 7 || BITSUM == 15 || BITSUM == 31 || BITSUM == 63)) ||
    (!_SIGN && (BITSUM == 8 || BITSUM == 16 || BITSUM == 32 || BITSUM == 64)),
                "FixedPoint.h: Sum of bits needs to equal "
                "either SIGNED:{7, 15, 31, 63} or UNSIGNED:{8, 16, 32, 64}");


    using fixSize = // Use smallest int data type and choose sign
        typename std::conditional<_SIGN,
            // Signed type names
            typename std::conditional<BITSUM == 7,
                fast8,
                typename std::conditional<BITSUM == 15,
                    fast16,
                    typename std::conditional<BITSUM == 31,
                        fast32,
                        fast64
                    >::type
                >::type
            >::type,
            // Unsigned type names
            typename std::conditional<BITSUM == 8,
                fastu8,
                typename std::conditional<BITSUM == 16,
                    fastu16,
                    typename std::conditional<BITSUM == 32,
                        fastu32,
                        fastu64
                    >::type
                >::type
            >::type
        >::type;

    using ufixSize = // Unsigned Fixed Size
        typename std::conditional<BITSUM == 7 || BITSUM == 8,
            fastu8,
            typename std::conditional<BITSUM == 15 || BITSUM == 16,
                fastu16,
                typename std::conditional<BITSUM == 31 || BITSUM == 32,
                    fastu32,
                    fastu64
                >::type
            >::type
        >::type;

    using dfixSize = // Double Fixed Size
        typename std::conditional<BITSUM == 7 || BITSUM == 8,
            fast16,
            typename std::conditional<BITSUM == 15 || BITSUM == 16,
                fast32,
                typename std::conditional<BITSUM == 31 || BITSUM == 32,
                    fast64,
                    fast128
                >::type
            >::type
        >::type;


    fixSize number;
    const static dfixSize MULT_ROUND = TO_DFIX(1) << (_FRAC - 1);

    // Private constructor to set raw number directly
    Fixed(fixSize num, bool): number(num){}

public:

    // **************************************************************
    //                          Constructors
    // **************************************************************
    Fixed():                 number(0){}
    Fixed(int num):          number(TO_SFIX(num) << _FRAC){}
    Fixed(double num):       number(num * (TO_SFIX(1) << _FRAC)){}
    Fixed(float num):        number(num * (TO_SFIX(1) << _FRAC)){}
    Fixed(const Fixed &num): number(num.number){}
    ~Fixed(){}

    const Fixed& operator=(const Fixed &num){
        number = num.number;
        return *this;
    }

    // **************************************************************
    //                    Convert From Functions
    // **************************************************************
    void fromFloat(float num){
        number = num * (TO_SFIX(1) << _FRAC);
    }
    void fromDouble(double num){
        number = num * (TO_SFIX(1) << _FRAC);
    }
    void fromInt(fixSize num){
        number = num << _FRAC;
    }

    // **************************************************************
    //                      Convert To Functions
    // **************************************************************
    float toFloat() const{
        return number / CAST<float>(1 << _FRAC);
    }

    double toDouble() const{
        return number / CAST<double>(1 << _FRAC);
    }

    fixSize toInt() const {
        return number >> _FRAC;
    }

    std::string toString(fast16 precision = -1) const{
        static const ufixSize mask = (~TO_UFIX(0)) << _FRAC;

        const bool isNegative = (number < 0);
        const ufixSize uNum = (isNegative) ? -number : number;
        const fixSize tempFracBits = _FRAC;
        ufixSize intPart = (uNum & mask) >> tempFracBits;
        bool allNines = true;

        // Double size used to prevent overflow
        dfixSize fracPart = uNum & (~mask);

        std::string fracMsg = ".";

        // If precision is 0, round to nearest integer
        if(precision == 0) return std::to_string(round().toInt()) + ".0";

        do{ // Convert fractional part to string
            precision--;
            fracPart *= 10;
            fast8 digit = '0' + (fracPart >> tempFracBits);
            if(allNines && digit != '9') allNines = false;

            if(precision == 0){
                fracPart &= (TO_DFIX(1) << tempFracBits) - 1;
                fracPart *= 10;
                if(('0' + (fracPart >> tempFracBits)) >= '5'){
                    if(allNines){
                        fracMsg = ".0";
                        intPart += 1;
                        break;
                    }
                    digit++;
                }
                fracMsg += digit;
                break;
            }

            fracMsg += digit;
            fracPart &= (TO_DFIX(1) << tempFracBits) - 1;

        }while(fracPart > 0);

        while(precision > 0){ // Append 0's until full precision reached
            fracMsg += '0';
            precision--;
        }


        return ((isNegative) ? "-" : "") + std::to_string(intPart) + fracMsg;
    }

    std::string toBinary() const{
        std::string ret = "";
        ufixSize mask = (TO_UFIX(1) << (BITSUM - ((_SIGN) ? 0 : 1)));
        while(mask){
            ret += (mask & number) ? '1' : '0';
            mask >>= 1;
        }
        return ret;
    }

    // **************************************************************
    //                 Number Information Functions
    // **************************************************************
    Fixed getMaxValue() const{
        static const ufixSize maxVal = (TO_UFIX(1) << BITSUM) - 1;
        static const ufixSize uMaxVal = TO_UFIX(maxVal << 1) | 1;

        if(_SIGN) return Fixed(maxVal, true);
        else      return Fixed(uMaxVal, true);
    }

    Fixed getMinValue() const{
        static const fixSize minVal = TO_UFIX(1) << (BITSUM - ((_SIGN) ? 0:1));

        if(_SIGN) return Fixed(minVal, true);
        else      return Fixed(0, true); // Min of unsigned is 0
    }

    Fixed getResolution() const{
        return Fixed(1, true);
    }

    Fixed<_INT-1, _FRAC+1, _SIGN> getPrecision() const{
        Fixed<_INT-1, _FRAC+1, _SIGN> ret;
        ret.setRawNumber(1);
        return ret;
    }

    Fixed getFraction() const{
        static const fixSize mask = ~(~TO_SFIX(0) << _FRAC);
        return Fixed((number < 0) ? -((-number) & mask) :
                                       (number & mask), true);
    }

    Fixed abs() const{
        return Fixed((number < 0) ? -number : number, true);
    }

    // **************************************************************
    //                 Number Rounding Functions
    // **************************************************************
    Fixed floor() const{
        return Fixed((number >> _FRAC) << _FRAC, true);
    }

    Fixed ceil() const{
        static const ufixSize mask = ~((~TO_UFIX(0)) << _FRAC);
        // If any frac bits, return floor + 1
        if(number & mask)
            return Fixed(((number >> _FRAC) + 1) << _FRAC, true);
        return
            Fixed(number, true);
    }

    Fixed round() const{
        static const fixSize mask = TO_SFIX(1) << (_FRAC - 1);
        // Check if fraction is >= 0.5
        if(number & mask) return ceil();
        else              return floor();

    }

    // Fitting Operation
    template<fastu16 newINT, fastu16 newFRAC, bool newSIGN = SIGNED>
    Fixed<newINT, newFRAC, newSIGN> fit() const{
        // Use largest used data type for most efficient use, prevents data loss
        // 8 <--> 64 bit conversions
        static const fast64 shift = (newFRAC >= _FRAC) ?
                                    (newFRAC - _FRAC): (_FRAC - newFRAC);
        const fast64 temp = number;

        Fixed<newINT, newFRAC, newSIGN> ret;
        ret.setRawNumber((newFRAC >= _FRAC) ? (temp << shift): (temp >> shift));

        return ret;
    }

    // Additional number information
    void setRawNumber(fixSize num){ number = num;}
    fixSize getRawNumber() const{ return number;}

    bool isZero() const{ return number == 0;}
    bool isNonZero() const{ return number != 0;}
    bool isNegative() const{ return number < 0;}
    bool isPositive() const{ return number > 0;}

    // Stream Output
    friend std::ostream& operator<<(std::ostream &out, const Fixed &fp){
        out << fp.toString();
        return out;
    }

    // **************************************************************
    //                        Math Operators
    // **************************************************************

    // Negation
    Fixed operator-() const{
        return Fixed(-number, true);
    }

    // Addition
    friend Fixed operator+(const Fixed &L, const Fixed &R){
        return Fixed(L.number + R.number, true);
    }
    void operator+=(const Fixed &R){
        number += R.number;
    }
    void increment(){
        number += TO_SFIX(1) << _FRAC;
    }

    // Subtraction
    friend Fixed operator-(const Fixed &L, const Fixed &R){
        return Fixed(L.number - R.number, true);
    }
    void operator-=(const Fixed &R){
        number -= R.number;
    }
    void decrement(){
        number -= TO_SFIX(1) << _FRAC;
    }

    // Multiplication
    friend Fixed operator*(const Fixed &L, const Fixed &R){
        if(ENABLE_DW_BIT_MATH == 0 && (BITSUM > BIT_THRESHOLD))
            return L.longMultiply(R);

        return Fixed(((TO_DFIX(L.number) *
                       TO_DFIX(R.number)) + MULT_ROUND) >> _FRAC, true);
    }
    void operator*=(const Fixed &R){
        if(ENABLE_DW_BIT_MATH == 0 && (BITSUM > BIT_THRESHOLD))
            number = longMultiply(R).number;
        else
            number = ((TO_DFIX(number) * TO_DFIX(R.number))
                        + MULT_ROUND) >> _FRAC;
    }

    // Division
    friend Fixed operator/(const Fixed &L, const Fixed &R){
        assert(R.number != 0); // Exit or set to max val?
        //if(R.number == 0) return L.getMaxValue();

        if(ENABLE_DW_BIT_MATH == 0 && (BITSUM > BIT_THRESHOLD))
            return L.longDivide(R);

        return Fixed((TO_DFIX(L.number) << _FRAC) / R.number, true);
    }
    void operator/=(const Fixed &R){
        assert(R.number != 0);
        //if(R.number == 0)
            //number = getMaxValue().getRawNumber();
        if(ENABLE_DW_BIT_MATH == 0 && (BITSUM > BIT_THRESHOLD))
            number = longDivide(R).number;
        else
            number = (TO_DFIX(number) << _FRAC) / R.number;
    }

    // Left Shift
    Fixed operator<<(const fast16 shift){
        return Fixed(number << shift, true);
    }
    void operator<<=(const fast16 shift){
        number <<= shift;
    }

    // Right Shift
    Fixed operator>>(const fast16 shift){
        return Fixed(number >> shift, true);
    }
    void operator>>=(const fast16 shift){
        number >>= shift;
    }

    // **************************************************************
    //                      Comparison Operators
    // **************************************************************
    friend bool operator>(const Fixed &L, const Fixed &R){
        return L.number > R.number;
    }
    friend bool operator<(const Fixed &L, const Fixed &R){
        return L.number < R.number;
    }
    friend bool operator>=(const Fixed &L, const Fixed &R){
        return L.number >= R.number;
    }
    friend bool operator<=(const Fixed &L, const Fixed &R){
        return L.number <= R.number;
    }
    friend bool operator!=(const Fixed &L, const Fixed &R){
        return L.number != R.number;
    }
    friend bool operator==(const Fixed &L, const Fixed &R){
        return L.number == R.number;
    }

private:
    //  https://stackoverflow.com/questions/79677/
    //      whats-the-best-way-to-do-fixed-point-math
    //  Based on Evan Teran's multiply implementation
    Fixed longMultiply(const Fixed &R) const{
        static const fast64 mask = (TO_SFIX(1) << _FRAC) - 1;

        const fast64 LHi = (number & (~mask)) >> _FRAC;
        const fast64 RHi = (R.number & (~mask)) >> _FRAC;
        const fast64 LLo = number & mask;
        const fast64 RLo = R.number & mask;

        const fast64 x1 = LHi * RHi;
        const fast64 x2 = LHi * RLo;
        const fast64 x3 = LLo * RHi;
        const fast64 x4 = LLo * RLo + MULT_ROUND;

        return Fixed((x1 << _FRAC) + x2 + x3 + (x4 >> _FRAC), true);
    }

    //  https://codereview.stackexchange.com/questions/
    //      67962/mostly-portable-128-by-64-bit-division
    //  Based on joe63074's division implementation, with some modifications
    Fixed longDivide(const Fixed &R) const{
        bool LNegative = false, RNegative = false;
        fastu64 left = number, right = R.number;
        if(number < 0){
            left = -number;
            LNegative = true;
        }
        if(R.number < 0){
            right = -R.number;
            RNegative = true;
        }

        const fastu64 ALo = left << _FRAC;
        const fastu64 AHi = left >> (BITSUM - _FRAC);

        fast64 ret = 0;

        fastu64 quotient = ALo << 1;
        fastu64 remainder = AHi;
        fastu64 carry = ALo >> (BITSUM - 1);
        fastu64 tempCarry = 0;

        for(fastu16 i = 0; i < BITSUM; i++){
            tempCarry = remainder >> (BITSUM - 1);
            remainder <<= 1;
            remainder |= carry;
            carry = tempCarry;

            if(carry == 0){
                if(remainder >= right){
                    carry = 1;
                }
                else{
                    tempCarry = quotient >> (BITSUM - 1);
                    quotient <<= 1;
                    quotient |= carry;
                    carry = tempCarry;
                    continue;
                }
            }

            remainder -= right;
            remainder -= (1 - carry);
            carry = 1;
            tempCarry = quotient >> (BITSUM - 1);
            quotient <<= 1;
            quotient |= carry;
            carry = tempCarry;
        }

        if(LNegative ^ RNegative) ret = -quotient;
        else ret = quotient;

        return Fixed(ret, true);
    }

};

// Test to make sure Fixed auto sizing is working
static_assert(sizeof(Fixed<4, 3>) == sizeof(fast8),
        "FixedPoint.h: Fixed<4, 3> is not the same size as fast8");
static_assert(sizeof(Fixed<5, 10>) == sizeof(fast16),
        "FixedPoint.h: Fixed<5, 10> is not the same size as fast16");
static_assert(sizeof(Fixed<11, 20>) == sizeof(fast32),
        "FixedPoint.h: Fixed<11, 20> is not the same size as fast32");
static_assert(sizeof(Fixed<33, 30>) == sizeof(fast64),
        "FixedPoint.h: Fixed<33, 30> is not the same size as fast64");

static_assert(sizeof(Fixed<4, 4, false>) == sizeof(fast8),
        "FixedPoint.h: Fixed<4, 4, false> is not the same size as fast8");
static_assert(sizeof(Fixed<6, 10, false>) == sizeof(fast16),
        "FixedPoint.h: Fixed<6, 10, false> is not the same size as fast16");
static_assert(sizeof(Fixed<12, 20, false>) == sizeof(fast32),
        "FixedPoint.h: Fixed<12, 20, false> is not the same size as fast32");
static_assert(sizeof(Fixed<34, 30, false>) == sizeof(fast64),
        "FixedPoint.h: Fixed<34, 30, false> is not the same size as fast64");


#endif // FIXEDPOINT_H
