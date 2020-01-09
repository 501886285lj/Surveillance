#include "Buff.h"
#include <stdlib.h>
#include <string.h>
#include <iostream>

using namespace std;

NaluBuff::NaluBuff(int capability):Buff(capability){
	buff = new x264_nal_t[capability];
}

NaluBuff::~NaluBuff(void)
{
	delete[] buff;
}

int NaluBuff::push(void *data){
	if(isFull())
		return -1;

	x264_nal_t* nalu = (x264_nal_t*)data;

//	WaitForSingleObject(mutex_tail,INFINITE);

	buff[tail] = *nalu;	//复制

	buff[tail].p_payload = (uint8_t*)malloc(nalu->i_payload);	//alloc

	memcpy(buff[tail].p_payload,nalu->p_payload,nalu->i_payload);			//copy data

	tail = (tail+1)%capacity;

//	ReleaseMutex(mutex_tail);

	return 0;
}

void* NaluBuff::top(){
	if(isEmpty())
		return nullptr;

//	WaitForSingleObject(mutex_head,INFINITE);

	x264_nal_t* nalu = buff+head;

//	ReleaseMutex(mutex_head);

	return nalu;
}

int NaluBuff::pop(){
	if(isEmpty())
		return -1;

//	WaitForSingleObject(mutex_head,INFINITE);

//	free(buff[head].p_payload);	//释放nalu的data

	head = (head+1)%capacity;	//更新头

//	ReleaseMutex(mutex_head);

	return 0;
}