#include <stdio.h>

int main(int argc, char ** argv) {
	char name[128];
	printf("Enter your name:\n");
	scanf("%128s", name);
	printf("Hello, %s!\n", name);
	return 0;
}
