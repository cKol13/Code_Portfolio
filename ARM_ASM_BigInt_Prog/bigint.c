/*
    Authors: Chris Kolegraff, Dr. Pyeatt
    File: bigint.c
    Class: CSC 314
    Project: Program 1 - Bigint Factorial
    
    Description: This file contains the function definitions for the bigint
    struct. There are also function declarations for functions that are
    written in ARM Assembly. At the bottom of this file are old C functions
    that have been replaced by Assembly versions. They are not included in the
    compiled code, as they are either outdated, incomplete, or contain errors.
    Some functions at the bottom of this file are not a direct translation from
    C to Assembly, as some algorithms can be implemented in a much better or 
    different way in Assembly than in C.
*/



#include <bigint.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>

typedef uint32_t chunk;
typedef int32_t schunk;
typedef uint64_t bigchunk;
#define CHUNKMASK 0xFFFFFFFF

#define BITSPERCHUNK ((sizeof(chunk)<<3))

// A bigint is an array of chunks of bits
struct bigint_struct{
  chunk *blks;        // array of bit chunks 
  int size;           // number of chunks in the array
};

#define MAX(a,b) ((a<b)?b:a)

/********************************************************/
/*            ASM Function Declarations                 */
/********************************************************/
// Add two bigints with a carry value
bigint bigint_adc(bigint l, bigint r, chunk carry);

// Multiply a bigint with unsigned int
bigint bigint_mul_uint(bigint l, unsigned r, bigint sum, 
    bigint tmp1, bigint shiftResult);
    
// Shift a blocks array by chunks
bigint bigint_shift_left_chunk(bigint l, int chunks, bigint result);

// Create a bigint from an int value
bigint bigint_from_int(int val);

// Allocate space for a bigint of size 'chunks'
bigint bigint_alloc(int chunks);

// Get the remainder of a bigint / num divide
unsigned bigint_smallmod(bigint b,chunk num);

// Free a bigint's resources
void bigint_free(bigint b);

// Add two bigints together, storing result in 'l'
bigint accumulate_noTrim(bigint l, bigint r);



/********************************************************/
/*  Utility functions                                   */
/********************************************************/

void alloc_err()
{
  printf("error allocating\n");
  exit(1);
}

/********************************************************/
void bigint_dump(bigint b)
{
  int i;
  printf("%d chunks:",b->size);
  for(i=b->size-1;i>=0;i--)
    printf(" %02X",b->blks[i]);
  printf("\n");
}

/********************************************************/
bigint bigint_trim(bigint b)
{
    bigint d;
    int i = b->size-1;
    if(i>0)
    {
        if(b->blks[i] == 0)
        { // we have a leading block that is all 0
            do
                i--; // search for first block that is not all 0
            while ((i>0) && (!b->blks[i]));
            
            if(b->blks[i] & (1<<(BITSPERCHUNK-1)))
                i++;  // if msb of current block is 1, then we went too far
        }
    }
    
    i++;  // i is now the number of blocks to copy
    if(i < b->size)
    {
        d = bigint_alloc(i);
        memcpy(d->blks,b->blks,d->size*sizeof(chunk));
    }
    else
    d = bigint_copy(b);
    return d;
}

/********************************************************/
/* bigint_cmp compares two bigints 
   returns -1 if l<r
   returns 0 if l==r
   returns 1 if l>r
*/
int bigint_cmp(bigint l, bigint r)
{
  int i=l->size-1;
  int j=r->size-1;
  while(i>j)
    if(l->blks[i--])
      return 1;
  while(j>i)
    if(r->blks[j--])
      return -1;
  while(i>=0)
    {
      if(l->blks[i]<r->blks[i])
	return -1;
      if(l->blks[i]>r->blks[i])
	return 1;
      i--;
    }
  return 0;
}

/********************************************************/
inline int bigint_is_zero(bigint b)
{
  int i;
  for(i=0;i<b->size;i++)
    if(b->blks[i])
      return 0;
  return 1;
}

/********************************************************/
bigint bigint_shift_right_chunk(bigint l, int chunks)
{
   bigint tmp;
   int i;
   tmp=bigint_alloc(l->size-chunks);
   for(i=0;i<tmp->size;i++)
     {
       if(i<chunks)
	 tmp->blks[i]=0;
       else
	 tmp->blks[i]=l->blks[i-chunks];
     }
   return tmp;
}

/********************************************************/
/* Conversion and copy functions                        */
/********************************************************/
bigint bigint_copy(bigint source)
{
  bigint r;
  r = bigint_alloc(source->size);
  memcpy(r->blks,source->blks,r->size*sizeof(chunk));
  return r;
}

/********************************************************/
bigint bigint_complement(bigint b)
{
  int i;
  bigint r = bigint_copy(b);
  for(i=0;i<r->size;i++)
    r->blks[i] ^= CHUNKMASK;
  return r;
}

/********************************************************/
bigint bigint_negate(bigint b)
{
  bigint tmp1,tmp2;
  bigint r = bigint_complement(b);
  tmp1=bigint_from_int(1);
  tmp2=bigint_adc(r,tmp1,0);
  bigint_free(tmp1);
  bigint_free(r);
  return tmp2;
}

/********************************************************/
char *bigint_to_str(bigint b)
{
  int chars,i;
  unsigned remainder;
  char *s,*r;
  bigint tmp;
  //bigint tmp2;
  /* rough estimate of the number of characters needed */
  chars = log10(pow(2.0,(b->size * BITSPERCHUNK)))+3;
  i = chars-1;
  if((s = (char*)malloc(1 + chars * sizeof(char))) == NULL)
    {
      perror("bigint_str");
      exit(1);
    }
  s[i]=0;
  tmp = bigint_copy(b);

  if(bigint_is_zero(tmp))
    s[--i] = '0';
  else
    do
      {
	remainder = bigint_smallmod(tmp,10);
	s[--i] = remainder + '0';
      } while(!bigint_is_zero(tmp));
      
  r = strdup(s+i);
  bigint_free(tmp);
  free(s);
  return r;
}

/********************************************************/
bigint bigint_from_str(char *s)
{
  bigint d;
  bigint power;
  bigint ten;
  bigint tmp;
  bigint currprod;
  int i,negative=0;
  d = bigint_from_int(0);
  ten =   bigint_from_int(10);
  power = bigint_from_int(1);
  if(*s == '-')
    {
      negative = 1;
      s++;
    }
  for(i=strlen(s)-1; i>=0;i--)
    {
      if(!isdigit(s[i]))
	{
	  fprintf(stderr,"Cannot convert string to bigint\n");
	  exit(1);
	}
      tmp = bigint_from_int(s[i]-'0');
      currprod = bigint_mul(tmp,power);
      bigint_free(tmp);
      tmp = bigint_adc(currprod,d,0);
      bigint_free(d);
      d=tmp;
      bigint_free(currprod);
      if(i>0)
	{
	  tmp = bigint_mul(power,ten);
	  bigint_free(power);
	  power = tmp;
	}
    }
  if(negative)
    {
      tmp=bigint_negate(d);
      bigint_free(d);
      d=tmp;
    }
  return d;
}

/********************************************************/
int bigint_to_int(bigint b)
{
  int i,negative=0,result=0;
  bigint tmp1, tmp2;
  tmp1 = bigint_trim(b); /* make a trimmed copy */
  if(tmp1->size*sizeof(chunk) > sizeof(int))
    {
      fprintf(stderr,
	      "Cannot convert bigint to int\n%ld bytes\n",
	      (long)tmp1->size*sizeof(chunk));
      exit(1);
    }
  /* check sign and negate if necessary */
  if(tmp1->blks[tmp1->size-1] & (1<<(BITSPERCHUNK-1)))
    {
      negative=1;
      tmp2=bigint_negate(tmp1);
      bigint_free(tmp1);
      tmp1=tmp2;
    }
  for(i=tmp1->size-1;i>=0;i--)
    result |= (tmp1->blks[i]<<(i*BITSPERCHUNK));
  bigint_free(tmp1);
  if(negative)
    result = -result;
  return result;
}

/********************************************************/
bigint  bigint_extend(bigint b,int nchunks)
{
  bigint tmp;
  int i,negative;
  negative=0;
  if(b->blks[b->size-1] & (1<<(BITSPERCHUNK-1)))
    negative=1;
  tmp = bigint_alloc(nchunks);
  for(i=0;i<nchunks;i++)
    if(i < b->size)
      tmp->blks[i] = b->blks[i];
    else
      if(negative)
	tmp->blks[i] = CHUNKMASK;
      else
	tmp->blks[i] = 0;
  return tmp;
}

/********************************************************/
/* Mathematical operations                              */
/********************************************************/

/********************************************************/
/* The add function calls adc to perform an add with    */
/* initial carry of zero                                */
bigint bigint_add(bigint l, bigint r)
{
  return bigint_adc(l,r,0);
} 
    
/********************************************************/
bigint bigint_sub(bigint l, bigint r)
{
  bigint tmp1,tmp2;
  tmp1 = bigint_complement(r);
  tmp2 = bigint_adc(l,tmp1,1);
  bigint_free(tmp1);
  return tmp2;
}

/********************************************************/
bigint bigint_shift_left(bigint l, int shamt)
{
  int extra,i;
  bigint tmp, shiftResult;
  
  l =  bigint_extend(l,l->size+1);
  extra = shamt % BITSPERCHUNK;
  shamt = shamt / BITSPERCHUNK;
  shiftResult = bigint_alloc(l->size + shamt);
  if(shamt)
    {
      l = bigint_shift_left_chunk(l,shamt, shiftResult);
    }
  if(extra)
    {
      for(i=l->size-1;i>0;i--)
	{
	  l->blks[i] = (l->blks[i]<<extra) |
	    (l->blks[i-1]>>(BITSPERCHUNK-extra));
	}
      l->blks[0] = (l->blks[0]<<extra);
    }
  tmp = bigint_trim(l);
  bigint_free(l);
  bigint_free(shiftResult);
  return tmp; 
}

/********************************************************/
bigint bigint_shift_right(bigint l, int shamt)
{
  int extra,i;
  bigint tmp;
  extra = shamt % BITSPERCHUNK;
  shamt = shamt / BITSPERCHUNK;
  l =  bigint_shift_right_chunk(l,shamt);
  if(extra)
    {
      for(i=0;i<l->size;i++)
	{
	  l->blks[i] = (l->blks[i]>>extra) |
	    (l->blks[i+1]<<(BITSPERCHUNK-extra));
	}
    }
  tmp = bigint_trim(l);
  bigint_free(l);
  return tmp; 
}

/********************************************************/
// Still broken/slow, haven't fixed/optimized it
bigint bigint_div(bigint l, bigint r)
{
  bigint lt,rt,tmp,q, shiftResult;
  int shift,chunkshift,negative=0;
  q = bigint_from_int(0);
  lt = bigint_trim(l);
  rt = bigint_trim(r);
  
  shiftResult = bigint_alloc(l->size);
  
  if(lt->size >= rt->size)
    {
      /* make sure the right operand is not negative */
      if(r->blks[r->size-1]&(1<<(BITSPERCHUNK-1)))
	{
	  negative = 1;  /* track sign of result */
	  tmp = rt;
	  rt = bigint_negate(rt);
	  bigint_free(tmp);
	}
      /* make sure the left operand is not negative */
      if(l->blks[l->size-1]&(1<<(BITSPERCHUNK-1)))
	{
	  negative ^= 1;  /* track sign of result */
	  tmp = lt;
	  lt = bigint_negate(lt);
	  bigint_free(tmp);
	}
      /* do shift by chunks */
      chunkshift = lt->size - rt->size - 1;
      if(chunkshift>0)
	{
	  rt = bigint_shift_left_chunk(rt, chunkshift, shiftResult);
	}
      /* do remaining shift bit-by-bit */
      shift = 0;
      while((shift < 31) && bigint_lt(rt,lt))
	{
	  shift++;
	  tmp = rt;
	  rt = bigint_shift_left(rt,1);
	  bigint_free(tmp);
	}
      shift += (chunkshift * BITSPERCHUNK); /* total shift */
      /* loop to shift right and subtract */
      while(shift >= 0)
	{
	  tmp = q;
	  q = bigint_shift_left(q,1);
	  bigint_free(tmp);
	  if(bigint_le(rt,lt))
	    {
	      /* perform subtraction */
	      tmp = lt;
	      lt = bigint_sub(lt,rt);
	      bigint_free(tmp);
	      /* change lsb from zero to one */
	      q->blks[0] |= 1;
	    }
	  tmp = rt;
	  rt = bigint_shift_right(rt,1);
	  bigint_free(tmp);
	  shift--;
	}
      /* correct the sign of the result */
      if(negative)
	{
	  tmp = bigint_negate(q);
	  bigint_free(q);
	  q = tmp;
	}
    }
  bigint_free(rt);
  bigint_free(lt);
  bigint_free(shiftResult);
  return q;
}

/********************************************************/
/* Test and compare functions                           */
/********************************************************/
inline int bigint_le(bigint l, bigint r)
{
  return (bigint_cmp(l, r) < 1);
}

/********************************************************/
inline int bigint_lt(bigint l, bigint r)
{
  return (bigint_cmp(l, r) == -1);
}

/********************************************************/
inline int bigint_ge(bigint l, bigint r)
{
  return (bigint_cmp(l, r) > -1);
}

/********************************************************/
inline int bigint_gt(bigint l, bigint r)
{
  return (bigint_cmp(l, r) == 1);
}

/********************************************************/
inline int bigint_eq(bigint l, bigint r)
{
  return (!bigint_cmp(l, r));
}

/********************************************************/
inline int bigint_ne(bigint l, bigint r)
{
  return abs(bigint_cmp(l, r));
}




// This section contains the functions that have been rewritten in ASM.
// They may or may not compile, or be direct translations of what the 
// ASM code does. Descriptions of these functions can be found in their 
// respective '.S' files.
/*********************************************************/

# ifndef USE_ASM

bigint bigint_alloc(int chunks)
{
  bigint r = (bigint)malloc(8);
  if(r == NULL)
    {
      perror("bigint_alloc");
      exit(1);
    }
  r->size = chunks;
  r->blks = (chunk*)malloc(chunks << 2);
  if(r->blks == NULL)
    {
      perror("bigint_alloc");
      exit(1);
    }
  return r;
}



void bigint_free(bigint b)
{
    free(b->blks);
    free(b);
}



/* smallmod divides a bigint by a small number
   and returns the modulus. b changes as a SIDE-EFFECT.  
   This is used by the to_str function. 
*/
unsigned bigint_smallmod(bigint b,chunk num)
{
    // Don't write a C version, just implement in ASM, make own divide function

    bigchunk tmp;
    chunk tmp2;
    int i;

    // start with most significant chunk and work down, taking
    // two overlapping chunks at a time 
    tmp = b->blks[b->size-1];
    for(i=b->size-1; i>0;i--)
    {
        b->blks[i] = tmp / num;    // Divide 64-bit number
        tmp2 = tmp % num;
        // Get mod of previous division, shift left 32, orr in next block value
        tmp = (tmp2 << BITSPERCHUNK) | b->blks[i-1]; 
        // Set hiword of tmp to tmp2, set loword of tmp to b->blks[i-1]
    }
    
    b->blks[0] = tmp/num;
    tmp = (tmp % num);
    return tmp;
}

bigint bigint_shift_left_chunk(bigint l, int chunks, bigint result)
{
    int i, j = 0;
    
    for(i = 0; i < result->size; i++){
        if(i < chunks)
            result->blks[i] = 0;
        else if(j < l->size)
            result->blks[i] = l->blks[j++];
        else
            result->blks[i] = 0;
    }
    
    return result;
}

/********************************************************/
/* this is the internal add function.  It includes a    */
/* carry. Several other functions use it.               */
bigint bigint_adc(bigint l, bigint r, chunk carry)
{
  bigint sum,tmpl,tmpr;
  int i,nchunks;
  bigchunk tmpsum;
  /* allocate one extra chunk to make sure overflow
     cannot occur */
  nchunks = MAX(l->size,r->size)+1;
  /* make sure both operands are the same size */
  tmpl = bigint_extend(l,nchunks);
  tmpr = bigint_extend(r,nchunks);
  /* allocate space for the result */
  sum = bigint_alloc(nchunks);
  /* perform the addition */
  for(i=0 ;i < nchunks ; i++)
    {
      /* add the current block of bits */
      tmpsum = tmpl->blks[i] + tmpr->blks[i] + carry;
      sum->blks[i] = tmpsum & CHUNKMASK;
      /* calculate the carry bit for the next block */
      carry = (tmpsum >> BITSPERCHUNK)&CHUNKMASK;
    }
  bigint_free(tmpl);
  bigint_free(tmpr);
  tmpl = bigint_trim(sum);
  bigint_free(sum);
  return tmpl;
}

bigint bigint_mul_uint(bigint l, unsigned r, bigint sum,
    bigint tmp1, bigint shiftResult)
{
    int i;
    bigchunk tmpchunk;
    
    // perform the multiply
    for(i=0;i<l->size;i++){
        tmpchunk = (bigchunk)l->blks[i] * r;
        
        tmp1->blks[0] = tmpchunk & CHUNKMASK;
        tmp1->blks[1] = (tmpchunk>>BITSPERCHUNK) & CHUNKMASK;
        tmp1->blks[2] = 0;
        
        bigint_shift_left_chunk(tmp1,i, shiftResult);
        accumulate_noTrim(sum, shiftResult);
    }
    
    return sum;
}

bigint accumulate_noTrim(bigint l, bigint r){
    int i;
    
    for(i = 0; i < r->size; i++)
        l->blks[i] = l->blks[i] + r->blks[i];
    
    return l;
}

bigint bigint_from_int(int val)
{
    bigint tmp = bigint_alloc(1);
    tmp->blks[0] = val;
    return tmp;
}


/*
    bigint_mul uses the algorithm from Section 7.2.5
    The strategy for the most speed up I am using is to try and pull as much 
    dynamic memory allocation out of the loop as possible, and put it into the 
    highest level of function calls. Meaning, I want to allocate all dynamic 
    memory in bigint_mul and have all the functions used by bigint_mul expect 
    those variables they use as temps.
*/
bigint bigint_mul(bigint l, bigint r)
{
    bigint sum, tmp1, tmp2, tmp3, shiftResult;
    int i, j;
    
    // We know the sizes required for the sum, tmp1, tmp3, and shiftResult
    // variables, because of this, we can allocate them outside of the 
    // multiplication loop, and pass them in to the functions in the loop

    // the result may require the sum of the number of chunks in l and r
    sum = bigint_alloc(l->size + r->size);
    tmp1 = bigint_alloc(l->size + 1); // Used to store result for uint mult
    tmp3 = bigint_alloc(3); // Used as a temp variable for uint mult
    shiftResult = bigint_alloc(l->size + r->size + 1); 
    
    // These bigints need to be initialized to 0 before being used
    for(i = 0; i < sum->size; i++) sum->blks[i] = 0;
    for(i = 0; i < tmp1->size; i++) tmp1->blks[i] = 0;
    
    // Perform the multiplication, shift, and accumulation algorithm
    // No memory allocation/freeing is performed in this loop
    for(i=0;i<r->size - 1;i++){
        bigint_mul_uint(l,r->blks[i], tmp1, tmp3, shiftResult);
        bigint_shift_left_chunk(tmp1,i, shiftResult);
        accumulate_noTrim(sum, shiftResult);
        
        // Reset tmp1 for bigint_mul_uint to work
        for(j = 0; j < tmp1->size; j++) tmp1->blks[j] = 0;
    }

    // For the last iteration of the accumulation algorithm, 
    // use Pyeatt's bigint_adc function instead of the accumulate_noTrim 
    // function in order to trim result
    bigint_mul_uint(l,r->blks[i], tmp1, tmp3, shiftResult);
    bigint_shift_left_chunk(tmp1,i, shiftResult);
    tmp2 = sum;
    sum = bigint_adc(sum,shiftResult,0);
    
    bigint_free(tmp1);
    bigint_free(tmp2);
    bigint_free(tmp3);
    bigint_free(shiftResult);
    
    return sum;
}

#endif
