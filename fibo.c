#include <stdio.h>
#include <emscripten.h>
#include <stdint.h>

EMSCRIPTEN_KEEPALIVE
uint64_t fibo(int n) {
	uint64_t f0 = 0;
	uint64_t f1 = 1;
	int count;
	if (n <= 0) return 0;
	for (count = 0; count <n; count++){
		uint64_t temp = f0+f1;
		f0 = f1;
		f1 = temp;
	}
	return f1;
} 
