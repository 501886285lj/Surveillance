#pragma once
#include "inttypes.h"
extern "C"{
#include "x264.h"
};

class Fu
{
private:
	struct Indicator{				//fu 的 indicator
		char TYPE:5;
		char NRI:2;
		char F:1;
	} *indicator;

	struct Header{					//fu 的 header
		char TYPE:5;
		char R:1;
		char E:1;
		char S:1;
	} *header;
	int MaxLen;						//fu的最大长度
	char *nalu;				//用于拆片的nalu
	int nalu_len;					//用于拆片的nalu的长度

public:
	char *data;					//fu 的负载
	int fu_len;						//fu实际的长度		最后一fu会变，前面n-1个fu的长度都是manlen0

	~Fu(void);

	Fu(int MaxLen = 1400);

	void load(char *nalu,int length);

	void split(int fuNo);
	
	int getFuNums();

};

