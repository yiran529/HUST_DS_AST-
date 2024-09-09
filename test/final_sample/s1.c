#include <stdio.h>
int a;
void def();long b;
char d,e;
double c,h;
int num[10];
int i;
int func(int a,int b)
{
    a=1;
    b=2l;
    c=0.14;
    h=3.13f;
    d='\x22';
    e='\'';
    e='1';
    a=(1+b);
    b=(2+b);
    b=c;
    if(a==b)
        a=(a+b);
    else if(a<b)
    {
        a=(a-b);
        if(a>b)
        {
            a=(a+1);
        }
    }
    else
    {
        a=(a/b);
        a=(a+1);
    }
    if(a>b)
        if(a>b)
            a+1;
    if(a>b)
    {
        a=((a+1)*3);
        b=(b+(c+2));
    }
    while(1)
    {
        for(;;)
            if(a>b)
            {
                a=((a+1)*3);
                b=(b+(c+2));
                break;
            }
        if(a<b)
        {
            continue;
        }
        while(a==1)
        {
            a=(a+1);
        }
    }
    for(i=0;i<a;i=(i+1))
    {
        a=(a+1);
    }
    for(;;i=(i-1))
    {
        a=((a+1)*2);
    }
    return a+(b+num[def(a,num[0]-1)]);
}
