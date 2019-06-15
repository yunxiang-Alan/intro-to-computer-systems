/* 
 * CS:APP Data Lab 
 * 
 * Author: Qizheng Zhang (qizhengz)
 * 
 * bits.c - Source file with your solutions to the project.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the project by
editing the collection of functions in this source file.

CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. The maximum number of ops for each function is given in the
     header comment for each function.

/*
 * STEP 2: Modify the following functions according to the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the btest checker to verify that your solutions produce
 *      the correct answers.
 */


#endif
/* 
 * absVal - absolute value of x
 *   Example: absVal(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 10
 *   Rating: 4
 */
int absVal(int x) {
       int ifnegative = x >> 31; 
       // equals -1 if x is negative, 0 if positive
       return ~ifnegative + 1 + (x^ifnegative);
}
/* 
 * addOK - Determine if can compute x+y without overflow
 *   Example: addOK(0x80000000,0x80000000) = 0,
 *            addOK(0x80000000,0x70000000) = 1, 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int addOK(int x, int y) {
  /*
    Note: the part before | is used to decide if x and y have the same sign,
    since overflow never occurs when x and y have different signs;
    when x and y have the same sign, the part after | is used to decide if
    that sign is equal to the sign of x+y. An overflow occurs if these two
    signs are different.
   */
  int result = !(!((x >> 31) ^ (y >> 31))) | !((x >> 31) ^ ((x+y) >> 31));
  return result;
}
/* 
 * allEvenBits - return 1 if all even-numbered bits in word set to 1
 *   Examples allEvenBits(0xFFFFFFFE) = 0, allEvenBits(0x55555555) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 2
 */
int allEvenBits(int x) {
  int mask = 0x55 + (0x55 << 8) + (0x55 << 16) + (0x55 << 24);
  // the mask is 0101 0101 ... 0101 0101 (0x55555555)
  int masked = x & mask;
  /*
    note: this step leaves all odd-numbered bits 0,
    and enables me to deal only with even-numbered bits
    (I must be a genius to have thought of this)
   */
  int result = !(~masked + mask + 1);
  return result;
}
/*
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  /*
    Note: zero is the only number that has the same value as its opposite
    As a result, for zero, minus^x gives 0000...0000 (0 in decimal)
   */
  int minus = ~x+1;
  return ((minus | x) >> 31) + 1;
  /*
    Note: here I use >> 31 because (minus | x) will be 1000...0000 if x is
    Tmin, and >> 31 perfectly converts every (minus | x) to 1111...1111 (when
    x is not zero)
    Note: at first I used >> 1 and it gives 0xc0000001 when x = Tmin
   */
}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  int mask_one = 0x11 + (0x11 << 8) + (0x11 << 16) + (0x11 << 24);
  // mask_one is 0001...0001
  int mask_two = 0x0f + (0x0f << 8);
  // mask_two is 0000...0000 1111 0000 1111 
  int acc = 0; // accumulator storing num of 1's
  acc += (x&mask_one)+((x>>1)&mask_one)+((x>>2)&mask_one)+((x>>3)&mask_one);
  // shift and add up the num of 1's in each 4-bit nipple

  /*
    Note: now we have the num of 1's in each nipple, and the final thing
    to do is to add them up!
  */
  acc = acc + (acc >> 16);
  // Note: this step above is valid and would not cause overflow b/c the max
  // num of 1's on each nipple right now is 8
  acc = (acc & mask_two) + ((acc >> 4) & mask_two);
  // now each two nipple records the number of 1's
  return (acc + (acc >> 8)) & 0xff;
}
/*
 * bitNor - ~(x|y) using only ~ and & 
 *   Example: bitNor(0x6, 0x5) = 0xFFFFFFF8
 *   Legal ops: ~ &
 *   Max ops: 8
 *   Rating: 1
 */
int bitNor(int x, int y) {
  return (~x)&(~y);
}
/*
 * byteSwap - swaps the nth byte and the mth byte
 *  Examples: byteSwap(0x12345678, 1, 3) = 0x56341278
 *            byteSwap(0xDEADBEEF, 0, 2) = 0xDEEFBEAD
 *  You may assume that 0 <= n <= 3, 0 <= m <= 3
 *  Legal ops: ! ~ & ^ | + << >>
 *  Max ops: 25
 *  Rating: 2
 */
int byteSwap(int x, int n, int m) {
  /*
    Basic logic: 1. extract and store content of byte n and m
    2. subtract byte n and m from x and leave their places 00
    store the result after subtraction as mask_x
    e.g. 12 34 56 78 (n=1,m=3) -> 00 34 00 78
    3. add byte n and m to their new location after the swap to mask_x
   */
  int byte_n = 0xff & (x >> (n << 3));
  int byte_m = 0xff & (x >> (m << 3));
  int minus_n_inx = ~(byte_n << (n << 3))+1;
  int minus_m_inx = ~(byte_m << (m << 3))+1;
  int mask_x = x + minus_n_inx + minus_m_inx;
  int result = mask_x + (byte_m << (n << 3)) + (byte_n << (m << 3));
  return result;
}
/* 
 * conditional - same as x ? y : z 
 *   Example: conditional(2,4,5) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 16
 *   Rating: 3
 */
int conditional(int x, int y, int z) {
  /*
    basic logic: put both y and z in the expression of the final result,
    generate some variable that unlocks either y or z in the expression
    in different situations
   */
  int minus_x = ~(!(!x))+1; // equals 0 (if x is 0) or -1 (if x is 1)
  int result = (y&minus_x) + (z&(~minus_x));
  return result;
}
/*
 * ezThreeFourths - multiplies by 3/4 rounding toward 0,
 *   Should exactly duplicate effect of C expression (x*3/4),
 *   including overflow behavior.
 *   Examples: ezThreeFourths(11) = 8
 *             ezThreeFourths(-9) = -6
 *             ezThreeFourths(1073741824) = -268435456 (overflow)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 3
 */
int ezThreeFourths(int x) {
  // pay extra attention to this!!
  // especially the trick that rounds a num toward 0
  int result = (x << 1) + x; // 3*x
  int sign_result = result >> 31;
  int extra_for_neg = sign_result & 0x3;
  // equals 0 if sign_result is pos and 3 if sign_result is neg
  return (result + extra_for_neg) >> 2;
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  // this func is really hard and uses a brilliant trick
  /*
    basic logic: if a number can be represented as an n-bit integer,
    it would be equal to the value obtained by shifting it first to 
    the left by 32 - n bits and then to the right by 32 - n bits
   */
  int num_shift = 32 + (~n+1); // 32 - n
  int result = !(x ^ ((x << num_shift) >> num_shift));
  return result;
}
/*
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  int mask = 255;
  int shifted = x >> (n << 3);
  return shifted & mask;
}
/* 
 * greatestBitPos - return a mask that marks the position of the
 *               most significant 1 bit. If x == 0, return 0
 *   Example: greatestBitPos(96) = 0x40
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 70
 *   Rating: 4 
 */
int greatestBitPos(int x) {
  /*
    basic logic: fill all the bits to the right of the highest bit with 1,
    and then obtain the intended mask from (the result after filling)
   */
  int if_negative = x >> 31;
  // equals all 0's if pos and all 1's if neg
  int mask_ifneg = if_negative << 31;
  // equals all 0's if pos and 1000...000 if neg
  int neg_mask = ~if_negative;
  int shift_acc = x;
  shift_acc = x | (x >> 1);
  shift_acc = shift_acc | (shift_acc >> 2);
  shift_acc = shift_acc | (shift_acc >> 4);
  shift_acc = shift_acc | (shift_acc >> 8);
  shift_acc = shift_acc | (shift_acc >> 16);
  shift_acc = shift_acc & ~(shift_acc >> 1);  
  return mask_ifneg | (neg_mask & shift_acc);
}
/*
 * implication - return x -> y in propositional logic - 0 for false, 1
 * for true
 *   Example: implication(1,1) = 1
 *            implication(1,0) = 0
 *   Legal ops: ! ~ ^ |
 *   Max ops: 5
 *   Rating: 2
 */
int implication(int x, int y) {
  int result = (!x) | y;
  return result;
}
/* 
 * isAsciiDigit - return 1 if 0x30 <= x <= 0x39 (ASCII codes for characters '0' to '9')
 *   Example: isAsciiDigit(0x35) = 1.
 *            isAsciiDigit(0x3a) = 0.
 *            isAsciiDigit(0x05) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 3
 */
int isAsciiDigit(int x) {
  /*
    basic logic: 
    1. decide if x is in the form of 0x3?,
    this is achieved from using mask_one and mask_two
    purpose: exclude the following two situations
    (1) there is any bit 1 ahead of 3
    (2) x is in form of 0x2?, 0x5?, etc.

    2. decide if adding six to x leads to any change in the second byte
    e.g. if 0x3? changes to 0x4?, then x is not a Ascii digit
  */
  int mask_one = ~16+1;
  // 1111...1111 1111 0000 
  int mask_two = 0x30;
  // 0000...0000 0011 0000
  int if_first_three = !((x & mask_one) ^ mask_two);
  // equals 1 if 0x3?, 0 otherwise
  int add_six = x + 6;
  int if_still_three = !((add_six & mask_one) ^ mask_two);
  return if_first_three & if_still_three;
}
/* 
 * isEqual - return 1 if x == y, and 0 otherwise 
 *   Examples: isEqual(5,5) = 1, isEqual(4,5) = 0
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int isEqual(int x, int y) {
  int result = !(x ^ y);
  return result;
}
/*
 * isLess - if x < y  then return 1, else return 0 
 *   Example: isLess(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLess(int x, int y) {
  /*
    basic logic: 
    1. if x > 0, y < 0, return 0
    2. if x < 0, y > 0, return 1
    3. if x and y have the same sign, compute x + (-y)
    (1) if x + (-y) < 0, return 1
    (2) if x + (-y) > 0, return 0
   */
  int sign_x = x >> 31;
  int sign_y = y >> 31;
  int if_xpos_yneg = (~sign_x) & sign_y; // =1 if x+ y-
  int if_xneg_ypos = sign_x & (~sign_y); // =1 if x- y+
  int x_plus_miny = x + (~y+1); // x + (-y)
  int sign_x_plus_miny = !(!(x_plus_miny >> 31));
  // equals 0 if x + (-y) > 0 and equals 1 if x + (-y) < 0
  return (!if_xpos_yneg) & (if_xneg_ypos | sign_x_plus_miny);
}
/*
 * isNonNegative - return 1 if x >= 0, return 0 otherwise 
 *   Example: isNonNegative(-1) = 0.  isNonNegative(0) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 3
 */
int isNonNegative(int x) {
  int result = !(x >> 31);
  return result;
}
/*
 * isPower2 - returns 1 if x is a power of 2, and 0 otherwise
 *   Examples: isPower2(5) = 0, isPower2(8) = 1, isPower2(0) = 0
 *   Note that no negative number is a power of 2.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 4
 */
int isPower2(int x) {
  // this one takes me a lot of time, take a second look before exams
  /*
    basic logic:
    if x <= 0 then x is not a power of 2;
    if x > 0 then 
       if x&(x-1) = 0, x is a power of 2
       if x&(x-1) != 0, otherwise
   */
  int if_negative = !(!(x >> 31)); // equals 1 if neg and 0 if pos
  int result = (!(!x)) & (!if_negative) & (!(x&(x+~0)));
  return result;
}
/*
 * isTmin - returns 1 if x is the minimum, two's complement number,
 *     and 0 otherwise 
 *   Legal ops: ! ~ & ^ | +
 *   Max ops: 10
 *   Rating: 1
 */
int isTmin(int x) {
  /*
    basic logic: only Tmin and 0 are equal to ~x+1 (their opposite)
    0 because of -0 = 0, Tmin because of overflow (-Tmin > Tmax)
   */
  int minus_x = ~x+1;
  return (!(!x)) & (!(x ^ minus_x)); // (!(!x)) excludes the situation of 0
  /*
    Note: at first I used x & ... and it gives 0 when x = Tmin, why?
   */
}
/*
 * minusOne - return a value of -1 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 2
 *   Rating: 1
 */
int minusOne(void) {
  return ~0;
}
/*
 * rotateLeft - Rotate x to the left by n
 *   Can assume that 0 <= n <= 31
 *   Examples: rotateLeft(0x87654321,4) = 0x76543218
 *   Legal ops: ~ & ^ | + << >> !
 *   Max ops: 25
 *   Rating: 3 
 */
int rotateLeft(int x, int n) {
  /*
    basic logic: store the digits that are going to be rotated to the right,
    and add them to the shifted x
   */
  int mask = ~((~0) << n);
  int left_n = (x >> (32 + (~n+1))) & mask;
  int result = (x << n) + left_n;
  return result;
}
/*
 * satMul2 - multiplies by 2, saturating to Tmin or Tmax if overflow
 *   Examples: satMul2(0x30000000) = 0x60000000
 *             satMul2(0x40000000) = 0x7FFFFFFF (saturate to TMax)
 *             satMul2(0x60000000) = 0x80000000 (saturate to TMin)
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3
 */
int satMul2(int x) {
  /*
    basic logic: if the sign of 2x is different from the sign of x,
    saturate the the value to Tmax or Tmin, depending on whether x is
    positive or negative
  
    value table:
    sign_x  sign_2x return
    0       0       2x
    1       1       2x
    0       1       tmax
    1       0       tmin
  */
  int tmin = 1 << 31;
  int tmax = ~tmin;
  int mul2_x = x << 1;
  int sign_x = x >> 31;
  int sign_2x = mul2_x >> 31;
  int result_1 = (~(sign_x ^ sign_2x) & mul2_x);
  int result_2 = ((~sign_x) & sign_2x) & tmax;
  int result_3 =  (sign_x & (~sign_2x)) & tmin;
  int result = result_1 + result_2 + result_3;
  return result;
}
