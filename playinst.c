#include <stdio.h>

int add_s(int a, int b);

int main(int argc, char **argv)
{
    int r;
    unsigned int *ip;
    unsigned int iw;
    
    r = add_s(1,2);

    printf("add_s(1,2) = %d\n", r);

    ip = (unsigned int *) add_s;
    iw = *ip;

    printf("iw = %X\n", iw);

    ip = ip + 1;
    iw = *ip;

    printf("iw = %X\n", iw);
        
    return 0;
}
