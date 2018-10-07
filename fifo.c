/* FIFO */

#include "bootpack.h"

#define FLAGS_OVERRUN		0x0001

void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task)
/* FIFO�������ĳ�ʼ��*/
{
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size; /*��*/
	fifo->flags = 0;
	fifo->p = 0; /*д��λ��*/
	fifo->q = 0; /*��ȡλ��*/
	fifo->task = task; /*������д��ʱ��Ҫ���ѵ�����*/
	return;
}

int fifo32_put(struct FIFO32 *fifo, int data)
/*��FIFOд�����ݲ��ۻ�����*/
{
	if (fifo->free == 0) {
		/*û�п���ռ䣬���*/
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->p] = data;
	fifo->p++;
	if (fifo->p == fifo->size) {
		fifo->p = 0;
	}
	fifo->free--;
	if (fifo->task != 0) {
		if (fifo->task->flags != 2) { /*�������������״̬*/
			task_run(fifo->task, -1, 0); /*��������*/
		}
	}
	return 0;
}

int fifo32_get(struct FIFO32 *fifo)
/*��FIFOȡ��һ������*/
{
	int data;
	if (fifo->free == fifo->size) {
	/*��������Ϊ�յ�����·���-1*/
		return -1;
	}
	data = fifo->buf[fifo->q];
	fifo->q++;
	if (fifo->q == fifo->size) {
		fifo->q = 0;
	}
	fifo->free++;
	return data;
}

int fifo32_status(struct FIFO32 *fifo)
/*�����Ѿ��洢�˶�������*/
{
	return fifo->size - fifo->free;
}