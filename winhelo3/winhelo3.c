#include "apilib.h"

void HariMain(void)
{
    char *buf;
    int win;

    api_initmalloc();
    buf=api_malloc(150*50);  //winhelo3采用的是malloc方法，其与_alloca方法生成的.hrb文件的字节大小差距不大

    win = api_openwin(buf, 150, 50, -1, "hello");
    api_boxfilwin(win, 8, 36, 141, 43, 6);
    api_putstrwin(win, 28, 28, 0, 12, "hello, world");
    for(;;)
    {
        if(api_getkey(1)==0x0a)
        {
            break;
        }
    }
    api_end();
}



