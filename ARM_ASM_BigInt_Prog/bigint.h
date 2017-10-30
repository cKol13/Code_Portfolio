#ifndef BIGINT_H
#define BIGINT_H

struct bigint_struct;

/* define bigint to be a pointer to a bigint_struct */
typedef struct bigint_struct* bigint;

/* there are three ways to create a bigint */
bigint bigint_from_str(char *s);
bigint bigint_from_int(int i);
bigint bigint_copy(bigint source);

/* bigints can be converted to integers */
/* if it won't fit in an integer, the program exits */
int bigint_to_int(bigint b);

/* to print a bigint, you must convert it to a string */
char *bigint_to_str(bigint b);

/* this function frees the memory used by a bigint */
void bigint_free(bigint b);

/* there are five arithmetic operations */
bigint bigint_add(bigint l, bigint r);
bigint bigint_sub(bigint l, bigint r);
bigint bigint_mul(bigint l, bigint r);
bigint bigint_div(bigint l, bigint r);
bigint bigint_negate(bigint b);

/* Two shift operations (all bigints are signed, so there is no
   such thing as an unsigned shift right) */
bigint bigint_shift_left(bigint l, int shamt);
bigint bigint_shift_right(bigint l, int shamt);

/* There are seven comparison operations.  
   They return 1 for true, or 0 for false. */
inline int bigint_is_zero(bigint b);
inline int bigint_le(bigint l, bigint r);
inline int bigint_lt(bigint l, bigint r);
inline int bigint_ge(bigint l, bigint r);
inline int bigint_gt(bigint l, bigint r);
inline int bigint_eq(bigint l, bigint r);
inline int bigint_ne(bigint l, bigint r);

#endif

