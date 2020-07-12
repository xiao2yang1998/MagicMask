#include "queue.h"
#include "hardware.h"
SqQueue q;
char buffer_JSONS_part[500];
//char buffer_JSONS_whole[500];	/**/
char buffer_JSON[200];	/*A complete JSON delivery data*/
static int i = 0;

void InitQueue(SqQueue *q)
{
    q->rear = q->front = 0;
}

bool EnQueue(SqQueue *q,QElemtype e)
{
    //Insert into the end of the queue
    long tmp = rt_hw_interrupt_disable();
    if((q->rear+1)%MAXQSIZE==q->front){
        //If the queue is full, continue to join the queue and overwrite the previous data.
        q->front =(q->front+1)%MAXQSIZE;
    }
    q->data[q->rear]=e;
    q->rear=(q->rear+1)%MAXQSIZE;
    rt_hw_interrupt_enable(tmp);
		//printf("|%c|\n",e);
    return true;
}

bool DeQueue(SqQueue *q)
{
    long tmp = rt_hw_interrupt_disable();
    if(q->front==q->rear)
        return false;
//    printf("%d",q->data[q->front]);
    q->data[q->front] = '0';
    q->front =(q->front+1)%MAXQSIZE;
    rt_hw_interrupt_enable(tmp);
    return true;
}

int QueueLength(SqQueue *q)
{
    return (q->rear-q->front+MAXQSIZE)%MAXQSIZE;
}

bool isEmpty(SqQueue *q)
{
	if(q->rear == q->front){
		return true;
        
	}
    for(i = 0; i < MAXQSIZE; i++ ){
        if(q->data[i] != '0'){
            break;
        }
    }
    if(i == MAXQSIZE){
        return true;
    }
    return false;
}

bool traverseQueue( SqQueue *q)
{
    if(isEmpty(q)){
        return false;
    }
    while(q->front != q->rear){
        printf("%c",q->data[q->front]);
        DeQueue(q);
        q->front = (q->front + 1)%MAXQSIZE;
    }
    return true;
}

