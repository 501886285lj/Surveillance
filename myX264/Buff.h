#pragma once

extern "C"{
#include <inttypes.h>
#include "x264.h"
};
#include <opencv2/core/core.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>

#include <Windows.h>

using namespace cv;

class Buff
{
protected:
	int capacity;
	int head;
	int tail;
	HANDLE mutex_head,mutex_tail;

public:
	Buff(int capability);
	virtual ~Buff(void);

	bool isFull();
	bool isEmpty();

	int size();

	void lockHead();
	void lockTail();

	void unlockHead();
	void unlockTail();

	virtual int push(void *data) = 0;
	virtual void* top() = 0;
	virtual int pop() = 0;
};

class NaluBuff:public Buff{
	x264_nal_t *buff;

public:
	NaluBuff(int capacity = 20);
	~NaluBuff();

	int push(void *buff);
	void* top();
	int pop();
};

class FrameBuff:public Buff{
	Mat *buff;

public:
	FrameBuff(int capacity = 20);
	~FrameBuff();

	int push(void *cap);
	int push(Mat &m);
	void* top();
	int pop();

};