/* bootpack */

#include "bootpack.h"
#include <stdio.h>
#include <string.h>

#define KEYCMD_LED 0xed
void keywin_off(struct SHEET *key_win);
void keywin_on(struct SHEET *key_win);
void close_console(struct SHEET *sht);
void close_constack(struct TASK *task);

void HariMain(void)
{
	struct BOOTINFO *binfo = (struct BOOTINFO *)ADR_BOOTINFO;
	struct SHTCTL *shtctl;
	char s[40];
	struct FIFO32 fifo, keycmd;
	int fifobuf[128], keycmd_buf[32] /* , *cons_fifo[2] */;
	int mx, my, i /* ,cursor_x, cursor_c*/;
	unsigned int memtotal;
	struct MOUSE_DEC mdec;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	unsigned char *buf_back, buf_mouse[256] /* ,*buf_win , *buf_cons[2] */;
	struct SHEET *sht_back, *sht_mouse /* ,*sht_win , *sht_cons[2]  */;
	struct TASK *task_a, *task  /* , *task_cons[2] */;
	// struct TIMER *timer;
	// struct CONSOLE *cons;
	*((int *)0x0fec) = (int)&fifo;
	int j, x, y;
	int mmx = -1, mmy = -1, mmx2 = 0;
	int new_mx = -1, new_my = 0, new_wx = 0x7fffffff, new_wy = 0;
	struct SHEET *sht = 0, *key_win, *sht2;
	
	int *fat;
	unsigned char *nihongo;
	struct FILEINFO *finfo;
	extern char hankaku[4096];

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
		0, '~', '!', '@', '#', '$', '%', '^','&', '*', '(', ')', '-', '+', 0x08, 0,
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
	task_a = task_init(memman);  //在任务初始化时生成了两个任务，task  idle 
	fifo.task = task_a;
	task_run(task_a, 1, 2);   //level, priority
	*((int *)0x0fe4) = (int)shtctl;
	task->langmode=0;

	/* sht_back */
	sht_back = sheet_alloc(shtctl);
	buf_back = (unsigned char *)memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1); /* 透明色 */
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	/* sht_cons */
	key_win = open_console(shtctl, memtotal);

	// for (i = 0; i < 2; i++)
	// {
	// 	sht_cons[i] = sheet_alloc(shtctl);
	// 	buf_cons[i] = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	// 	sheet_setbuf(sht_cons[i], buf_cons[i], 256, 165, -1); /* 透明色 */
	// 	make_window8(buf_cons[i], 256, 165, "console", 0);
	// 	make_textbox8(sht_cons[i], 8, 28, 240, 128, COL8_000000);
	// 	task_cons[i] = task_alloc();
	// 	task_cons[i]->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	// 	task_cons[i]->tss.eip = (int)&console_task;
	// 	task_cons[i]->tss.es = 1 * 8;
	// 	task_cons[i]->tss.cs = 2 * 8;
	// 	task_cons[i]->tss.ss = 1 * 8;
	// 	task_cons[i]->tss.ds = 1 * 8;
	// 	task_cons[i]->tss.fs = 1 * 8;
	// 	task_cons[i]->tss.gs = 1 * 8;
	// 	*((int *)(task_cons[i]->tss.esp + 4)) = (int)sht_cons[i];
	// 	task_run(task_cons[i], 2, 2); /* level=2, priority=2 */
	// 	*((int *)(task_cons[i]->tss.esp + 8)) = memtotal;
	// 	sht_cons[i]->task = task_cons[i];
	// 	sht_cons[i]->flags |= 0x20;

	// 	//将命令行的初始化放在前面，以免在还没初始化的情况下向命令行传送数据
	// 	cons_fifo[i] = (int *)memman_alloc_4k(memman, 128 * 4);
	// 	fifo32_init(&task_cons[i]->fifo, 128, cons_fifo[i], task_cons[i]);
	// }

	/* sht_mouse */
	sht_mouse = sheet_alloc(shtctl);
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99); //鼠标透明色号99
	init_mouse_cursor8(buf_mouse, 99);				//鼠标背景色号99
	mx = (binfo->scrnx - 16) / 2;					/* 画面中心位置计算 */
	my = (binfo->scrny - 28 - 16) / 2;

	sheet_slide(sht_back, 0, 0);
	// sheet_slide(sht_cons[1], 52, 6);
	sheet_slide(key_win, 32, 4);
	sheet_slide(sht_mouse, mx, my);
	sheet_updown(sht_back, 0);
	// sheet_updown(sht_cons[1], 1);
	sheet_updown(key_win, 1);
	sheet_updown(sht_mouse, 2);
	// key_win = sht_cons[0];
	keywin_on(key_win);

	// sprintf(s, "(%3d, %3d)", mx, my);
	// putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
	// sprintf(s, "memory %dMB   free : %dKB",
	// 		memtotal / (1024 * 1024), memman_total(memman) / 1024);
	// putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	//载入nihongo.fnt字符库
	nihongo=(unsigned char *) memman_alloc_4k(memman,16*256+32*94*47);  
	fat=(int *) memman_alloc_4k(memman,4*2880);
	file_readfat(fat,(unsigned char *)(ADR_DISKIMG + 0x000200));
	finfo=file_search("nihongo.fnt",(struct FILEINFO *)(ADR_DISKIMG + 0x002600),224);
	if(finfo !=0){
		i=finfo->size;
		nihongo=file_loadfile2(finfo->clustno,&i,fat); //将压缩文件载入内存
	}else{
		for(i=0;i<16*256;i++){
			nihongo[i]=hankaku[i];
		}
		for(i=16*256;i<16*256+32*94*47;i++){
			nihongo[i]=0xff;
		}
	}
	*((int *)0x0fe8)=(int) nihongo;
	memman_free_4k(memman,(int)fat,4*2880);

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
			if (new_mx >= 0)
			{
				io_sti();
				sheet_slide(sht_mouse, new_mx, new_my);
				new_mx = -1;
			}
			else if (new_wx != 0x7fffffff)
			{
				io_sti();
				sheet_slide(sht, new_wx, new_wy);
				new_wx = 0x07fffffff;
			}
			else
			{
				task_sleep(task_a);
				io_sti();
			}
		}
		else
		{
			i = fifo32_get(&fifo);
			io_sti();
			if (key_win != 0 && key_win->flags == 0)
			{
				if (shtctl->top == 1)
				{
					key_win = 0;
				}
				else
				{
					key_win = shtctl->sheets[shtctl->top - 1];
					keywin_on(key_win);
				}
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
				if (s[0] != 0 && key_win != 0)
				{ /* 一般字符 、退格键、回车键  因为去掉了task_a,故可一起通过命令行窗口来执行了*/
					fifo32_put(&key_win->task->fifo, s[0] + 256);
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
				if (i == 256 + 0x0f && key_win != 0)
				{ /* Tab */
					keywin_off(key_win);
					j = key_win->height - 1;
					if (j == 0)
					{
						j = shtctl->top - 1;
					}
					key_win = shtctl->sheets[j];
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
				if (i == 256 + 0x3b && key_shift != 0 && key_win != 0)
				{
					task = key_win->task; //用shift+F1强制结束应用程序时，以当前输入窗口为对象
					if (task != 0 && task->tss.ss0 != 0)
					{
						cons_putstr0(task->cons, "\nBreak(key):\n");
						io_cli();
						task->tss.eax = (int)&(task->tss.esp0);
						task->tss.eip = (int)asm_end_app;
						io_sti();
						task_run(task,-1,0);
					}
				}
				if (i == 256 + 0x3c && key_shift != 0)
				{ //shift+F2 打开一个新的命令窗口
					if (key_win != 0)
					{
						keywin_off(key_win);
					}
					key_win = open_console(shtctl, memtotal);
					sheet_slide(key_win, 32, 4);
					sheet_updown(key_win, shtctl->top);
					// keywin_off(key_win);
					// key_win=sht_cons[1];
					keywin_on(key_win);
				}
				if (i == 256 + 0x57)
				{ /* F11 将最下面的图层置换到最上层（去掉鼠标和背景层）*/
					sheet_updown(shtctl->sheets[1], shtctl->top - 1);
				}
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
					// sheet_slide(sht_mouse, mx, my);
					new_mx = mx;
					new_my = my;
					if ((mdec.btn & 0x01) != 0)
					{ //按下鼠标左键
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
										if (sht != key_win)
										{
											keywin_off(key_win);
											key_win = sht;
											keywin_on(key_win);
										}
										if (3 <= x && x < sht->bxsize - 3 && 3 <= y && y < 21)
										{ //如果鼠标点击区域位于标题栏部分
											mmx = mx;
											mmy = my;
											mmx2 = sht->vx0; //记录移动前图层的vx0
											new_wy = sht->vy0;
										}
										if (sht->bxsize - 21 <= x && x < sht->bxsize - 5 && 5 <= y && y < 19)
										{
											if ((sht->flags & 0x10) != 0)
											{
												// cons = (struct CONSOLE *)*((int *)0x0fec);
												task = sht->task; //用鼠标点击‘X’时，以被点击的窗口为对象
												cons_putstr0(task->cons, "\nBreak(mouse) :\n");
												io_cli();
												task->tss.eax = (int)&(task->tss.esp0);
												task->tss.eip = (int)asm_end_app;
												io_sti();
												task_run(task,-1,0);
											}
											else
											{
												task = sht->task;
												sheet_updown(sht,-1);   //隐藏命令行窗口
												keywin_off(key_win);
												key_win=shtctl->sheets[shtctl->top-1];
												keywin_on(key_win);
												io_cli();
												fifo32_put(&task->fifo, 4);
												io_sti();
											}
										}
										break;
									}
								}
							}
						}
						else
						{ //mmx不小于1了，说明处于窗口移动模式
							x = mx - mmx;
							y = my - mmy;
							// sheet_slide(sht, (mmx2 + x + 2) & ~3, sht->vy0 + y);
							new_wx = (mmx2 + x + 2) & ~3;
							new_wy = new_wy + y;
							mmy = my;
						}
					}
					else
					{ //没有按下鼠标左键，切换到一般模式，立即移动窗口，因为窗口的坐标可能为负数，故选取0x7fffffff这个不可能出现的值
						mmx = -1;
						if (new_wx != 0x7fffffff)
						{
							sheet_slide(sht, new_wx, new_wy);
							new_wx = 0x7fffffff;
						}
					}
				}
			}
			else if (768 <= i && i <= 1023)
			{
				close_console(shtctl->sheets0 + (i - 768));   //初始化图层的时候，图层最大个数设定了256个
			}
			else if (1024 <= i && i <= 2023)
			{
				close_constack(taskctl->tasks0 + (i - 1024));  //初始化任务的时候，任务最大个数设定了1000个
			}
			else if(2024<=i && i<=2279)
			{
				sht2=shtctl->sheets0+(i-2024);
				memman_free_4k(memman,(int) sht2->buf,256*165);
				sheet_free(sht2);
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

void keywin_off(struct SHEET *key_win /*, struct SHEET *sht_win, int cur_c, int cur_x */)
{
	change_wtitle8(key_win, 0);
	if ((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 3); //命令行窗口光标OFF，0x20用于判别窗口是否为命令行窗口
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

void keywin_on(struct SHEET *key_win)
{
	change_wtitle8(key_win, 1);
	if ((key_win->flags & 0x20) != 0)
	{
		fifo32_put(&key_win->task->fifo, 2); //命令行窗口光标ON
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

struct SHEET *open_console(struct SHTCTL *shtctl, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SHEET *sht = sheet_alloc(shtctl);
	unsigned char *buf = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht, buf, 256, 165, -1);
	make_window8(buf, 256, 165, "console", 0);
	make_textbox8(sht, 8, 28, 240, 128, COL8_000000);
	sht->task = open_constask(sht, memtotal);
	sht->flags |= 0x20;
	return sht;
}

struct TASK *open_constask(struct SHEET *sht, unsigned int memtotal)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct TASK *task = task_alloc();
	int *cons_fifo = (int *)memman_alloc_4k(memman, 128 * 4); //命令行的初始化放在前面，防止在没有初始化的情况下向命令行传值
	task->cons_stack = memman_alloc_4k(memman, 64 * 1024);	//将创建的命令行窗口的栈地址保存起来
	task->tss.esp = task->cons_stack + 64 * 1024 - 12;
	task->tss.eip = (int)&console_task;
	task->tss.es = 1 * 8;
	task->tss.cs = 2 * 8;
	task->tss.ss = 1 * 8;
	task->tss.ds = 1 * 8;
	task->tss.fs = 1 * 8;
	task->tss.gs = 1 * 8;
	*((int *)(task->tss.esp + 4)) = (int)sht;
	*((int *)(task->tss.eip + 8)) = memtotal;
	task_run(task, 2, 2);
	fifo32_init(&task->fifo, 128, cons_fifo, task);
	return task;
}

void close_console(struct SHEET *sht)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct TASK *task = sht->task;
	memman_free_4k(memman, (int)sht->buf, 256 * 165);
	sheet_free(sht);
	close_constack(task); //将关闭任务和关闭图层分为两个函数来写
	return;
}

void close_constack(struct TASK *task)
{
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	task_sleep(task);
	memman_free_4k(memman, task->cons_stack, 64 * 1024); //task->cons_stack 保存着命令行窗口创建的栈地址
	memman_free_4k(memman, (int)task->fifo.buf, 128 * 4);
	task->flags = 0;
	return;
}


