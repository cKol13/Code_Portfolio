#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#define SIGNED true
#define UNSIGNED false

#include <cassert>
#include <bitset>
#include <inttypes.h>

/*
// Can set typedef's integer widths up to (u)int32_t, 64-bit version creates
// ambiguity in constructors. Do not make fast8's bigger than fast16's etc
// otherwise the code will not compile... fast8 <= fast16 <= fast32
typedef uint32_t  fastu8;
typedef uint32_t fastu16;
typedef uint32_t fastu32;
typedef uint64_t fastu64;

typedef int32_t  fast8;
typedef int32_t fast16;
typedef int32_t fast32;
typedef int64_t fast64;
typedef __int128 fast128;
*/

// Choose which data types you want to use
typedef uint8_t  fastu8;
typedef uint16_t fastu16;
typedef uint32_t fastu32;
typedef uint64_t fastu64;

typedef int8_t  fast8;
typedef int16_t fast16;
typedef int32_t fast32;
typedef int64_t fast64;
typedef __int128 fast128;

// Used for the Fit function, maximum int word length for conversions
typedef fast64 fastLargestUsed; // exclude fast128


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
static_assert(sizeof(fast32) == 4, "FixedPoint.h: fast32 needs to be 32 bits");
static_assert(sizeof(fast64) == 8, "FixedPoint.h: fast64 needs to be 64 bits");
static_assert(sizeof(fast128) == 16,
              "FixedPoint.h: fast128 needs to be 128 bits");

static_assert((sizeof(fast8) <= sizeof(fast16)) && (sizeof(fast16) <= 4),
              "FixedPoint.h: Typedefs need to be in correct order");
static_assert(sizeof(fastLargestUsed) <= 8, "fastLargestUsed size too large");



/*
    The goal is to use the smallest data type possible to try to speed
    up calculations, reduce the amount of storage needed, and/or to increase
    portability between systems.

    Fixed variables can be either signed or unsigned. The sum of INT and
    FRAC represent the number of data bits. Each Fixed number needs at least
    1 INT bit or 1 FRAC bit.

    The user will need to choose numbers such that the data bit count adds up to
    either 7, 15, 31, or 63 for signed numbers, or 8, 16, 32, or 64 for
    unsigned. This is done for clarity in knowing the true data size of the
    fixed point variable.
*/
template<fastu16 _INT, fastu16 _FRAC, bool _SIGN = SIGNED> class Fixed{
    typedef Fixed<_INT, _FRAC, _SIGN> fixed;

    const static fastu16 BITSUM = _INT + _FRAC;

    static_assert(_INT > 0, "FixedPoint.h: Not enough INT bits");
    static_assert(_INT < 64, "FixedPoint.h: Too many INT bits");
    static_assert(_FRAC > 0, "FixedPoint.h: Not enough FRAC bits");
    static_assert(_FRAC < 64, "FixedPoint.h: Too many INT bits");

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

    template<fastu16 _I, fastu16 _F, bool _S, typename _T = fixSize>
    Fixed<_I, _F, _S> createFixed(_T rawNumber) const{
        Fixed<_I, _F, _S> ret;
        ret.setRawNumber(rawNumber);
        return ret;
    }

    fixSize number;
    const static dfixSize MULT_ROUND = static_cast<dfixSize>(1) << (_FRAC - 1);

public:

    // **************************************************************
    //                          Constructors
    // **************************************************************
    Fixed(){
        number = 0;
    }
    Fixed(fastu32 num){
        fromInt(num);
    }
    Fixed(fast32 num){
        fromInt(num);
    }
    Fixed(double num){
        fromDouble(num);
    }
    Fixed(float num){
        fromFloat(num);
    }
    ~Fixed(){}

    // Equals Operators
    fixed& operator=(fastu32 num){
        fromInt(num);
        return *this;
    }
    fixed& operator=(fast32 num){
        fromInt(num);
        return *this;
    }
    fixed& operator=(float num){
        fromFloat(num);
        return *this;
    }
    fixed& operator=(double num){
        fromDouble(num);
        return *this;
    }

    // **************************************************************
    //                    Convert From Functions
    // **************************************************************
    void fromFloat(float num){
        number = num * (static_cast<fixSize>(1) << _FRAC);
    }

    void fromDouble(double num){
        number = num * (static_cast<fixSize>(1) << _FRAC);
    }

    void fromInt(fixSize num){
        number = num << _FRAC;
    }

    // **************************************************************
    //                      Convert To Functions
    // **************************************************************
    float toFloat() const{
        return number / static_cast<float>(1 << _FRAC);
    }

    double toDouble() const{
        return number / static_cast<double>(1 << _FRAC);
    }

    fixSize toInt() const {
        return number >> _FRAC;
    }

    std::string toString(fast8 precision = -1) const{
        static const ufixSize mask = (~static_cast<ufixSize>(0)) << _FRAC;

        const bool isNegative = (number < 0);
        const ufixSize uNum = (isNegative) ? -number : number;
        const dfixSize tempFracBits = _FRAC;
        const ufixSize intPart = (uNum & mask) >> tempFracBits;

        // double size to prevent overflow, fracPart = 0b00011111 would overflow
        // if FRAC was 5, for example
        dfixSize fracPart = uNum & (~mask);

        std::string fracMsg = ".";

        // If precision is 0, round to nearest integer
        if(precision == 0) return std::to_string(round().toInt()) + ".0";

        if(tempFracBits > 0){
            do{ // Convert fractional part to string
                precision--;
                fracPart *= 10;
                fast8 digit = '0' + (fracPart >> tempFracBits);

                if(precision == 0){
                    fracPart &= (static_cast<dfixSize>(1) << tempFracBits) - 1;
                    fracPart *= 10;
                    if(('0' + (fracPart >> tempFracBits)) >= '5') digit++;
                    fracMsg += digit;
                    break;
                }

                fracMsg += digit;
                fracPart &= (static_cast<dfixSize>(1) << tempFracBits) - 1;

            }while(fracPart > 0);

            while(precision > 0){ // Append 0's until full precision reached
                fracMsg += '0';
                precision--;
            }
        }

        return ((isNegative) ? "-" : "") + std::to_string(intPart) + fracMsg;
    }

    std::string toBinary() const{
        return std::bitset<sizeof(fixSize) * 8>(number).to_string();
    }

    // **************************************************************
    //                 Number Information Functions
    // **************************************************************
    fixed getMaxValue() const{
        static const ufixSize maxVal =
                    (static_cast<ufixSize>(1) << (BITSUM - 1)) - 1;

        return createFixed<_INT, _FRAC, _SIGN>(maxVal);
    }

    fixed getMinValue() const{
        static const fixSize minVal =
                    -(static_cast<fixSize>(1) << (BITSUM - 1));

        if(_SIGN == SIGNED)
            return createFixed<_INT, _FRAC, _SIGN>(minVal);

        return 0; // Min of unsigned is 0
    }

    fixed getResolution() const{
        return createFixed<_INT, _FRAC, _SIGN>(1);
    }

    Fixed<_INT-1, _FRAC+1, _SIGN> getPrecision() const{
        return createFixed<_INT-1, _FRAC+1, _SIGN>(1);
    }

    fixed getFraction() const{
        static const fixSize mask = ~(~(static_cast<fixSize>(0)) << _FRAC);
        return createFixed<_INT, _FRAC, _SIGN>((number < 0) ?
                                    -((-number) & mask) : (number & mask));
    }

    fixed floor() const{
        return createFixed<_INT, _FRAC, _SIGN>((number >> _FRAC) << _FRAC);
    }

    fixed ceil() const{
        static const fixSize mask = ~((~static_cast<fixSize>(0)) << _FRAC);
        // If any frac bits, return floor + 1
        if(number & mask)
            return createFixed<_INT, _FRAC, _SIGN>(
                                        ((number >> _FRAC) + 1) << _FRAC);
        return
            *this;
    }

    fixed round() const{
        static const fixSize mask = static_cast<fixSize>(1) << (_FRAC - 1);
        // Check if fraction is >= 0.5
        if(number & mask)
            return ceil();
        else
            return floor();
    }

    fixed abs() const{
        return createFixed<_INT, _FRAC, _SIGN>((number < 0) ? -number : number);
    }

    // Fitting Operation
    template<fastu16 newINT, fastu16 newFRAC, bool newSIGN = SIGNED>
    Fixed<newINT, newFRAC, newSIGN> fit() const{
        // Use largest used data type for most efficient use, prevents data loss
        // 8 <--> 64 bit conversions
        static const fastLargestUsed shift = (newFRAC >= _FRAC) ?
                                    (newFRAC - _FRAC) : (_FRAC - newFRAC);
        const fastLargestUsed temp = number;

        return createFixed<newINT, newFRAC, newSIGN, fastLargestUsed>(
                    (newFRAC >= _FRAC) ? (temp << shift) : (temp >> shift));
    }

    // Additional number information
    void setRawNumber(fixSize num){ number = num;}
    fixSize getRawNumber() const{ return number;}

    fastu16 getFracBits() const{ return _FRAC;}
    fast16 getIntBits() const{ return _INT;}

    bool isZero() const{ return number == 0;}
    bool isNonZero() const{ return number != 0;}
    bool isNegative() const{ return number < 0;}
    bool isPositive() const{ return number > 0;}

    // Stream Output
    friend std::ostream& operator<<(std::ostream &out, const fixed fp){
        out << fp.toString();
        return out;
    }

    // **************************************************************
    //                        Math Operators
    // **************************************************************

    // Negation
    fixed operator-() const{
        return createFixed<_INT, _FRAC, _SIGN>(-number);
    }

    // Addition
    friend fixed operator+(fixed L, const fixed R){
        L.number += R.number;
        return L;
    }
    void operator+=(const fixed R){
        number += R.number;
    }
    void increment(){
        number += static_cast<fixSize>(1) << _FRAC;
    }

    // Subtraction
    friend fixed operator-(fixed L, const fixed R){
        L.number -= R.number;
        return L;
    }
    void operator-=(const fixed R){
        number -= R.number;
    }
    void decrement(){
        number -= static_cast<fixSize>(1) << _FRAC;
    }

    // Multiplication
    friend fixed operator*(fixed L, const fixed R){
        L.number = ((static_cast<dfixSize>(L.number) *
                     static_cast<dfixSize>(R.number)) + MULT_ROUND) >> _FRAC;
        return L;
    }
    void operator*=(const fixed R){
        number = ((static_cast<dfixSize>(number) *
                   static_cast<dfixSize>(R.number)) + MULT_ROUND) >> _FRAC;
    }

    // Division
    friend fixed operator/(fixed L, const fixed R){
        assert(R.number != 0); // Exit or set to max val?
        //if(R.number == 0) return L.getMaxValue();

        L.number = (static_cast<dfixSize>(L.number) << _FRAC) / R.number;
        return L;
    }
    void operator/=(const fixed R){
        assert(R.number != 0);
        //if(R.number == 0)
            //number = getMaxValue().getRawNumber();
        //else
            number = (static_cast<dfixSize>(number) << _FRAC) / R.number;
    }

    // **************************************************************
    //                      Comparison Operators
    // **************************************************************
    friend bool operator>(const fixed L, const fixed R){
        return L.number > R.number;
    }
    friend bool operator<(const fixed L, const fixed R){
        return L.number < R.number;
    }
    friend bool operator>=(const fixed L, const fixed R){
        return L.number >= R.number;
    }
    friend bool operator<=(const fixed L, const fixed R){
        return L.number <= R.number;
    }
    friend bool operator!=(const fixed L, const fixed R){
        return L.number != R.number;
    }
    friend bool operator==(const fixed L, const fixed R){
        return L.number == R.number;
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
