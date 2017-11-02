#include <emscripten.h>
#include <time.h>

EMSCRIPTEN_KEEPALIVE
int fib(int n) {
	if (n <= 1) return 1;
	return n*fib(n-1);
}

EMSCRIPTEN_KEEPALIVE
int cntr = 0;

EMSCRIPTEN_KEEPALIVE
char shouldCount = 0;


EMSCRIPTEN_KEEPALIVE
void startCount(){
	shouldCount = 1;
	while(shouldCount){
		cntr++;
		unsigned long now = (unsigned long)time(NULL);

		while (now + 1 > (unsigned long)time(NULL)) {}
		
	}
}


EMSCRIPTEN_KEEPALIVE
int getCount(){
	return cntr;
}
