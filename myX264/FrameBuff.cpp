#include "Buff.h"


FrameBuff::FrameBuff(int capability):Buff(capability){
	buff = new Mat[capability];
}

FrameBuff::~FrameBuff(){
	delete[] buff;
}

int FrameBuff::push(void *cap){
	if(isFull())
		return -1;

//	WaitForSingleObject(mutex_tail,INFINITE);

	((VideoCapture*)cap)->read(buff[tail]);

	tail = (tail+1)%capacity;

//	ReleaseMutex(mutex_tail);

	return 0;
}

void* FrameBuff::top(){
	if(isEmpty())
		return nullptr;

//	WaitForSingleObject(mutex_head,INFINITE);

	Mat *m = buff+head;

//	ReleaseMutex(mutex_head);

	return m;
}

int FrameBuff::pop(){
	if(isEmpty())
		return -1;

//	WaitForSingleObject(mutex_head,INFINITE);
	head = (head+1)%capacity;	//¸üÐÂÍ·
//	ReleaseMutex(mutex_head);

	return 0;
}

int FrameBuff::push(Mat &m){
	if(isFull())
		return -1;

	//	WaitForSingleObject(mutex_tail,INFINITE);

	buff[tail] = m;

	tail = (tail+1)%capacity;

	//	ReleaseMutex(mutex_tail);

	return 0;
}