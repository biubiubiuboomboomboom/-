/* 定时器*/
#include"bootpack.h"

#define PIT_CTRL  0x0043
#define PIT_CNT0  0x0040

struct TIMERCTL timectl;


void init_pit(void)
{
	int i;
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timectl.count = 0;
	for (i = 0; i < 500; i++) {
		timectl.timers0[i].flags = 0; /* 没有使用 */
	}
	t = timer_alloc(); /* 取得一个 */
	t->timeout = 0xffffffff;
	t->flags = 2;
	t->next = 0; /* 末尾 */
	timectl.t0 = t; /* 因为现在只有哨兵，所以他就在最前面*/
	timectl.next = 0xffffffff; /* 因为只有哨兵，所以下一个超时时刻就是哨兵的时刻 */
}

void inthandler20(int *esp)
{
	char ts = 0;
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);
	timectl.count++;
	if(timectl.next > timectl.count)return;
	timer = timectl.t0;
	for (;;) {
		if (timer->timeout > timectl.count) {
			break;
		}
		timer->flags = 1;
		if(timer != task_timer){
		fifo32_put(timer->fifo, timer->data);
		}
		else{ ts =1; }
		
		timer = timer->next;
	}
	timectl.t0 = timer;
	timectl.next = timer->timeout;
	if (ts != 0) {
		task_switch();
	}
}

void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data)
{
	timer->fifo = fifo;
	timer->data = data;
}

void timer_free(struct TIMER *timer)
{
	timer->flags = 0; /* 未使用 */
}

void timer_settime(struct TIMER *timer, unsigned int timeout)
{
	int e;
	struct TIMER *t,*s;
	timer->timeout = timeout+timectl.count;
	timer->flags = 2;
	e = io_load_eflags();
	io_cli();
	t = timectl.t0;
	if (timer->timeout <= t->timeout) {
	/* 插入最前面的情况 */
		timectl.t0 = timer;
		timer->next = t; /* 下面是设定t */
		timectl.next = timer->timeout;
		io_store_eflags(e);
		return;
	}
	for (;;) {
		s = t;
		t = t->next;
		if (timer->timeout <= t->timeout) {
		/* 插入s和t之间的情况 */
			s->next = timer; /* s下一个是timer */
			timer->next = t; /* timer的下一个是t */
			io_store_eflags(e);
			return;
		}
	}
}


struct TIMER *timer_alloc(void)
{
	int i;
	for (i = 0; i < 500; i++) {
		if (timectl.timers0[i].flags == 0) {
			timectl.timers0[i].flags = 1;
			return &timectl.timers0[i];
		}
	}
	return 0; 
}
