#include <iostream>
#include <fstream>
#include <string>

#include "rtpsession.h"
#include "rtpudpv4transmitter.h"
#include "rtpipv4address.h"
#include "rtpsessionparams.h"
#include "rtperrors.h"
#include "rtplibraryversion.h"
#include "rtppacket.h"

#include "Fu.h"

#include "Buff.h"

#include <fstream>

using namespace std;
using namespace jrtplib;


#define MAX_PAYLOAD 1000

int read_a_frame(x264_picture_t *pic,const Mat &YUV){
	int width = pic->param->i_width;
	int height = pic->param->i_height;
	int frameSize = height * width;
	int p = 0;

	for(int i = 0;i<frameSize;i++){
		pic->img.plane[0][i] = YUV.data[p];
		p++;
	}
	for(int i = 0;i<frameSize/4;i++){
		pic->img.plane[1][i] = YUV.data[p];
		p++;
	}
	for(int i = 0;i<frameSize/4;i++){
		pic->img.plane[2][i] = YUV.data[p];
		p++;
	}
	return 0;
}

struct Pkt{
	FrameBuff *frames;
	NaluBuff *nalus;
};

DWORD WINAPI enc_thread(LPVOID lpParameter){							//编码线程，frameBuff=>naluBuff
	Pkt *pkt = (Pkt*)lpParameter;
	FrameBuff *frames = pkt->frames;
	NaluBuff *nalus = pkt->nalus;

	while(frames->isEmpty()) Sleep(1);		//等待framebuff中有数据

	int height = ((Mat*)frames->top())->size().height;
	int width = ((Mat*)frames->top())->size().width;

	x264_param_t para;
	x264_picture_t pic_in,pic_out;
	x264_t *handle;
	int64_t frameNo = 0;	//帧号
	int status = 0;		//返回状态

	x264_param_default(&para);
	x264_param_apply_profile(&para,"high");
	para.i_width = width;
	para.i_height = height;
	para.i_csp = X264_CSP_I420;
	para.i_fps_den = 1;
	para.i_fps_num = 8;

	x264_nal_t *nal;
	int nal_count = 0;

	//打开编码器
	handle = x264_encoder_open(&para);

	//申请图片内存
	x264_picture_alloc(&pic_in,X264_CSP_I420,width,height);
	x264_picture_init(&pic_out);
	pic_in.param = &para;

	//用于RGB=>YUV
	Mat frameYUV(height*3/2,width,CV_8UC1);

	while(1){
		Sleep(20);
		if(frames->isEmpty())
			continue;

		cvtColor(*(Mat*)frames->top(),frameYUV,CV_BGR2YUV_I420);	//取frame并转换像素格式

		frames->pop();		//frame出队列

		read_a_frame(&pic_in,frameYUV);
		pic_in.i_pts = frameNo++;		//pic的pts++;
		status = x264_encoder_encode(handle,&nal,&nal_count,&pic_in,&pic_out);	//进行编码
		
		for(int i = 0;i<nal_count;i++){		//遍历每个nal
			while(nalus->isFull()) Sleep(1);		//等待非空
			nalus->push(&(nal[i]));
		}
	}

	x264_picture_clean(&pic_in);
	x264_encoder_close(handle);

	return 0;
}

DWORD WINAPI cap_thread(LPVOID lpParameter){							//摄像头线程，将 采集=>frameBuff
	Pkt *pkt = (Pkt*)lpParameter;
	FrameBuff *&frames = pkt->frames;

	//打开设备
	VideoCapture cap;
	cap.open(0);


	while(cap.isOpened()){
		Sleep(10);
		if(!frames->isFull())
			frames->push(&cap);
	}

	cap.release();

	//test
// 	Mat m(200,200,CV_8UC3);
// 	while(1){
// 		randu(m, Scalar::all(0), cv::Scalar::all(255));
// 		frames->push(m);
// 		imshow("show",*(Mat*)frames->top());
// 		if(char(waitKey(100)) == 'q')
// 			Sleep(125);
// 	}

	return 0;
}

int main(){

#ifdef RTP_SOCKETTYPE_WINSOCK
	WSADATA dat;
	WSAStartup(MAKEWORD(2,2),&dat);
#endif // RTP_SOCKETTYPE_WINSOCK

	string ip = "127.0.0.1";

	/*
	*rtp
	*/
	RTPSession sess;	//会话
	RTPSessionParams sessPara;		//会话参数
	RTPUDPv4TransmissionParams tranParas;		//传输层参数
	int status;

	sessPara.SetAcceptOwnPackets(true);
	sessPara.SetOwnTimestampUnit(1.0/90000.0);
	tranParas.SetPortbase(5000);

	status = sess.Create(sessPara,&tranParas);

	RTPIPv4Address addr(ntohl(inet_addr(ip.c_str())),5002);
	status = sess.AddDestination(addr);


	Fu fu(MAX_PAYLOAD);		//初始化fu

	FrameBuff frames(50);
	NaluBuff nalus(50);
	Pkt pkt;
	pkt.frames = &frames;
	pkt.nalus = &nalus;

	CreateThread(NULL,0,cap_thread,&pkt,0,NULL);		//摄像头线程
	CreateThread(NULL,0,enc_thread,&pkt,0,NULL);		//编码线程

	x264_nal_t *nal = NULL;

	ofstream fout("F:\\video.264",ios::binary);

	while(1){

		while(nalus.isEmpty()) Sleep(1);
		nal = ((x264_nal_t*)nalus.top());	//取nalu

		//去掉00 00 00 01 或 00 00 01的nalu起始码
		int prefix = 0;
		while(nal->p_payload[prefix++] !=0x01);		//扫描起始码
		nal->p_payload+=prefix;		//负载首地址更新
		nal->i_payload-=prefix;		//负载长度更新

		if(nal->i_payload <= MAX_PAYLOAD){		//单个包就能发送
			status = sess.SendPacket(nal->p_payload,nal->i_payload,96,true,90000/25);
		}else{
			//将nalu加载到fu中
			fu.load((char *)nal->p_payload,nal->i_payload);
			//得到分片数量
			int fuNums = fu.getFuNums();
			//进行分片
			for(int f = 0;f<fuNums-1;f++){
				fu.split(f);
				status = sess.SendPacket(fu.data,fu.fu_len,96,false,0);
			}
			//尾片
			fu.split(fuNums-1);
			status = sess.SendPacket(fu.data,fu.fu_len,96,true,90000/25);
		}

		nalus.pop();	//nalu出队列

		Sleep(125);

	}

#ifdef RTP_SOCKETTYPE_WINSOCK
	WSACleanup();
#endif // RTP_SOCKETTYPE_WINSOCK

	system("pause");

	return 0;
}













