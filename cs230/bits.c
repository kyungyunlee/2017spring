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

