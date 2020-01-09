#include "Fu.h"


Fu::~Fu(void)
{
}

Fu::Fu(int MaxLen):MaxLen(MaxLen){
	data = new char[MaxLen];
}

void Fu::load(char *nalu,int length){
	this->nalu = nalu;
	this->nalu_len = length;

	//设置indicator,data第一字节
	indicator = (Indicator *)data;
	indicator->TYPE = 28;
	indicator->NRI = (nalu[0] & 0x60) >> 5;
	indicator->F = (nalu[0] & 0x80) >> 7;

	//设置header,默认非头片非尾片，data第二字节
	header = (Header *)(data + 1);
	header->TYPE = nalu[0] & 0x1f;
	header->R = 0;
	header->E = 0;
	header->S = 0;
}

void Fu::split(int fuNo){
	int i = 0;
	int fu_count = (nalu_len-1)/(MaxLen-2);		//有这么多段完整的fu
	int fu_least = (nalu_len-1)%(MaxLen-2);		//最后一段残fu的长度


	//重置header的S和E
	header->S = 0;
	header->E = 0;

	if(fuNo == 0){					//头片
		header->S = 1;
		//装载
		while(i < MaxLen - 2){
			data[i+2] = nalu[i+1];
			i++;
		}
		fu_len = MaxLen;
	}else if(fuNo == fu_count){		//尾片
		header->E = 1;
		//装载
		while(i < fu_least){
			data[i+2] = nalu[1 + fuNo * (MaxLen-2) + i];
			i++;
		}
		fu_len = fu_least+2;
	}else{							//中间片
		while(i  < MaxLen - 2){
			data[i+2] = nalu[1 + fuNo * (MaxLen-2) + i];
			i++;
		}
		fu_len = MaxLen;
	}
}

int Fu::getFuNums(){
	return (nalu_len-1)/(MaxLen-2) + 1;		//有这么多段完整的fu
}