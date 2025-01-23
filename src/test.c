#include <stdio.h>

int main() {
	FILE * f = fopen("bin", "wb");
	char c = 'E';
	fwrite(&c, sizeof(char), 1, f);
	fclose(f);
}
