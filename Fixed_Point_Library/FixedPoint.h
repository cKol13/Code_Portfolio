#ifndef FIXEDPOINT_H
#define FIXEDPOINT_H

#include <cassert>
#include <bitset>


typedef uint_fast8_t  fastu8;
typedef uint_fast16_t fastu16;
typedef uint_fast32_t fastu32;
typedef uint_fast64_t fastu64;

typedef int_fast8_t  fast8;
typedef int_fast16_t fast16;
typedef int_fast32_t fast32;
typedef int_fast64_t fast64;
typedef __int128 fast128;

// Used for the Fit function, maximum int word length for conversions
typedef fast64 fastLargestUsed;

/* // Choose which data types you want to use
typedef uint16_t  fastu8;
typedef uint16_t fastu16;
typedef uint32_t fastu32;
typedef uint64_t fastu64;

typedef int16_t  fast8;
typedef int16_t fast16;
typedef int32_t fast32;
typedef int64_t fast64;
typedef __int128 fast128;
*/

/*
    The goal is to use the smallest data type possible to try to speed
    up calculations, reduce the amount of storage needed, and/or to increase
    portability between systems (especially for systems without FPUs).

    The user/programmer will need to pick a fraction bit size, then pick an
    integer size such that 0 < INT + FRAC < 64. Fixed variables (for now)
    will always be signed. The sum of INT and FRAC represent the number
    of data bits.

    It is recommended (currently unenforced) that the bit count adds up to
    either 7, 15, 31, or 63 for clarity in knowing what the true data
    size is. A Fixed<2, 2> will still using all 7 data bits, for instance.
*/
template<fastu16 INT, fastu16 FRAC> class Fixed{

private:
    const static fastu16 BITSUM = INT + FRAC;

    static_assert(BITSUM > 0, "Not enough bits");
    static_assert(BITSUM < 64, "Too many bits");


    using fixSize = // Use smallest int data type
        typename std::conditional<(BITSUM >= 0) && (BITSUM < 8),
            fast8,
            typename std::conditional<(BITSUM >= 8) && (BITSUM < 16),
                fast16,
                typename std::conditional<(BITSUM >= 16) && (BITSUM < 32),
                    fast32,
                    fast64
                >::type
            >::type
        >::type;

    using ufixSize = // Unsigned Fixed Size
        typename std::conditional<(BITSUM >= 0) && (BITSUM < 8),
            fastu8,
            typename std::conditional<(BITSUM >= 8) && (BITSUM < 16),
                fastu16,
                typename std::conditional<(BITSUM >= 16) && (BITSUM < 32),
                    fastu32,
                    fastu64
                >::type
            >::type
        >::type;

    using dfixSize = // Double Fixed Size
        typename std::conditional<(BITSUM >= 0) && (BITSUM < 8),
            fast16,
            typename std::conditional<(BITSUM >= 8) && (BITSUM < 16),
                fast32,
                typename std::conditional<(BITSUM >= 16) && (BITSUM < 32),
                    fast64,
                    fast128
                >::type
            >::type
        >::type;

public:

    // **************************************************************
    //                          Constructors
    // **************************************************************
    Fixed(const fast8 &num){fromInt(num);}
    Fixed(const fast16 &num){fromInt(num);}
    Fixed(const fast32 &num){fromInt(num);}
    Fixed(const fast64 &num){fromInt(num);}
    Fixed(){number = 0;}
    Fixed(const Fixed<INT, FRAC> &num){number = num.number;}
    Fixed(const double &num){ fromDouble(num);}
    Fixed(const float &num){ fromFloat(num);}
    ~Fixed(){}

    // **************************************************************
    //                    Convert From Functions
    // **************************************************************

    void fromFloat(const float &num){
        number = num * (static_cast<fixSize>(1) << FRAC);
    }

    void fromDouble(const double &num){
        number = num * (static_cast<fixSize>(1) << FRAC);
    }

    void fromInt(const fixSize &num){
        number = num << FRAC;
    }

    // **************************************************************
    //                      Convert To Functions
    // **************************************************************
    float toFloat() const{
        return number / static_cast<float>(1 << FRAC);
    }

    double toDouble() const{
        return number / static_cast<double>(1 << FRAC);
    }

    fixSize toInt() const {
        return number >> FRAC;
    }

    std::string toString(fast8 precision = -1) const{
        static const ufixSize mask = (~static_cast<ufixSize>(0)) << FRAC;

        const bool isNegative = (number < 0);
        const ufixSize uNum = (isNegative) ? -number : number;
        const dfixSize tempFracBits = FRAC;
        const ufixSize intPart = uNum & mask;

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

        return ((isNegative) ? "-" : "") +
                std::to_string(intPart >> tempFracBits) + fracMsg;
    }

    std::string toBinary() const{
        return std::bitset<sizeof(fixSize) * 8>(number).to_string();
    }

    // **************************************************************
    //                 Number Information Functions
    // **************************************************************

    Fixed<INT, FRAC> getMaxValue() const{
        return createFixed<INT, FRAC>(static_cast<ufixSize>(-1) >> 1);
    }

    Fixed<INT, FRAC> getMinValue() const{
        return createFixed<INT, FRAC>(~(static_cast<ufixSize>(-1) >> 1));
    }

    Fixed<INT, FRAC> getResolution() const{
        return createFixed<INT, FRAC>(1);
    }

    Fixed<INT-1, FRAC+1> getPrecision() const{
        return createFixed<INT-1, FRAC+1>(1);
    }

    Fixed<INT, FRAC> getFraction() const{
        static const fixSize mask = ~(~(static_cast<fixSize>(0)) << FRAC);
        return createFixed<INT, FRAC>((number < 0) ? -((-number) & mask):
                                                        (number & mask));
    }

    Fixed<INT, FRAC> floor() const{
        return createFixed<INT, FRAC>((number >> FRAC) << FRAC);
    }

    Fixed<INT, FRAC> ceil() const{
        static const fixSize mask = ~((~static_cast<fixSize>(0)) << FRAC);
        // If any frac bits, return floor + 1
        if(number & mask)
            return createFixed<INT, FRAC>(((number >> FRAC) + 1) << FRAC);
        return
            *this;
    }

    Fixed<INT, FRAC> round() const{
        // Check if fraction is >= 0.5
        if(number & (static_cast<fixSize>(1) << (FRAC - 1)))
            return ceil();
        else
            return floor();
    }

    Fixed<INT, FRAC> abs() const{
        return createFixed<INT, FRAC>((number < 0) ? -number : number);
    }

    // Fitting Operation
    template<fastu16 newINT, fastu16 newFRAC>
    Fixed<newINT, newFRAC> fit() const{
        // Use largest used data type for most efficient use, prevents data loss
        const fastLargestUsed temp = number;
        const fastLargestUsed shift =(newFRAC >= FRAC) ? (newFRAC - FRAC):
                                                         (FRAC - newFRAC);

        return createFixed<newINT, newFRAC>((newFRAC >= FRAC) ?
                                        (temp << shift) : (temp >> shift));
    }

    // Additional number information
    void setRawNumber(const fixSize &num){ number = num;}
    fixSize getRawNumber() const{ return number;}

    fastu16 getFracBits() const{ return FRAC;}
    fast16 getIntBits() const{ return (8 * sizeof(fixSize)) - FRAC - 1;}

    bool isZero() const{ return number == 0;}
    bool isNonZero() const{ return number != 0;}
    bool isNegative() const{ return number < 0;}
    bool isPositive() const{ return number > 0;}

    // Equals Operators
    Fixed<INT, FRAC>&operator=(const fast8 &num){
        fromInt(num);
        return *this;
    }
    Fixed<INT, FRAC>&operator=(const fast16 &num){
        fromInt(num);
        return *this;
    }
    Fixed<INT, FRAC>&operator=(const fast32 &num){
        fromInt(num);
        return *this;
    }
    Fixed<INT, FRAC>&operator=(const fast64 &num){
        fromInt(num);
        return *this;
    }
    Fixed<INT, FRAC>&operator=(const float &num){
        fromFloat(num);
        return *this;
    }
    Fixed<INT, FRAC>&operator=(const double &num){
        fromDouble(num);
        return *this;
    }
    Fixed<INT, FRAC>&operator=(const Fixed<INT, FRAC> &num){
        number = num.number;
        return *this;
    }

    // Stream Output
    friend std::ostream& operator<<(std::ostream &out,
                                    const Fixed<INT, FRAC> &fp){
        out << fp.toString();
        return out;
    }

    // **************************************************************
    //                        Math Operators
    // **************************************************************

    // Negation
    Fixed<INT, FRAC> operator-() const{
        return createFixed<INT, FRAC>(-number);
    }

    // Addition
    template <typename Ltype, typename Rtype>
    friend Fixed<INT, FRAC> operator+(const Ltype &L, const Rtype &R){
        Fixed<INT, FRAC> left(L), right(R);
        left.number += right.number;
        return left;
    }
    template <typename Rtype> void operator+=(const Rtype &R){
        Fixed<INT, FRAC> right(R);
        number += right.number;
    }
    void increment(){
        number += static_cast<fixSize>(1) << FRAC;
    }

    // Subtraction
    template <typename Ltype, typename Rtype>
    friend Fixed<INT, FRAC> operator-(const Ltype &L, const Rtype &R){
        Fixed<INT, FRAC> left(L), right(R);
        left.number -= right.number;
        return left;
    }
    template <typename Rtype> void operator-=(const Rtype &R){
        Fixed<INT, FRAC> right(R);
        number -= right.number;
    }
    void decrement(){
        number -= static_cast<fixSize>(1) << FRAC;
    }

    // Multiplication
    template <typename Ltype, typename Rtype>
    friend Fixed<INT, FRAC> operator*(const Ltype &L, const Rtype &R){
        Fixed<INT, FRAC> left(L), right(R);
        left.number = (static_cast<dfixSize>(left.number) *
                       static_cast<dfixSize>(right.number)) >> FRAC;
        return left;
    }
    template <typename Rtype> void operator*=(const Rtype &R){
        Fixed<INT, FRAC> right(R);
        number = (static_cast<dfixSize>(number) *
                  static_cast<dfixSize>(right.number)) >> FRAC;
    }

    // Division
    template <typename Ltype, typename Rtype>
    friend Fixed<INT, FRAC> operator/(const Ltype &L, const Rtype &R){
        assert(R != 0);
        Fixed<INT, FRAC> left(L), right(R);

        left.number = (static_cast<dfixSize>(left.number) << FRAC) /
                                            right.number;
        return left;
    }
    template <typename Rtype> void operator/=(const Rtype &R){
        assert(R != 0);
        Fixed<INT, FRAC> right(R);
        number = (static_cast<dfixSize>(number) << FRAC) / right.number;
    }


    // **************************************************************
    //                      Comparison Operators
    // **************************************************************

    // Fixed compared to Any
    template <typename Rtype> bool operator>(const Rtype &R){
        return compareGT(R);
    }
    template <typename Rtype> bool operator<(const Rtype &R){
        return compareLT(R);
    }
    template <typename Rtype> bool operator>=(const Rtype &R){
        return compareGE(R);
    }
    template <typename Rtype> bool operator<=(const Rtype &R){
        return compareLE(R);
    }
    template <typename Rtype> bool operator==(const Rtype &R){
        return compareEQ(R);
    }
    template <typename Rtype> bool operator!=(const Rtype &R){
        return compareNE(R);
    }

    // Any compared to Fixed
    template <typename Ltype>
    friend bool operator>(const Ltype &L, const Fixed<INT, FRAC> &R){
        return R.compareLE(L);
    }
    template <typename Ltype>
    friend bool operator<(const Ltype &L, const Fixed<INT, FRAC> &R){
        return R.compareGE(L);
    }
    template <typename Ltype>
    friend bool operator>=(const Ltype &L, const Fixed<INT, FRAC> &R){
        return R.compareLT(L);
    }
    template <typename Ltype>
    friend bool operator<=(const Ltype &L, const Fixed<INT, FRAC> &R){
        return R.compareGT(L);
    }
    template <typename Ltype>
    friend bool operator!=(const Ltype &L, const Fixed<INT, FRAC> &R){
        return R.compareNE(L);
    }
    template <typename Ltype>
    friend bool operator==(const Ltype &L, const Fixed<INT, FRAC> &R){
        return R.compareEQ(L);
    }

private:
    // Comparison functions
    bool compareGT(const Fixed<INT, FRAC> &R)const {
        return number > R.number;
    }
    bool compareLT(const Fixed<INT, FRAC> &R)const {
        return number < R.number;
    }
    bool compareGE(const Fixed<INT, FRAC> &R)const {
        return number >= R.number;
    }
    bool compareLE(const Fixed<INT, FRAC> &R)const {
        return number <= R.number;
    }
    bool compareNE(const Fixed<INT, FRAC> &R)const {
        return number != R.number;
    }
    bool compareEQ(const Fixed<INT, FRAC> &R)const {
        return number == R.number;
    }

    template<fastu16 _INT, fastu16 _FRAC>
    Fixed<_INT, _FRAC> createFixed(const fast64& rawNumber) const{
        Fixed<_INT, _FRAC> ret;
        ret.setRawNumber(rawNumber);
        return ret;
    }

    fixSize number = 0;
};




#endif // FIXEDPOINT_H

