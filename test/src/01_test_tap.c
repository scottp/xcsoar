#include <stdio.h>

int main() {
	printf("1..100\n");
	printf("# a comment\n");
	printf("ok 1 - yes it works\n");
	int i;
	for(i = 0; i < 100; i++) {
		printf("not ok %i - inside loop %i\n", i+2, i);
	}
	return 0;
}
