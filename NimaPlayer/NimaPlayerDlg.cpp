
// NimaPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "NimaPlayer.h"
#include "NimaPlayerDlg.h"
#include "afxdialogex.h"

#ifdef _WIN32
//Windows
extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libswresample/swresample.h"
#include "SDL2/SDL.h"
};
#else
//Linux...
#ifdef __cplusplus
extern "C"
{
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libswresample/swresample.h>
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CNimaPlayerDlg 对话框




CNimaPlayerDlg::CNimaPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNimaPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CNimaPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDT_URL, m_edit_url);
	DDX_Control(pDX, IDC_LBL_DURATION, m_sta_duration);
}

BEGIN_MESSAGE_MAP(CNimaPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_PLAY, &CNimaPlayerDlg::OnBnClickedBtnPlay)
	ON_BN_CLICKED(IDC_BTN_FILEOPEN, &CNimaPlayerDlg::OnBnClickedBtnFileopen)
	ON_BN_CLICKED(IDC_BTN_PAUSE, &CNimaPlayerDlg::OnBnClickedBtnPause)
	ON_BN_CLICKED(IDC_BTN_STOP, &CNimaPlayerDlg::OnBnClickedBtnStop)
END_MESSAGE_MAP()


// CNimaPlayerDlg 消息处理程序

BOOL CNimaPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CNimaPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CNimaPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CNimaPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//============================================================

//#define VIDEO_FMT PIX_FMT_BGR24 
#define NVIDEO_PIX_FMT AV_PIX_FMT_YUV420P 
#define SDL_AUDIO_BUFFER_SIZE 1024
#define MAX_AUDIO_FRAME_SIZE 192000 // 1 second of 48khz 32bit audio 
//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;
int thread_pause=0;
int flag=100;//标识 播放，暂停，退出 0退出，1标识，2暂停

int fileDuration=0;//视频的长度  单位秒
//快进的参数
bool isSeek=false;//是否在快进
int global_seek_index=0;//文件索引 快进
double global_seek_pos=0;//快进的地方

//Buffer:  
//|-----------|-------------|  
//chunk-------pos---len-----|  
static  Uint8  *audio_chunk;   
static  Uint32  audio_len;   
static  Uint8  *audio_pos;

int sfp_refresh_thread(void *opaque){
	thread_exit=0;
	thread_pause=0;

	while (!thread_exit) {
		if(!thread_pause){
			SDL_Event event;
			event.type = SFM_REFRESH_EVENT;
			SDL_PushEvent(&event);
		}
		SDL_Delay(40);
	}
	thread_exit=0;
	thread_pause=0;
	//Break
	SDL_Event event;
	event.type = SFM_BREAK_EVENT;
	SDL_PushEvent(&event);

	return 0;
}

#define VideoBufferMaxSize 80//2048  //视频缓冲区最大值，大于此值 则不下载数据
#define VideoBufferMinSize 20//1024  //视频缓冲区最小值，小于此值，则唤醒下载
#define AudioBufferMaxSize 80//2048  //音频缓冲区最大值，大于此值 则不下载数据
#define AudioBufferMinSize 20//1024  //音频缓冲区最小值，小于此值，则唤醒下载

typedef struct VideoItem{
	Uint8 *videoData; //视频数据
	int width; //视频图片的宽度
	int height; //视频图片的高度
	int length; //视频长度
	double pts; //时间戳
	VideoItem *next; //尾部
}VideoItem;

typedef struct {
	VideoItem *firstItem;//队列头
	VideoItem *lastItem;//队列尾
	int length; //队列长度
	double bufferPts; // 缓冲的pts
	SDL_mutex *videoMutex; //用于同步两个线程同时操作队列的 互斥量
	SDL_cond *videoCond; //唤醒线程
}VideoQueue;

typedef struct AudioItem{
	Uint8 *audioData;//音频数据
	int length;//音频长度
	double pts;//时间戳
	AudioItem *next;
	SDL_AudioSpec *wanted_spec;
}AudioItem;

//存储音频的队列
typedef struct  {
	AudioItem *firstItem;//队列头
	AudioItem *lastItem; //队列尾
	int length; //队列长度
//	AVPacketList *first_pkt, *last_pkt; /* 队列头，队列尾 */
//	int nb_packets; /* 队列长度 */
	int size; 
	SDL_mutex *audioMutex; /* 用于同步两个线程同时操作队列的 互斥量 */
	SDL_cond *audioCond; //唤醒线程
} AudioQueue;

typedef struct {
	int length;
	char **urls;
	int *times; 
}VideoState;

VideoQueue *videoQueue = NULL; //视频队列
AudioQueue *audioQueue = NULL; //音频队列

//清空视频队列
void VideoQueueClear(VideoQueue *vq)
{
	VideoItem *item, *temp;
	SDL_LockMutex(vq->videoMutex);
	for( item = vq->firstItem; item != NULL; item = temp)
	{
		temp = item->next;
		av_free(item->videoData);//释放video里面的数据
		av_free(item);
		vq->length--;
	}
	vq->firstItem = NULL;
	vq->lastItem = NULL;
	SDL_UnlockMutex(vq->videoMutex);
}

//清空音频队列
void AudioQueueClear(AudioQueue *aq)
{
	AudioItem *item,*temp;
	SDL_LockMutex(aq->audioMutex);
	for (item=aq->firstItem; item!=NULL; item=temp)
	{
		temp=item->next;//
		av_free(item->audioData);//释放video里面的数据
		av_free(item->wanted_spec);
		av_free(item);
		aq->Length--;
	}
	aq->firstItem=NULL;
	aq->lastItem=NULL;
	SDL_UnlockMutex(aq->audioMutex);
}

//初始化视频队列
void VideoQueueInit(VideoQueue *vq)
{
	memset(vq, 0, sizeof(VideoQueue));//初始化首地址为0
	vq->videoMutex=SDL_CreateMutex();
	vq->videoCond=SDL_CreateCond();
}
//初始化音频队列
void AudioQueueInit(AudioQueue *aq)
{
	memset(aq,0,sizeof(AudioQueue));
	aq->audioMutex=SDL_CreateMutex();
	aq->audioCond=SDL_CreateCond();
}

//向队列添加数据
int VideoQueuePut(VideoQueue *vq,VideoItem *item)
{
	int result=0;
	SDL_LockMutex(vq->videoMutex);//加锁
	if(vq->Length < VideoBufferMaxSize)
	{
		if(!vq->firstItem)//第一个item为null 则队列是空的
		{
			vq->firstItem=item;
			vq->lastItem=item;
			vq->length=1;
			vq->bufferPts=item->pts;
		}
		else
		{
			vq->lastItem->next=item;//添加到队列后面
			vq->length++;
			vq->lastItem=item;//此item变成队列尾部
			vq->bufferPts=item->pts;
		}
		if(vq->length >= VideoBufferMinSize)
		{
			SDL_CondSignal(vq->videoCond);//唤醒其他线程  如果缓冲区里面有几个数据后再唤醒 较好
		}
		result=1;
	}
	else
	{
		SDL_CondWait(vq->videoCond,vq->videoMutex);//解锁  等待被唤醒
	}
	SDL_UnlockMutex(vq->videoMutex);//解锁
	return result;
}
//从队列中取出数据，放入item中
int  VideoQueueGet(VideoQueue *vq,VideoItem *item)
{
	int result=0;
	SDL_LockMutex(vq->videoMutex);
	if(vq->length>0)
	{
		if(vq->firstItem)//有数据
		{
			*item=*(vq->firstItem);
			if(!vq->firstItem->next)//只有一个
			{
				vq->firstItem=NULL;
				vq->lastItem=NULL;
			}else
			{
				vq->firstItem=vq->firstItem->next;
			}
			vq->length--;
			item->next=NULL;
			result= 1;
		}
		if(vq->length<=VideoBufferMinSize)
		{
			SDL_CondSignal(vq->videoCond);//唤醒下载线程
		}
	}
	else
	{
		SDL_CondWait(vq->videoCond,vq->videoMutex);//解锁  等待被唤醒
	}

	SDL_UnlockMutex(vq->videoMutex);
	return result;
}

//向队列添加数据
int AudioQueuePut(AudioQueue *aq,AudioItem *item)
{
	int result=0;
	SDL_LockMutex(aq->audioMutex);//加锁
	if(aq->length<AudioBufferMaxSize)
	{
		if(!aq->firstItem)//第一个item为null 则队列是空的
		{
			aq->firstItem=item;
			aq->lastItem=item;
			aq->length=1;
		}
		else
		{
			aq->lastItem->next=item;//添加到队列后面
			aq->length++;
			aq->lastItem=item;//此item变成队列尾部
		}

		if(aq->length>=AudioBufferMinSize)
		{
			SDL_CondSignal(aq->audioCond);//唤醒其他线程  如果缓冲区里面有几个数据后再唤醒 较好
		}
		result=1;
	}
	else///音频缓冲区的大小 大于设定值 则让线程等待
	{
		SDL_CondWait(aq->audioCond,aq->audioMutex);//解锁  等待被唤醒
	}
	SDL_UnlockMutex(aq->audioMutex);//解锁
	return result;
}
//从队列中取出数据，放入item中
int AudioQueueGet(AudioQueue *aq,AudioItem *item)
{
	int result=0;
	SDL_LockMutex(aq->audioMutex);
	if(aq->length>0)
	{
		if(aq->firstItem)//有数据
		{
			*item=*(aq->firstItem);
			if(!aq->firstItem->next)//只有一个
			{
				aq->firstItem=NULL;
				aq->lastItem=NULL;
			}else
			{
				aq->firstItem=aq->firstItem->next;
			}
			aq->length--;
			item->next=NULL;
			result=1;
		}
		if(aq->length<=AudioBufferMinSize)
		{
			SDL_CondSignal(aq->audioCond);//唤醒下载线程
		}
	}else
	{
		SDL_CondWait(aq->audioCond,aq->audioMutex);//解锁  等待被唤醒
	}
	SDL_UnlockMutex(aq->audioMutex);
	return result;
}

AudioQueue audioq;

//int quit = 0;

void packet_queue_init(AudioQueue *q) {
	memset(q, 0, sizeof(AudioQueue));
	q->mutex = SDL_CreateMutex();
	q->cond = SDL_CreateCond();
}
int packet_queue_put(AudioQueue *q, AVPacket *pkt) {

	AVPacketList *pkt1;
	if(av_dup_packet(pkt) < 0) {
		return -1;
	}
	pkt1 = (AVPacketList *)av_malloc(sizeof(AVPacketList));
	if (!pkt1)
		return -1;
	pkt1->pkt = *pkt;
	pkt1->next = NULL;


	SDL_LockMutex(q->mutex);

	if (!q->last_pkt)
		q->first_pkt = pkt1;
	else
		q->last_pkt->next = pkt1;
	q->last_pkt = pkt1;
	q->nb_packets++;
	q->size += pkt1->pkt.size;
	SDL_CondSignal(q->cond);

	SDL_UnlockMutex(q->mutex);
	return 0;
}
static int packet_queue_get(AudioQueue *q, AVPacket *pkt, int block)
{
	AVPacketList *pkt1;
	int ret;

	SDL_LockMutex(q->mutex);

	for(;;) {

		if(thread_exit) {
			ret = -1;
			break;
		}

		pkt1 = q->first_pkt;
		if (pkt1) {
			q->first_pkt = pkt1->next;
			if (!q->first_pkt)
				q->last_pkt = NULL;
			q->nb_packets--;
			q->size -= pkt1->pkt.size;
			*pkt = pkt1->pkt;
			av_free(pkt1);
			ret = 1;
			break;
		} else if (!block) {
			ret = 0;
			break;
		} else {
			SDL_CondWait(q->cond, q->mutex);
		}
	}
	SDL_UnlockMutex(q->mutex);
	return ret;
}

int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {

	static AVPacket pkt;
	static uint8_t *audio_pkt_data = NULL;
	static int audio_pkt_size = 0;
	static AVFrame frame;

	int len1, data_size = 0;

	for(;;) {
		while(audio_pkt_size > 0) {
			int got_frame = 0;
			len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
			if(len1 < 0) {
				/* if error, skip frame */
				audio_pkt_size = 0;
				break;
			}
			audio_pkt_data += len1;
			audio_pkt_size -= len1;
			if (got_frame)
			{
				data_size = 
					av_samples_get_buffer_size
					(
					NULL, 
					aCodecCtx->channels,
					frame.nb_samples,
					aCodecCtx->sample_fmt,
					1
					);
				memcpy(audio_buf, frame.data[0], data_size);
			}
			if(data_size <= 0) {
				/* No data yet, get more frames */
				continue;
			}
			/* We have data, return it and come back for more later */
			return data_size;
		}
		if(pkt.data)
			av_free_packet(&pkt);

		if(thread_exit) {
			return -1;
		}

		if(packet_queue_get(&audioq, &pkt, 1) < 0) {
			return -1;
		}
		audio_pkt_data = pkt.data;
		audio_pkt_size = pkt.size;
	}
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
#if 1
	SDL_memset(stream, 0, len);
	if(audio_len == 0)
		return;

	len = (len > audio_len? audio_len: len);/*  Mix  as  much  data  as  possible  */

	SDL_MixAudio(stream, audio_pos, len, SDL_MIX_MAXVOLUME);
	audio_pos += len;
	audio_len -= len;
#endif
#if 0
	AVCodecContext *pCodecCtx_au = (AVCodecContext *)userdata;
	int len1, audio_size;

	static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
	static unsigned int audio_buf_size = 0;
	static unsigned int audio_buf_index = 0;

	while(len > 0) {
		if(audio_buf_index >= audio_buf_size) {
			/* We have already sent all our data; get more */
			audio_size = audio_decode_frame(pCodecCtx_au, audio_buf, audio_buf_size);
			if(audio_size < 0) {
				/* If error, output silence */
				audio_buf_size = 1024; // arbitrary?
				memset(audio_buf, 0, audio_buf_size);
			} else {
				audio_buf_size = audio_size;
			}
			audio_buf_index = 0;
		}
		len1 = audio_buf_size - audio_buf_index;
		if(len1 > len)
			len1 = len;
		memcpy(stream, (uint8_t *)audio_buf + audio_buf_index, len1);
		len -= len1;
		stream += len1;
		audio_buf_index += len1;
	}
#endif
}

int ffmpegPlay1(LPVOID lpParam, void *arg)
{
	CString timelong;
	float /*framerate_temp,*/ timelong_temp/*, bitrate_temp*/;
	AVFormatContext	*pFormatCtx;
	int				i, video_index, audio_index;
	AVCodecContext	*pCodecCtx, *pCodecCtx_au;
	AVCodec			*pCodec, *pCodec_au;
	AVFrame	*pFrame,*pFrameYUV;
	unsigned char *out_buffer;
	AVPacket *packet;
	int ret, got_picture;

	//------------SDL----------------
	int screen_w,screen_h;
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid;
	SDL_Event event;
	SDL_AudioSpec   wanted_spec, spec;

	struct SwsContext *img_convert_ctx;

	VideoState *vs = (VideoState *)arg;
	int length = vs->length;
	double currentAllFilePts = 0;

	char tempMsgBuf[255] = {0};
	CString str_msg;
	//char filepath[]="bigbuckbunny_480x272.h265";
	//char filepath[]="Titanic.ts";
	//文件路径如下
	CNimaPlayerDlg *dlg = (CNimaPlayerDlg *)lpParam;
	wchar_t wFilePath[250] = {0};
	GetWindowText(dlg->m_edit_url, (LPTSTR)wFilePath, 250);
	
	int bufSize=WideCharToMultiByte(CP_ACP,NULL,wFilePath,-1,NULL,0,NULL,FALSE);
	char *filePath=new char[bufSize];
	WideCharToMultiByte(CP_ACP,NULL,wFilePath,-1,filePath,bufSize,NULL,FALSE);

	//sprintf_s(tempMsgBuf, "file path = %s", filePath);
	//str_msg = tempMsgBuf;
	//AfxMessageBox(str_msg);
	av_register_all();//注册所有解码器
	avformat_network_init();//初始化流媒体格式
	for (int j = 0; j < length; j++){

	double currentFilePts = 0;
	char *url = vs->urls[j];
	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx,url,NULL,NULL)!=0){
		AfxMessageBox(L"Couldn't open input stream.\n");
		return -1;
	}
	delete[] filePath;

	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		AfxMessageBox(L"Couldn't find stream information.\n");
		return -1;
	}
	video_index=audio_index=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			video_index=i;
		}
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audio_index = i;
		}
	}
		if(audio_index == -1 && video_index == -1){
			AfxMessageBox(_T("Didn't find a video/audio stream."));
			return -1;
		}

		//视频解码参数
		if(video_index != -1){
			//视频解码器上下文
			pCodecCtx=pFormatCtx->streams[video_index]->codec;
			pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
			if(pCodec==NULL){
				AfxMessageBox(L"Codec not found.\n");
				return -1;
			}
			if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
				AfxMessageBox(L"Could not open codec.\n");
				return -1;
			}
		}
		//音频解码参数
		if(audio_index != -1){
			pCodecCtx_au = pFormatCtx->streams[audio_index]->codec;

#if 0
			// Set audio settings from codec info
			wanted_spec.freq = pCodecCtx_au->sample_rate;
			wanted_spec.format = AUDIO_S16SYS;
			wanted_spec.channels = pCodecCtx_au->channels;
			wanted_spec.silence = 0;
			wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //pCodecCtx_au->frame_size; 
			wanted_spec.callback = audio_callback;
			wanted_spec.userdata = pCodecCtx_au;

			if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
				fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
				return -1;
			}
#endif

			pCodec_au = avcodec_find_decoder(pCodecCtx_au->codec_id);
			if(pCodec_au == NULL){
				AfxMessageBox(_T("Audio codec not found."));
				return -1;
			}

			if(avcodec_open2(pCodecCtx_au, pCodec_au, NULL) < 0 ){
				AfxMessageBox(_T("Could not open audio codec."));
				return -1;
			}
		}
		////duration是以微秒为单位
		//timelong_temp = pFormatCtx->duration / 1000000;
		//int tns, thh, tmm, tss;
		//tns = pFormatCtx->duration /1000000;
		//thh = tns /3600;
		//tmm = (tns % 3600)/ 60;
		//tss = (tns % 60);
		//timelong.Format(_T("%02d:%02d:%02d"), thh, tmm, tss);
		////timelong.Format(_T("pFormatCtx->duration = %lld,  ::%02d:%02d:%02d"),pFormatCtx->duration, thh, tmm, tss);
		////AfxMessageBox(timelong);
		//if(tns < 0){
		//	timelong = _T("--:--:--");
		//}
		//dlg->m_sta_duration.SetWindowText(timelong);

		pFrame=av_frame_alloc();//保存原始的帧
		pFrameYUV=av_frame_alloc();//保存转换后的帧
		int frameFinished=0;//是否凑成一帧数据
		int result=0;//标识一个视频是否解码完毕
		int audioLength=0;//音频数组的长度
		int videoLength=0;//视频数组的长度

		//把视频帧转化为数组参数
		struct SwsContext *img_convert_ctx;
		if( video_index != -1){
			
			videoLength = av_image_get_buffer_size(NVIDEO_PIX_FMT,  pCodecCtx->width, pCodecCtx->height,1);
			img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
				pCodecCtx->width, pCodecCtx->height, NVIDEO_PIX_FMT, SWS_BICUBIC, NULL, NULL, NULL); 
		}


		//out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));//放置原始数据
		//av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer,
		//	AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);//把帧和新申请的内存结合

		////Output Info-----------------------------
		//printf("---------------- File Information ---------------\n");
		////av_dump_format(pFormatCtx,0,filePath,0);
		//
		//printf("-------------------------------------------------\n");

		//img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		//	pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 
		//把音频帧转化为数组的参数
		//uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO; 
		AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;
		int out_sample_rate=44100;
		int64_t in_channel_layout=av_get_channel_layout_nb_channels(pCodecCtx_au->channels);

		int out_channels=av_get_channel_layout_nb_channels(pCodecCtx_au->channels);
		int out_nb_samples=1024;
		audioLength=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);

		struct SwrContext *au_convert_ctx; 
		au_convert_ctx = swr_alloc(); 
		au_convert_ctx=swr_alloc_set_opts(au_convert_ctx,pCodecCtx_au->channels, out_sample_fmt, out_sample_rate, 
			in_channel_layout,pCodecCtx_au->sample_fmt , pCodecCtx_au->sample_rate,0, NULL); 
		swr_init(au_convert_ctx); 
		int sample=SDL_AUDIO_BUFFER_SIZE;

		
		}
		if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
			sprintf_s( tempMsgBuf, "Could not initialize SDL - %s\n", SDL_GetError()); 
			str_msg = tempMsgBuf;
			AfxMessageBox(str_msg);
			return -1;
		} 


		//SDL 2.0 Support for multiple windows
		screen_w = pCodecCtx->width;
		screen_h = pCodecCtx->height;
		//显示在弹出窗口
		//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		//	screen_w, screen_h,SDL_WINDOW_OPENGL);
		//显示在MFC控件上
		screen = SDL_CreateWindowFrom(dlg->GetDlgItem(IDC_PIC)->GetSafeHwnd());
		if(!screen) {  
			sprintf_s(tempMsgBuf, "SDL: could not create window - exiting:%s\n",SDL_GetError()); 
			str_msg = tempMsgBuf;
			AfxMessageBox(str_msg);
			return -1;
		}
		sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
		//IYUV: Y + U + V  (3 planes)
		//YV12: Y + V + U  (3 planes)
		sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

		sdlRect.x=0;
		sdlRect.y=0;
		sdlRect.w=screen_w;
		sdlRect.h=screen_h;

		packet=(AVPacket *)av_malloc(sizeof(AVPacket));
		//-------------------------------
		//av_init_packet(packet); 
		//Out Audio Param  
		//uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;  
		//nb_samples: AAC-1024 MP3-1152  
		//int out_nb_samples=pCodecCtx_au->frame_size;  
		//AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;  
		//int out_sample_rate=44100;  
		//int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);  
		//Out Buffer Size  
		//int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);  

		//out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);  
		//pFrame=av_frame_alloc();  

		//SDL_AudioSpec  
		//wanted_spec.freq = out_sample_rate;   
		//wanted_spec.format = AUDIO_S16SYS;   
		//wanted_spec.channels = out_channels;   
		//wanted_spec.silence = 0;   
		//wanted_spec.samples = out_nb_samples;   
		//wanted_spec.callback = fill_audio;   
		//wanted_spec.userdata = pCodecCtx;   

		//if (SDL_OpenAudio(&wanted_spec, NULL)<0){   
		//	printf("can't open audio.\n");   
		//	return -1;   
		//}   
		//---------------------------------

		video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
		//------------SDL End------------
		//Event Loop

		for (;;) {
			//Wait
			SDL_WaitEvent(&event);
			if(event.type==SFM_REFRESH_EVENT){
				while(1){
					if(av_read_frame(pFormatCtx, packet)<0)
						thread_exit=1;

					if(packet->stream_index == video_index || packet->stream_index == audio_index)
						break;
				}
				if(isSeek) //要快进
				{
					//快进
					if( j == global_seek_index){
						int seekFlag = avformat_seek_file(pFormatCtx,
							-1, 
							(global_seek_pos - 10)* AV_TIME_BASE, 
							global_seek_pos *AV_TIME_BASE,
							(global_seek_pos + 10)*AV_TIME_BASE,
							AVSEEK_FLAG_ANY);
						if(seekFlag >= 0){
							currentAllFilePts = 0;
							for (int k= 0; k < j; k++)
								currentAllFilePts += vs->times[k];
						}
						isSeek = false;
					}else{
						j = global_seek_index - 1;
						break;
					}
				}

				if( packet->stream_index == video_index ){
					ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
					if(ret < 0){
						AfxMessageBox(L"Decode Error.\n");
						return -1;
					}
					if(got_picture){
						sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
						//SDL---------------------------
						SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
						SDL_RenderClear( sdlRenderer );  
						//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
						SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);  
						SDL_RenderPresent( sdlRenderer );  
						//SDL End-----------------------
					}
					av_free_packet(packet);
				}else if( packet->stream_index == audio_index ){
					packet_queue_put(&audioq, packet);
				} else {
					av_free_packet(packet);
				}
				av_free_packet(packet);
			}else if(event.type==SDL_KEYDOWN){
				//Pause
				if(event.key.keysym.sym==SDLK_SPACE)
					thread_pause=!thread_pause;
			}else if(event.type==SDL_QUIT){
				thread_exit=1;
			}else if(event.type==SFM_BREAK_EVENT){
				break;
			}

		}

		sws_freeContext(img_convert_ctx);

		SDL_Quit();
		dlg->GetDlgItem(IDC_PIC)->ShowWindow(SW_SHOWNORMAL);
		//--------------
		av_frame_free(&pFrameYUV);
		av_frame_free(&pFrame);
		avcodec_close(pCodecCtx);
		avformat_close_input(&pFormatCtx);

		return 0;
}

//获取视频的总长度
void InitAllTime(VideoState *vs)
{
	fileDuration=0;
	int length=vs->length;
	av_register_all();  //注册所有解码器
	avformat_network_init();  //初始化流媒体格式   
	for (int j = 0; j < length; j++)
	{
		char* url=vs->urls[j];
		AVFormatContext *pFormatCtx = avformat_alloc_context(); 
		//打开文件
		if(avformat_open_input(&pFormatCtx,url,NULL,NULL)!=0)
		{ 
			//strcpy(ErrorMsg,"无法打开网络流");
			return;
		}
		if(av_find_stream_info(pFormatCtx)<0) 
		{ 
			//strcpy(ErrorMsg,"无法获取流信息"); 
			return; 
		}
		//保存每个文件的播放长度
		vs->times[j]=pFormatCtx->duration/1000000;
		//获取此视频的总时间 微妙转化为妙
		fileDuration+= vs->times[j];

		avformat_close_input(&pFormatCtx);
	}
	avformat_network_deinit();
}
//============================================================

int ffmpegPlay(LPVOID lpParam)
{
	CString timelong;
	float /*framerate_temp,*/ timelong_temp/*, bitrate_temp*/;
	AVFormatContext	*pFormatCtx;
	int				i, video_index, audio_index;
	AVCodecContext	*pCodecCtx, *pCodecCtx_au;
	AVCodec			*pCodec, *pCodec_au;
	AVFrame	*pFrame,*pFrameYUV;
	unsigned char *out_buffer;
	AVPacket *packet;
	int ret, got_picture;

	//------------SDL----------------
	int screen_w,screen_h;
	SDL_Window *screen; 
	SDL_Renderer* sdlRenderer;
	SDL_Texture* sdlTexture;
	SDL_Rect sdlRect;
	SDL_Thread *video_tid;
	SDL_Event event;
	SDL_AudioSpec   wanted_spec, spec;

	struct SwsContext *img_convert_ctx;

	char tempMsgBuf[255] = {0};
	CString str_msg;
	//char filepath[]="bigbuckbunny_480x272.h265";
	//char filepath[]="Titanic.ts";
	//文件路径如下
	CNimaPlayerDlg *dlg = (CNimaPlayerDlg *)lpParam;
	wchar_t wFilePath[250] = {0};
	GetWindowText(dlg->m_edit_url, (LPTSTR)wFilePath, 250);

	int bufSize=WideCharToMultiByte(CP_ACP,NULL,wFilePath,-1,NULL,0,NULL,FALSE);
	char *filePath=new char[bufSize];
	WideCharToMultiByte(CP_ACP,NULL,wFilePath,-1,filePath,bufSize,NULL,FALSE);

	//sprintf_s(tempMsgBuf, "file path = %s", filePath);
	//str_msg = tempMsgBuf;
	//AfxMessageBox(str_msg);
	av_register_all();//注册所有解码器
	avformat_network_init();//初始化流媒体格式
	pFormatCtx = avformat_alloc_context();

	if(avformat_open_input(&pFormatCtx,filePath,NULL,NULL)!=0){
		AfxMessageBox(L"Couldn't open input stream.\n");
		return -1;
	}
	delete[] filePath;

	if(avformat_find_stream_info(pFormatCtx,NULL)<0){
		AfxMessageBox(L"Couldn't find stream information.\n");
		return -1;
	}
	video_index=audio_index=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			video_index=i;
		}
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audio_index = i;
		}
	}
	if(audio_index == -1 && video_index == -1){
		AfxMessageBox(_T("Didn't find a video/audio stream."));
		return -1;
	}

	//视频解码参数
	if(video_index != -1){
		//视频解码器上下文
		pCodecCtx=pFormatCtx->streams[video_index]->codec;
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(pCodec==NULL){
			AfxMessageBox(L"Codec not found.\n");
			return -1;
		}
		if(avcodec_open2(pCodecCtx, pCodec,NULL)<0){
			AfxMessageBox(L"Could not open codec.\n");
			return -1;
		}
	}
	//音频解码参数
	if(audio_index != -1){
		pCodecCtx_au = pFormatCtx->streams[audio_index]->codec;

		// Set audio settings from codec info
		wanted_spec.freq = pCodecCtx_au->sample_rate;
		wanted_spec.format = AUDIO_S16SYS;
		wanted_spec.channels = pCodecCtx_au->channels;
		wanted_spec.silence = 0;
		wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE; //pCodecCtx_au->frame_size; 
		wanted_spec.callback = audio_callback;
		wanted_spec.userdata = pCodecCtx_au;

		if(SDL_OpenAudio(&wanted_spec, &spec) < 0) {
			fprintf(stderr, "SDL_OpenAudio: %s\n", SDL_GetError());
			return -1;
		}

		pCodec_au = avcodec_find_decoder(pCodecCtx_au->codec_id);
		if(pCodec_au == NULL){
			AfxMessageBox(_T("Audio codec not found."));
			return -1;
		}

		if(avcodec_open2(pCodecCtx_au, pCodec_au, NULL) < 0 ){
			AfxMessageBox(_T("Could not open audio codec."));
			return -1;
		}
		packet_queue_init(&audioq);
		SDL_PauseAudio(0);
	}
	//duration是以微秒为单位
	timelong_temp = pFormatCtx->duration / 1000000;
	int tns, thh, tmm, tss;
	tns = pFormatCtx->duration /1000000;
	thh = tns /3600;
	tmm = (tns % 3600)/ 60;
	tss = (tns % 60);
	timelong.Format(_T("%02d:%02d:%02d"), thh, tmm, tss);
	//timelong.Format(_T("pFormatCtx->duration = %lld,  ::%02d:%02d:%02d"),pFormatCtx->duration, thh, tmm, tss);
	//AfxMessageBox(timelong);
	if(tns < 0){
		timelong = _T("--:--:--");
	}
	dlg->m_sta_duration.SetWindowText(timelong);

	pFrame=av_frame_alloc();//保存原始的帧
	pFrameYUV=av_frame_alloc();//保存转换后的帧

	out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));//放置原始数据
	av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer,
		AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);//把帧和新申请的内存结合

	//Output Info-----------------------------
	printf("---------------- File Information ---------------\n");
	//av_dump_format(pFormatCtx,0,filePath,0);

	printf("-------------------------------------------------\n");

	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt, 
		pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL); 


	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
		sprintf_s( tempMsgBuf, "Could not initialize SDL - %s\n", SDL_GetError()); 
		str_msg = tempMsgBuf;
		AfxMessageBox(str_msg);
		return -1;
	} 


	//SDL 2.0 Support for multiple windows
	screen_w = pCodecCtx->width;
	screen_h = pCodecCtx->height;
	//显示在弹出窗口
	//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
	//	screen_w, screen_h,SDL_WINDOW_OPENGL);
	//显示在MFC控件上
	screen = SDL_CreateWindowFrom(dlg->GetDlgItem(IDC_PIC)->GetSafeHwnd());
	if(!screen) {  
		sprintf_s(tempMsgBuf, "SDL: could not create window - exiting:%s\n",SDL_GetError()); 
		str_msg = tempMsgBuf;
		AfxMessageBox(str_msg);
		return -1;
	}
	sdlRenderer = SDL_CreateRenderer(screen, -1, 0);  
	//IYUV: Y + U + V  (3 planes)
	//YV12: Y + V + U  (3 planes)
	sdlTexture = SDL_CreateTexture(sdlRenderer, SDL_PIXELFORMAT_IYUV, SDL_TEXTUREACCESS_STREAMING,pCodecCtx->width,pCodecCtx->height);  

	sdlRect.x=0;
	sdlRect.y=0;
	sdlRect.w=screen_w;
	sdlRect.h=screen_h;

	packet=(AVPacket *)av_malloc(sizeof(AVPacket));
	//-------------------------------
	//av_init_packet(packet); 
	//Out Audio Param  
	//uint64_t out_channel_layout=AV_CH_LAYOUT_STEREO;  
	//nb_samples: AAC-1024 MP3-1152  
	//int out_nb_samples=pCodecCtx_au->frame_size;  
	//AVSampleFormat out_sample_fmt=AV_SAMPLE_FMT_S16;  
	//int out_sample_rate=44100;  
	//int out_channels=av_get_channel_layout_nb_channels(out_channel_layout);  
	//Out Buffer Size  
	//int out_buffer_size=av_samples_get_buffer_size(NULL,out_channels ,out_nb_samples,out_sample_fmt, 1);  

	//out_buffer=(uint8_t *)av_malloc(MAX_AUDIO_FRAME_SIZE*2);  
	//pFrame=av_frame_alloc();  

	//SDL_AudioSpec  
	//wanted_spec.freq = out_sample_rate;   
	//wanted_spec.format = AUDIO_S16SYS;   
	//wanted_spec.channels = out_channels;   
	//wanted_spec.silence = 0;   
	//wanted_spec.samples = out_nb_samples;   
	//wanted_spec.callback = fill_audio;   
	//wanted_spec.userdata = pCodecCtx;   

	//if (SDL_OpenAudio(&wanted_spec, NULL)<0){   
	//	printf("can't open audio.\n");   
	//	return -1;   
	//}   
	//---------------------------------

	video_tid = SDL_CreateThread(sfp_refresh_thread,NULL,NULL);
	//------------SDL End------------
	//Event Loop

	for (;;) {
		//Wait
		SDL_WaitEvent(&event);
		if(event.type==SFM_REFRESH_EVENT){
			while(1){
				if(av_read_frame(pFormatCtx, packet)<0)
					thread_exit=1;

				if(packet->stream_index == video_index || packet->stream_index == audio_index)
					break;
			}
			if( packet->stream_index == video_index ){
				ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
				if(ret < 0){
					AfxMessageBox(L"Decode Error.\n");
					return -1;
				}
				if(got_picture){
					sws_scale(img_convert_ctx, (const unsigned char* const*)pFrame->data, pFrame->linesize, 0, pCodecCtx->height, pFrameYUV->data, pFrameYUV->linesize);
					//SDL---------------------------
					SDL_UpdateTexture( sdlTexture, NULL, pFrameYUV->data[0], pFrameYUV->linesize[0] );  
					SDL_RenderClear( sdlRenderer );  
					//SDL_RenderCopy( sdlRenderer, sdlTexture, &sdlRect, &sdlRect );  
					SDL_RenderCopy( sdlRenderer, sdlTexture, NULL, NULL);  
					SDL_RenderPresent( sdlRenderer );  
					//SDL End-----------------------
				}
				av_free_packet(packet);
			}else if( packet->stream_index == audio_index ){
				packet_queue_put(&audioq, packet);
			} else {
				av_free_packet(packet);
			}
			av_free_packet(packet);
		}else if(event.type==SDL_KEYDOWN){
			//Pause
			if(event.key.keysym.sym==SDLK_SPACE)
				thread_pause=!thread_pause;
		}else if(event.type==SDL_QUIT){
			thread_exit=1;
		}else if(event.type==SFM_BREAK_EVENT){
			break;
		}

	}

	sws_freeContext(img_convert_ctx);

	SDL_Quit();
	dlg->GetDlgItem(IDC_PIC)->ShowWindow(SW_SHOWNORMAL);
	//--------------
	av_frame_free(&pFrameYUV);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
}

//播放的线程
UINT Thread_Play(LPVOID lpParam){
	CNimaPlayerDlg *dlg = (CNimaPlayerDlg *)lpParam;
	ffmpegPlay(lpParam);
	return 0;
}
void CNimaPlayerDlg::OnBnClickedBtnPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	//NET_SDK_CLIENTINFO clientInfo;
	pThreadPlay = AfxBeginThread(Thread_Play, this);//开启线程
}



void CNimaPlayerDlg::OnBnClickedBtnFileopen()
{
	// TODO: 在此添加控件通知处理程序代码
	CString filePathName;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, NULL);// TRUE 为OPEN对话框
	if(dlg.DoModal() == IDOK){
		filePathName = dlg.GetPathName();
		m_edit_url.SetWindowTextW(filePathName);
	}
}


void CNimaPlayerDlg::OnBnClickedBtnPause()
{
	// TODO: 在此添加控件通知处理程序代码
	thread_pause = !thread_pause;
}



void CNimaPlayerDlg::OnBnClickedBtnStop()
{
	// TODO: 在此添加控件通知处理程序代码
	thread_exit = 1;
}
