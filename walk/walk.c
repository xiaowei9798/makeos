#include "apilib.h"
// char *api_malloc(int size);
// void api_initmalloc(void);
// int api_openwin(char *buf,int xsiz,int ysiz,int col_inv,char *title);
// void api_boxfilwin(int win,int x0,int y0,int x1,int y1,int col_inv);
// void api_putstrwin(int win,int x,int y,int col,int len, char *str);
// int api_getkey(int mode); 
// void api_closewin(int win);
// void api_end(void);

void HariMain(void)
{
    char *buf;
    int win,x,y,i;
    api_initmalloc();
    buf=api_malloc(160*100);
    win=api_openwin(buf,160,100,-1,"walk");
    api_boxfilwin(win,4,24,155,95,0);
    x=76;
    y=56;

    api_putstrwin(win,x,y,3,1,"*");
    for(;;){
        i=api_getkey(1);
        api_putstrwin(win,x,y,0,1,"*");
        if(i=='8'&&y>24){ y-=8;}
        if(i=='5'&&y<80){ y+=8;}
        if(i=='6'&&x<148){ x+=8;}
        if(i=='4'&&x>4){ x-=8;}
        if(i==0x0a){ break;}
        api_putstrwin(win,x,y,3,1,"*");
    }
    api_closewin(win);
    api_end();
}

