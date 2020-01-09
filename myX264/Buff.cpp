#include "Buff.h"


Buff::Buff(int capacity):capacity(capacity)
{
	head = 0;
	tail = 0;
	
	mutex_head = CreateMutex(nullptr,false,nullptr);
	mutex_tail = CreateMutex(nullptr,false,nullptr);
}


Buff::~Buff(void)
{
	CloseHandle(mutex_head);
	CloseHandle(mutex_tail);
}

bool Buff::isFull(){
	bool ret = false;

// 	WaitForSingleObject(mutex_head,INFINITE);
// 	WaitForSingleObject(mutex_tail,INFINITE);

	if( ((tail+1) % capacity) == head)
		ret =  true;
	else
		ret = false;

// 	ReleaseMutex(mutex_tail);
// 	ReleaseMutex(mutex_head);

	return ret;
}

bool Buff::isEmpty(){
	bool ret = false;

// 	WaitForSingleObject(mutex_head,INFINITE);
// 	WaitForSingleObject(mutex_tail,INFINITE);

	if(tail == head)
		ret =  true;
	else
		ret =  false;

// 	ReleaseMutex(mutex_tail);
// 	ReleaseMutex(mutex_head);

	return ret;
}

int Buff::size(){
	int ret = 0;

// 	WaitForSingleObject(mutex_head,INFINITE);
// 	WaitForSingleObject(mutex_tail,INFINITE);

	if(tail < head)
		ret =  tail+capacity-head;
	else
		ret =  tail-head;

// 	ReleaseMutex(mutex_tail);
// 	ReleaseMutex(mutex_head);

	return ret;
}

void Buff::lockHead(){
	WaitForSingleObject(mutex_head,INFINITE);
}

void Buff::lockTail(){
	WaitForSingleObject(mutex_tail,INFINITE);
}

void Buff::unlockHead(){
	ReleaseMutex(mutex_head);
}

void Buff::unlockTail(){
	ReleaseMutex(mutex_tail);
}
