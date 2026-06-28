#include <stdio.h>

int main() {
    char *delims = "\n\r";
    for (int i = 0; i < 4; i++) {
            printf("%hhx ", delims[i]);
    }
    printf("\n");

    return 0;
}