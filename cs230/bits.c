/* 
 * CS:APP Data Lab 
 * 
 * <Please put your name and userid here>
 * 
 * bits.c - Source file with your solutions to the Lab.
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

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
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

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* Copyright (C) 1991-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */
/* This header is separate from features.h so that the compiler can
   include it implicitly at the start of every compilation.  It must
   not itself include <features.h> or any other header that includes
   <features.h> because the implicit include comes before any feature
   test macros that may be defined in a source file before it first
   explicitly includes a system header.  GCC knows the name of this
   header in order to preinclude it.  */
/* glibc's intent is to support the IEC 559 math functionality, real
   and complex.  If the GCC (4.9 and later) predefined macros
   specifying compiler intent are available, use them to determine
   whether the overall intent is to support these features; otherwise,
   presume an older compiler has intent to support these features and
   define these macros by default.  */
/* wchar_t uses Unicode 8.0.0.  Version 8.0 of the Unicode Standard is
   synchronized with ISO/IEC 10646:2014, plus Amendment 1 (published
   2015-05-15).  */
/* We do not support C11 <threads.h>.  */
/* 
 * fp_func4 - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 30
 *   Points: 4
 */
unsigned fp_func4(unsigned uf) {
	unsigned sign, exp, frac, new_exp,new_frac;
	/*If the input is either NaN or infinity, then should return the argument. So, we check only the exp part and see if it is all 1's by checking the first 3 bytes of the input through comparing with 0x7f8.*/
  if ((!(((uf>>20)&0x7f8)^0x7f8))){
		return uf;
	}
	/*If the input is 0 then, it should return itself as well.*/
	else if(!(uf&0x7fffffff)){
		return uf;
	}
  
	else{
		/*Divide the input into sign, exp, and frac part*/
		sign = uf & 0x80000000;
  	exp = ((uf <<1)>>24);
		frac = (uf << 9)>>9;
				
		/*if exp is zero, just shift the fraction part*/
		if (!exp){
			new_frac = frac<<1;
		  new_exp = exp;	
			/*If the exp is zero and if the fraction part is 0x400000 (equivalent to 0.5), then we have to add 1 to exp.*/
			if(!(exp^0x400000)){
         new_exp =1;
			}
		} 
		else{
			/*if exp is not zero, fraction stays the same and we add 1 to the exp, which is equivalent to multiplying by 2.*/
			new_frac = frac;
			new_exp = exp+1;	
			}

		/*Combine the sign, exp and frac through | operation.*/
  	return ((sign|(new_exp<<23))|new_frac);
   }
}

/* 
 * is_addition_no_problem - Will it be okay(no overflow) when we add x and y?
 *   If there is an overflow, return 0. Otherwise, return 1.
 *   Example: is_addition_no_problem(0x80000000,0x80000000) = 0,
 *            is_addition_no_problem(0x80000000,0x70000000) = 1, 
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 20
 *   Points: 3
 */
int is_addition_no_problem(int x, int y) {
  /*If x and y have different sign, then addition is safe. If x, y and x+y have same sign, it means that there is no overflow. so I checked the equality of sign first. If they have same sign comp_sign is 0 and if they have different sign, comp_sign is -1, therefore the function will return 1 by & operation with 1. If comp_sign is 0, then we check whether the sign of x and the sign of (x+y) are same. If it is same, the function returns 1, if not, it returns 0 */
  int comp_sign = ((x>>31)^(y>>31));
	return (comp_sign&0x01) +((~comp_sign)&(~((x>>31)^((x+y)>>31))&0x01));
}


/* 
 * is_x_fits_in_16_bit - Returns 1 if x fits in 16-bit, 2's complement.
 *   Examples: is_x_fits_in_16_bit(34000) = 0, is_x_fits_in_16_bit(-32768) = 1
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 8
 *   Points: 1
 */
int is_x_fits_in_16_bit(int x) {
  /*First check the sign of x. If x is positive, "sign" is 0 and if x is negative, "sign" is -1*/
	int sign = x>>31;
	/*if "sign" is -1, meaning that x is negative, invert x by ~x and then shift x>>15. If "sign" is 0, meaning that x is positive, then, we just shift x>>15. If x>>15 is not zero, then it means x doesn't fit in 16 bits, so return 0, but if x>>15 is zero, return 1.*/
	return !(((sign&(~x))+((~sign)&x))>>15);
}
/* 
 * fp_func1 - Return bit-level equivalent of expression 0.5*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 30
 *   Points: 4
 */
unsigned fp_func1(unsigned uf) {	
	unsigned sign, exp, frac, new_exp,new_frac, minus_exp;
	/*check for all ones in exp. It is either infinty or NaN, so just return argument. First perform & operation with 0x7fffffff (all 1's except the MSB) to ignore the sign. Then mask with 0x7f800000.If it is equal to 0x7f800000(the positive infinity value, then it means that the given input has all 1's in the exponent.*/
  if ((!(((uf&0x7fffffff)&0x7f800000)^0x7f800000))){
		return uf;
	}
	/*if exp is not all ones : check the following*/
	else{
	  /*extract sign, exp and frac part from the argument*/
		sign = uf & 0x80000000;
  	exp = ((uf <<1)>>24);
		frac = (uf << 9)>>9;
		minus_exp = exp-1;
  	/*if exp is zero or one, then first, shift the fraction part*/
		if (!(exp ^ 0x00) || !((minus_exp)^0x00)){
			new_frac = frac >>1;
			/*if the fraction before shifting was odd and the shifted fraction is also odd, then we need to add one to round to even.(ex. frac = 0x7, new_frac = 0x3)*/
			if((!((frac%2)^0x01))&&((new_frac%2)^0x00)){
					new_frac+=0x01;
			}
			/*if exp is one, then we also need to add 1/2 value to fraction*/
			if(!((minus_exp)^0x00)){
					new_frac = new_frac|0x400000;
					new_exp = minus_exp;
			}
			/*if exp is zero, then it will just still be zero*/
			else{
	  		new_exp = exp;
			}
		}
		/*if exp is >1, then we subtract 1 from current exp and fraction part will be still the same.*/
		else{	
			new_exp = minus_exp;
			new_frac = frac;
		}	
	/*combine sign, new_exp and new_frac*/
  return ((sign|(new_exp<<23))|new_frac);
	}
}

/* 
 * fp_func3 - Return bit-level equivalent of absolute value of f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument..
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 10
 *   Points: 2
 */
unsigned fp_func3(unsigned uf) {
	/*first, check it is NaN*/
	/*I used the fact that NaN have all 1's in exp and non-zero numbers in fraction part. The MSB is ignorable in this case. So, by performing & operation with 0x7f8, if the result is equal to 0x7f8, we know that uf has all 1's in exp. Then, we check whether uf contains nonzero numbers in fraction by shifting to left by 9. If it is all zero, then it is not NaN, but if it is not zero then it is NaN*/
  if ((!(((uf>>20)&0x7f8)^0x7f8)) && ((uf<<9))){
		return uf;
	}

	if ((uf >>31) & 0x01){
	/*if uf is not NaN and if sign of uf is negative, we simply invert the sign*/
		uf = uf&0x7fffffff;
	}

  return uf;
}

/* 
 * fp_func2 - Return bit-level equivalent of expression (int) f
 *   for floating point argument f.
 *   Argument is passed as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point value.
 *   Anything out of range (including NaN and infinity) should return
 *   0x80000000u.
 *   Operations allowed to use: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max # of operations: 30
 *   Points: 4
 */
int fp_func2(unsigned uf) {
	int exp,frac;
  int sign=0;
  int  E;
	unsigned new_frac;
	/*first divide the input into sign, exp, frac*/
	sign = ((uf>>31)&0x01);
  exp = ((uf<<1)>>24);
	frac = ((uf<<9)>>9);
	new_frac=  frac;
	/*if uf is -0 or +0 return 0*/
  if(!(uf&0x7fffffff)){
		return 0;
	} 
	/*if exp is all 1's, it is NaN or infinity, so return 0x80000000*/
	else if(!(exp^255)){
	  return 0x80000000;
	}
 
  else{
		/*if exp is 0, it is denormalized, so E =1-bias = 1-127. Also, if it is 2^(-126), then it is too small, so return 0*/
     if (!exp){
						 E = -126;
						 return 0;
						}
		 /*if the input is normalized, then E = exp-127 and also, we need to add the leading one.*/
		 else{
				E =exp-127;
				new_frac = frac+(1<<23);
		 }
     
		 /*if E is negative, then the value is too small and multiplying by fractional number will make it smaller, so return 0.*/
		 if(E>>31){
		 		return 0;
		 }
		 /*if E is greater than 32, the number overflows, so return 0x80000000*/
		 else if((E>>6)){
					return 0x80000000;
	  }
		/*else we will shift the fraction value by (23-E) because the values in fraction that are less than 1/(2^E), when multiplied with 2^E, will be smaller than 1, so it can be rounded to 0.*/
    else{
			/*if the input is negative, multiply(-1)*/
			if(sign){
				return (-1)*(new_frac>>(23-E));
			}
      else{
				return new_frac>>(23-E);
			}
		}

	
	}
}

	

/* 
 * extract_nth_byte - Extract the nth byte from a word x and return it.
 *   The index of the byte at LSB is 0.
 *   The index of the byte at MSB is 3.
 *   Examples: extract_nth_byte(0xFF34AB78,1) = 0xAB
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 6
 *   Points: 2
 */
int extract_nth_byte(int x, int n) {
	/*Because each byte contains 8 bits, we can multiply the input n, by 8. Then, we shift the value x to the right by n*8, which brings the nth byte to the right end. We can simply perform & opertion with 0xff to extract the last 8 bits.*/
 	int _n = n << 3;
	return (x >> _n) & 0xff;
}


/* 
 * substitute_byte(x,n,c) - In a word x, replace the nth byte with a new byte c.
 *   The index of the byte at LSB is 0.
 *   The index of the byte at MSB is 3.
 *   Examples: substitute_byte(0xffffffff,1,0xab) = 0xffffabff
 *   Assume that 0 <= n <= 3 and 0 <= c <= 255.
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 10
 *   Points: 3
 */
int substitute_byte(int x, int n, int c) {
	/*Similar to extract byte question, we can find out where the wanted byte is in the value by multiplying n by 8, because each byte has 8 bits. Then we will shift the value c to the correct position within the value. We empty the about-to-be-replaced byte within x and perform | operation with the shifted c to get the result.*/
  int _n = n << 3;
	int _c = c << _n;
  return (~(0xff << _n)& x) | _c;
}

/*
 * floor_log_2 - return floor(log base 2 of x), where x > 0
 *   Example: floor_log_2(16) = 4
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 90
 *   Points: 4
 */
int floor_log_2(int x) {
  int count=0;
	int new_x = x;
	/*Divide the input into 8 groups of 4 bits. Starting from the right-end group, we right shift 1 byte and check if the rest is 0 or not. If it is not zero, we know that x is at least 2^4, so we add 4 to "count". Then we move on to the next group and do the same until we are left with the left most group. If we right shift and the remaining is zero, then nothing will be added.*/
	count += (!!(x>>4))<<2;
	count += (!!(x>>8))<<2;
	count += (!!(x>>12))<<2;
	count += (!!(x>>16))<<2;
	count += (!!(x>>20))<<2;
  count += (!!(x>>24))<<2;
	count += (!!(x>>28))<<2;
	/*Now we know that 2^count gives us the lower bound of x, meaning that we know the number is larger than 2^count and less than 2^(count+4). Therefore, we have to check for the values between 2^count and 2^(count+4) by first, shifting right by count, which is equivalent to performing 2^count. Then by checking the last 4 bits, we can accurately find the lower-bound(=floor) of x. */
   new_x= x>>count;
	 count += (!!(new_x>>1))&0x01;
	 count +=(!!(new_x>>2))&0x01;
	 count += (!!(new_x>>3))&0x01;

  return count;  
}
/* 
 * absolute_of_x - Return abs(x)
 *   Example: absolute_of_x(-1) = 1.
 *   You may assume -TMax <= x <= TMax
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 10
 *   Points: 4
 */
int absolute_of_x(int x) {
  /*first find the sign of x*/
	int sign  = (x>>31);
	int abs;
	/*if sign is positive, meaning that the value sign is 0, then we just return x by (~sign&x). if sign is negative, then the value sign is -1, then we obtain the negative value of the current x by (~x+1) and return that. This behaves somewhat like an if/else function (at least my intention was...) */
	abs = (sign&(~x+1)) + (~sign&x);
  return abs;
}
/* 
 * is_le - Return 1 when x <= y, else return 0 
 *   Example: is_le(4,5) = 1.
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 24
 *   Points: 3
 */
int is_le(int x, int y) {
	/*first compare the signs of x and y to avoid overflow when finding the difference between x,y by y-x. if x,y have equal sign, return -1 and if they have different sign return 0. Different methods will be used to determine whether x<=y depending on the equality of signs.*/
	int comp_sign = ~((x>>31)^(y>>31));
	/*if x,y have same sign, we subtract x from y. If (y-x)>>31 is 0 then y>=x and if (y-x)>>31 is -1 then x>y,so by adding 1 to these values we can get the correct answer. If x,y have different sign, if x>>31 is 0, meaning that x is positive, then we know that x>y so we return 0 and if x>>31 is -1, then x is negative so we know that x<y and return 1*/
 	int diff =y + (~x)+1;
	return (comp_sign&((diff>>31)+1)) + ((~comp_sign) & ((!!(x>>31))&0x01));

	}


/* 
 * is_fits_in_bits - Can x be represented in 2's complement integer with n-bit?
 s*   1 <= n <= 32
 *   Examples: is_fits_in_bits(5,3) = 0, is_fits_in_bits(-4,3) = 1
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 15
 *   Points: 2
 */
int is_fits_in_bits(int x, int n) {
	/*Essentially, if we shift positiive x by n bits and the remaining bits are not all 0s, then we know that x doesnt fit in n bits.So, first we find the sign of the x. If it is negative, we find the ~x and perform right shift by n-1 times. */
/*	int sign = x>>31;
	return !(((sign&(~x))+((~sign)&x))>>(n+(~1)+1));
*/
	int xsign = (x>>31)&0x1;
	int poscase = !(x>>(n+(~1)+1));
	int negcase = !(~(x>>(n+(~1)+1)));

	return ((!xsign)&poscase)+(xsign&negcase);
	}


/*
 * lbc - The number of consecutive one's in left-side (MSB-side) end of a word.
 *   Examples: lbc(-1) = 32, lbc(0xFFFF000F0) = 16
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 50
 *   Points: 4
 */
int lbc(int x) {
	/*The logic is very similar to the function floor_log_2. First, create temp variables from 1~7, which corresponds to the number of left most groups. So, temp1 contains the first left most group, temp2 contains the 2 left most groups, ...and temp7 contains 7 left most groups. */
  int temp1 = x>>28;
	int temp2 = x>>24;
	int temp3 = x>>20;
	int temp4 = x>>16;
	int temp5 = x>>12;
	int temp6 = x>>8;
	int temp7 = x>>4;

	int count=0;
  int neg1 = ~0;
	int new_x;
  /*Now we check if any of the temp is equal to 0xf. We have to be aware that temp variables will be negative if MSB is 1, so shifting the negative inputs will be done through arithmetic shifting. Therefore, if temp variables equal -1(neg1) then it means that it contains all 1s, so we can add 1 to count. We accumulate count, so we start from temp1.*/
	count += (!(temp1^neg1));
	count += (!(temp2^neg1));
	count += (!(temp3^neg1));
	count += (!(temp4^neg1));
	count += (!(temp5^neg1));
	count += (!(temp6^neg1));
	count += (!(temp7^neg1));
/*I only added +1 to count(if temp contained all 1s), so we have to scale it by 4 because 1 group contains 4 bits*/
  count = count<<2;  

  /*Now we needed to check the left most non-0xf group. We perform shifting to place it to the last position. Shifting to the right and checking if it is all 1s (same idea as above), we add 1 bit if it is true.*/
  new_x = (x<< count)>>28;
	count += !(~(new_x>>3));
	count += !(~(new_x>>2));
	count += !(~(new_x>>1));
	count += !(~new_x);
	return count;
}

/* 
 * divide_by_power_of_2 - Return the same value as x/(2^n), for 0 <= n <= 30
 *                        Please round to 0.
 *   Examples: divide_by_power_of_2(15,1) = 7, divide_by_power_of_2(-33,4) = -2
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 15
 *   Points: 2
 */
int divide_by_power_of_2(int x, int n) {
	/*	return (x >> n)+ ((x>>28)&0x01);*/
	/*Divide by power of 2 is essentially equivalent to shifting to the right. However, we need to watch out for negative numbers. Depending on the value of (x>>31), one or the other operation will take effect. If the number is positive, then (x>>31) is 0, so the part where ~(x>>31) will be true. So, we just shift right by n. If x is negative, (x>>31) is true, and we cannot just shift right, because of the signed bits. Therefore, we will add the value 2^n -1 to x and then shift it by n to get the negative number rounded to 0.*/
  return (~(x>>31)&(x>>n)) + ((x>>31)&((x+((1<<n)+(~1)+1))>>n));
}
/* 
 * is_subtraction_no_problem - Can x-y be done without any problem(overflow)?
 *   If there is an overflow, return 0. Otherwise, return 1.
 *   Example: is_subtraction_no_problem(0x80000000,0x80000000) = 1,
 *            is_subtraction_no_problem(0x80000000,0x70000000) = 0, 
 *   Operations allowed to use: ! ~ & ^ | + << >>
 *   Max # of operations: 20
 *   Points: 3
 */
int is_subtraction_no_problem(int x, int y) {
	/*If x and y have same sign, it is safe. If x is pos and y is neg and the diff of (x-y) is neg, then it means that there is a positive overflow. If x is neg and y is pos, and the diff of (x-y) is pos, then there is a negative overflow. So, first, figure out whether x and y have same sign or not and store it in comp_sign(value is either -1 or 0). Then calculate diff. If comp_sign is -1, then just return 1. If not, check the sign of diff and return 1 or 0. */
/*
	int comp_sign = ~((x>>31)^(y>>31));
	int diff= x + (~y) +1;
	return (comp_sign&0x01) + ((~comp_sign)&((!((x>>31)^(diff>>31)))&0x01));
*/
	int xsign = x>>31;
	int ysign= y>>31;
	int subsign = (x+(~y)+1)>>31;
	int comp = xsign^ysign;

	return ((!comp)&0x1)+((comp)&(!((subsign^xsign)&0x1)));
	}


