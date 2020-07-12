#include "driverlib.h"
#include <stdio.h>
#define MAXQSIZE 500

typedef char QElemtype;

typedef struct{
//    QElemtype *base;
	QElemtype data[MAXQSIZE];
    int front;
    int rear;
}SqQueue;

extern SqQueue q;
extern char buffer_JSONS_part[500];
//extern char buffer_JSONS_whole[500];	/**/
extern char buffer_JSON[200];	/*A complete JSON delivery data*/

void InitQueue(SqQueue *q);
bool EnQueue(SqQueue *q,QElemtype e);
bool DeQueue(SqQueue *q);
int QueueLength(SqQueue *q);
bool isEmpty(SqQueue *q);
bool traverseQueue( SqQueue *q);
