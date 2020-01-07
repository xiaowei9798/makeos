/* bootpack */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED 0xed
void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	struct SHTCTL *shtctl;
	char s[40];
	struct FIFO32 fifo, keycmd;
	int fifobuf[128], keycmd_buf[32], *cons_fifo[2];
	int mx, my, i /* ,cursor_x, cursor_c*/;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	unsigned char *buf_back, buf_mouse[256] /* ,*buf_win */, *buf_cons[2];
	struct SHEET *sht_back, *sht_mouse /* ,*sht_win */, *sht_cons[2];
	struct TASK  *task_a, *task_cons[2], *task;
	// struct TIMER *timer;
	// struct CONSOLE *cons;

	int j, x, y;
	int mmx = -1, mmy = -1;
	struct SHEET *sht = 0, *key_win;

	static char keytable0[0x80] = {
		0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '^', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '@', '[', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', ':', 0, 0, ']', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0x5c, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x5c, 0, 0};
	static char keytable1[0x80] = {
		0, 0, '!', 0x22, '#', '$', '%', '&', 0x27, '(', ')', '~', '=', '~', 0x08, 0,
		'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '`', '{', 0x0a, 0, 'A', 'S',
		'D', 'F', 'G', 'H', 'J', 'K', 'L', '+', '*', 0, 0, '}', 'Z', 'X', 'C', 'V',
		'B', 'N', 'M', '<', '>', '?', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
		'2', '3', '0', '.', 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, '_', 0, 0, 0, 0, 0, 0, 0, 0, 0, '|', 0, 0};
	int key_shift = 0, key_leds = (binfo->leds >> 4) & 7, keycmd_wait = -1;

	init_gdtidt();
	init_pic();
	io_sti(); /* GDT/IDT/PIC初期化 */
	fifo32_init(&fifo, 128, fifobuf, 0);
	init_pit();
	init_keyboard(&fifo, 256);
	enable_mouse(&fifo, 512, &mdec);
	io_out8(PIC0_IMR, 0xf8); /* PIT定时器和PIC1键盘許可(11111000) */
	io_out8(PIC1_IMR, 0xef); /* 鼠标許可(11101111) */
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	memtotal = memtest(0x00400000, 0xbfffffff);
	memman_init(memman);
	memman_free(memman, 0x00001000, 0x0009e000); /* 0x00001000 - 0x0009efff */
	memman_free(memman, 0x00400000, memtotal - 0x00400000);

	init_palette();
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 2);
	*((int *)0x0fe4) = (int)shtctl;
	

	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 透明色 */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_cons */
	for(i=0;i<2;i++)
	{
		sht_cons[i] = sheet_alloc(shtctl);
		buf_cons[i] = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
		sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1); /* 透明色 */
		make_window8(buf_cons[i], 256, 165, "console", 0);
		make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
		task_cons[i] = task_alloc();
		task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
		task_cons[i]->tss.eip = (int)&console_task;
		task_cons[i]->tss.es = 1 * 8;
		task_cons[i]->tss.cs = 2 * 8;
		task_cons[i]->tss.ss = 1 * 8;
		task_cons[i]->tss.ds = 1 * 8;
		task_cons[i]->tss.fs = 1 * 8;
		task_cons[i]->tss.gs = 1 * 8;
		*((int *)(task_cons[i]->tss.esp + 4)) = (int)sht_cons[i];
		task_run(task_cons[i], 2, 2); /* level=2, priority=2 */
		*((int *)(task_cons[i]->tss.esp + 8)) = memtotal;
		sht_cons[i]->task = task_cons[i];
		sht_cons[i]->flags |= 0x20;

		//将命令行的初始化放在前面，以免在还没初始化的情况下向命令行传送数据
		cons_fifo[i]=(int *) memman_alloc_4k(memman,128*4);
		fifo32_init(&task_cons[i]->fifo,128,cons_fifo[i],task_cons[i]);
	}

	/* sht_win */
	// sht_win = sheet_alloc(shtctl);
	// buf_win = (unsigned char *)memman_alloc_4k(memman, 160 * 52);
	// sheet_setbuf(sht_win, buf_win, 144, 52, -1); /* -1表示没有透明色 */
	// make_window8(buf_win, 144, 52, "task_a", 1);
	// make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	// cursor_x = 8;
	// cursor_c = COL8_FFFFFF;
	// timer = timer_alloc();
	// timer_init(timer, &fifo, 1);
	// timer_settime(timer, 50);

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);    //鼠标透明色号99
	init_mouse_cursor8(buf_mouse, 99);                 //鼠标背景色号99
	mx = (binfo->scrnx - 16) / 2; /* 画面中心位置计算 */
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back, 0, 0);
	sheet_slide(sht_cons[1], 52, 6);
	sheet_slide(sht_cons[0],8,2);
	// sheet_slide(sht_win, 64, 56);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);
	sheet_updown(sht_cons[1], 1);
	sheet_updown(sht_cons[0],2);
	// sheet_updown(sht_win, 3);
	sheet_updown(sht_mouse, 3);

	// key_win = sht_win;
	key_win=sht_cons[0];
	keywin_on(key_win);

	// sprintf(s, "(%3d, %3d)", mx, my);
	// putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	// sprintf(s, "memory %dMB   free : %dKB",
	// 		memtotal / (1024 * 1024), memman_total(memman) / 1024);
	// putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	/* 进行初始设定，为了避免和键盘当前状态冲突 */
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	for (;;)
	{
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0)
		{
			/* 如果键盘控制器缓冲区里面有数据，并且处于通常状态，发送它 */
			keycmd_wait = fifo32_get(&keycmd);
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		io_cli();
		if (fifo32_status(&fifo) == 0)
		{
			task_sleep(task_a);
			io_sti();
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win->flags == 0)
			{
				key_win = shtctl->sheets[shtctl->top - 1];
				keywin_on(key_win);
			}
			if (256 <= i && i <= 511)
			{ /* 键盘数据 */
				// sprintf(s, "%02X", i - 256);
				// putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				if (i < 0x80 + 256)
				{ /* 将按键编码成字符编码 */
					if (key_shift == 0)
					{
						s[0] = keytable0[i - 256];
					}
					else
					{
						s[0] = keytable1[i - 256];
					}
				}
				else
				{
					s[0] = 0;
				}
				if ('A' <= s[0] && s[0] <= 'Z')
				{ /* 如果输入的是英文文字 */
					if (((key_leds & 4) == 0 && key_shift == 0) ||
						((key_leds & 4) != 0 && key_shift != 0))
					{
						s[0] += 0x20; /* 大小写文字変換 */
					}
				}
				if (s[0] != 0)
				{ /* 一般字符 、退格键、回车键  因为去掉了task_a,故可一起通过命令行窗口来执行了*/
					fifo32_put(&key_win->task->fifo,s[0]+256);
				}
				// if (i == 256 + 0x0e)
				// { /* 退格键 */
				// 	if (key_win == sht_win)
				// 	{
				// 		if (cursor_x > 8)
				// 		{
				// 			putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
				// 			cursor_x -= 8;
				// 		}
				// 	}
				// 	else
				// 	{
				// 		fifo32_put(&key_win->task->fifo, 8 + 256);
				// 	}
				// }
				// if (i == 256 + 0x1c)
				// { /* 回车键 */
				// 	if (key_win != sht_win)
				// 	{
				// 		fifo32_put(&key_win->task->fifo, 10 + 256);
				// 	}
				// }
				if (i == 256 + 0x0f)
				{ /* Tab */
					keywin_off(key_win);
					j=key_win->height-1;
					if(j==0){
						j=shtctl->top-1;
					}
					key_win=shtctl->sheets[j];
					keywin_on(key_win);
				}
				if (i == 256 + 0x2a)
				{ /* 左 ON */
					key_shift |= 1;
				}
				if (i == 256 + 0x36)
				{ /* 右 ON */
					key_shift |= 2;
				}
				if (i == 256 + 0xaa)
				{ /* 左 OFF */
					key_shift &= ~1;
				}
				if (i == 256 + 0xb6)
				{ /* 右 OFF */
					key_shift &= ~2;
				}
				if (i == 256 + 0x3a)
				{ /* CapsLock */
					key_leds ^= 4;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x45)
				{ /* NumLock */
					key_leds ^= 2;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x46)
				{ /* ScrollLock */
					key_leds ^= 1;
					fifo32_put(&keycmd, KEYCMD_LED);
					fifo32_put(&keycmd, key_leds);
				}
				if (i == 256 + 0x3b && key_shift != 0 )
				{
					task=key_win->task;   //用F1强制结束应用程序时，以当前输入窗口为对象
					if(task!=0&&task->tss.ss0!=0)
					{
						cons_putstr0(task->cons, "\nBreak(key):\n");
						io_cli();
						task->tss.eax = (int)&(task->tss.esp0);
						task->tss.eip = (int)asm_end_app;
						io_sti();
					}
				}
				if (i == 256 + 0x57 )
				{    /* F11 将最下面的图层置换到最上层（去掉鼠标和背景层）*/
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
				/* 重新显示光标 */
				// if (cursor_c >= 0)
				// {
				// 	boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				// }
				// sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				if (i == 256 + 0xfa)
				{ /* 键盘成功接收到数据 */
					keycmd_wait = -1;
				}
				if (i == 256 + 0xfe)
				{ /* 键盘没有成功接收到数据 */
					wait_KBC_sendready();
					io_out8(PORT_KEYDAT, keycmd_wait);
				}
			}
			else if (512 <= i && i <= 767)
			{ /* 鼠标数据 */
				if (mouse_decode(&mdec, i - 512) != 0)
				{
					// 		sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					// 		if ((mdec.btn & 0x01) != 0)
					// 		{
					// 			s[1] = 'L';
					// 		}
					// 		if ((mdec.btn & 0x02) != 0)
					// 		{
					// 			s[3] = 'R';
					// 		}
					// 		if ((mdec.btn & 0x04) != 0)
					// 		{
					// 			s[2] = 'C';
					// 		}
					// 		putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);
					// 		/* 移动鼠标 */
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0)
					{
						mx = 0;
					}
					if (my < 0)
					{
						my = 0;
					}
					if (mx > binfo->scrnx - 1)
					{
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1)
					{
						my = binfo->scrny - 1;
					}
					// sprintf(s, "(%3d, %3d)", mx, my);
					// putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					sheet_slide(sht_mouse, mx, my);
					if ((mdec.btn & 0x01) != 0)
					{
						/* 点击鼠标移动sht_win图层 */
						// sheet_slide(sht_win, mx - 80, my - 8);
						if (mmx < 0)
						{
							for (j = shtctl->top - 1; j > 0; j--)
							{
								sht = shtctl->sheets[j];
								x = mx - sht->vx0;
								y = my - sht->vy0;
								if (0 <= x && x < sht->bxsize && 0 <= y && y < sht->bysize)
								{
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv)
									{
										sheet_updown(sht, shtctl->top - 1);
										if(sht!=key_win){
											keywin_off(key_win);
											key_win=sht;
											keywin_on(key_win);
										}
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21)
										{
											mmx = mx;
											mmy = my;
										}
										if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19)
										{
											if ((sht->flags & 0x10) != 0)
											{
												// cons = (struct CONSOLE *)*((int *)0x0fec);
												task=sht->task;  //用鼠标点击‘X’时，以被点击的窗口为对象
												cons_putstr0(task->cons, "\nBreak(mouse) :\n");
												io_cli();
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int)asm_end_app;
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						else
						{
							x = mx - mmx;
							y = my - mmy;
							sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
							mmx = mx;
							mmy = my;
						}
					}
					else
					{
						mmx = -1;
					}
				}
			}
			// else if (i <= 1)
			// { /* 光标用定时器 */
			// 	if (i != 0)
			// 	{
			// 		timer_init(timer, &fifo, 0); /* 初始定时器0 */
			// 		if (cursor_c >= 0)
			// 		{
			// 			cursor_c = COL8_000000;
			// 		}
			// 	}
			// 	else
			// 	{
			// 		timer_init(timer, &fifo, 1); /* 初始定时器1 */
			// 		if (cursor_c >= 0)
			// 		{
			// 			cursor_c = COL8_FFFFFF;
			// 		}
			// 	}
			// 	timer_settime(timer, 50); //光标定时器设置0.5S
			// 	if (cursor_c >= 0)
			// 	{
			// 		boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
			// 	}
			// 	sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			// }
		}
	}
}

void keywin_off(struct SHEET *key_win  /*, struct SHEET *sht_win, int cur_c, int cur_x */)
{
	change_wtitle8(key_win, 0);
	if((key_win->flags&0x20)!=0){
		fifo32_put(&key_win->task->fifo,3);   //命令行窗口光标OFF，0x20用于判别窗口是否为命令行窗口
	}
	// if (key_win == sht_win)
	// {
	// 	cur_c = -1;
	// 	boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cur_x, 28,cur_x+7,43 );
	// }else{
	// 	if((key_win->flags & 0x20)!=0){
	// 		fifo32_put(&key_win->task->fifo,3);
	// 	}
	// }
	return;
}

void keywin_on(struct SHEET *key_win  )
{
	change_wtitle8(key_win, 1);
	if((key_win->flags&0x20)!=0){
		fifo32_put(&key_win->task->fifo,2);   //命令行窗口光标ON
	}
	// if (key_win == sht_win)
	// {
	// 	cur_c = COL8_000000;
	// }
	// else
	// {
	// 	if ((key_win->flags & 0x20) != 0)
	// 	{
	// 		fifo32_put(&key_win->task->fifo, 2);
	// 	}
	// }
	return;
}

