#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(void){
    void *cur_brk, *tmp_brk = NULL;

    // sbrk(0) gives current "program break" location
    tmp_brk = cur_brk = sbrk(0);
    printf("program break1: %p\n", cur_brk);

    // brk(addr) incrementes/decrements proram break location
    brk(cur_brk + 0x3000);
    cur_brk = sbrk(0);
    printf("program break2: %p\n", cur_brk);

    brk(tmp_brk);
    cur_brk = sbrk(0);

    // printf("program break3: %p\n", cur_brk);
    return 0;
}