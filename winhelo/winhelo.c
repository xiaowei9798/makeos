#include "apilib.h"

// char buf[150 * 50];  之前采用的方法，在函数外部声明的变量，会被解释成DB和RESB；函数内部声明的非static变量会从栈中分配空间

void HariMain(void)
{
    int win;
    char buf[150*50];  //当增加了__alloca函数后，采用内部声明变量的方法，此时注意修改makefile函数中栈空间大小的分配

    win = api_openwin(buf, 150, 50, -1, "hello");
    for(;;)
    {
        if(api_getkey(1)==0x0a)
        {
            break;
        }
    }
    api_end();
}

