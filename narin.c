#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#define PUT(p, val) (*(int *)(p) = (val))

int main(void){
    int b = 12;
    int c = 10;
    int *a;
    PUT(a, 0);
    PUT(a, b);
    printf("%d\n", *a);
    return 0;
}