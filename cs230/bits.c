int extract_nth_byte(int x, int n){
	int _n = n<<3;
	return (x >> _n) & 0xff;
}

int substitute_byte(int x, int n, int c){
	/*make use of << as a multiplication of power of 2 and calcualate the number of left shift*/
	int _n = n<<3;
	/*shift the value c to the right position*/
	int _c = c<<_n;
	return (~(0xff << _n)&x) | _c;
}

/* return 1 if even-numbered bit in x is set */
int aeb(int x){
	/*create 0x55555555 and perform intersection*/
	int temp = 0x55 + (0x55<<8) + (0x55 <<16)+(0x55<<24);
	return !!(x&temp);
}

int rr(int x, int n){
	int tempx = x >> n;
	int left = 32 + (~n) + 1;
	int tail = tempx << left;
	return tempx + tail;
}

int hmb(int x){
	return 0;
}

int bang(int x){
	return 0;
}

int lbc(int x){
	return 0;
}

int is_addition_no_problem(int x, int y){
	return ((((x>>31)^(y>>31))&0x01)+(((x>>31)&(y>>31))^(~((x+y)>>31)))&0x01);
}
int is_subtraction_no_problem(int x, int y) {
	return (!((x>>31) ^ ((x+(~y)+1)>>31))&0x01)+ (!(x+(~y)+1)&0x01);
}

int is_x_fits_in_16_bit(int x) {
   int sign = x>>31;
   printf("%d\n",sign);
   return !(((sign&(~x))+((~sign)&x))>>15);
}

int absolute_of_x(int x) {
   int onesorzeros = (x>>31);
   int abs;
   abs = (onesorzeros&(~x+1)) + (~onesorzeros&x);
   return abs;
 }

int floor_log_2(int x){
	return 0;
}

int is_le(int x, int y){
	int diff = x+(~y)+1;
	return ((diff>>31) & 0x01)+(!diff & 0x01);
}

int is_fits_in_bits(int x, int n) {
	int sign = x>>31;
	return !(((sign&(~x))+((~sign)&x))>>(n+(~1)+1));
}

int divide_by_power_of_2(int x, int n){
	return (~(x>>31)&(x>>n)) + ((x>>31)&((x+((1<<n)+(~1)+1))>>n));
}