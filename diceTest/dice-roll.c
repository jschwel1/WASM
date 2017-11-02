#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <emscripten/emscripten.h>

/*
int main(int argc, char ** argv) {
	printf("WebAssembly module loaded\n");
	return 0;
}
*/
int EMSCRIPTEN_KEEPALIVE roll_dice() {
	srand ( time(NULL) );
	return rand()%6+1;
}

double EMSCRIPTEN_KEEPALIVE avgNRolls(int n) {
	srand ( time(NULL) );
	int i;
	int sum = 0;
	for (i = 0; i < n; i++)
		sum += rand()%6+1;
	return (double)(sum)/n;
}

double fact(int n) {
	if (n <= 1) return 1;
	return n*fact(n-1);
}

double EMSCRIPTEN_KEEPALIVE choose(int n, int k) {
	printf("%d!/(!%d * (%d))\n", n, k, (n-k));
	return fact(n)/(fact(k)*fact(n-k));
	
}


