
// NimaPlayerDlg.cpp : ʵ���ļ�
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
#include <SDL2/SDL.h>
#ifdef __cplusplus
};
#endif
#endif


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
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


// CNimaPlayerDlg �Ի���




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


// CNimaPlayerDlg ��Ϣ�������

BOOL CNimaPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
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

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CNimaPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CNimaPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//============================================================

//Refresh Event
#define SFM_REFRESH_EVENT  (SDL_USEREVENT + 1)

#define SFM_BREAK_EVENT  (SDL_USEREVENT + 2)

int thread_exit=0;
int thread_pause=0;

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
int ffmpegPlay(LPVOID lpParam)
{
	CString timelong;
	float /*framerate_temp,*/ timelong_temp/*, bitrate_temp*/;
	AVFormatContext	*pFormatCtx;
	int				i, video_stream, audio_stream;
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

	struct SwsContext *img_convert_ctx;

	char tempMsgBuf[255] = {0};
	CString str_msg;
	//char filepath[]="bigbuckbunny_480x272.h265";
	//char filepath[]="Titanic.ts";
	//�ļ�·������
	CNimaPlayerDlg *dlg = (CNimaPlayerDlg *)lpParam;
	wchar_t wFilePath[250] = {0};
	GetWindowText(dlg->m_edit_url, (LPTSTR)wFilePath, 250);
	
	int bufSize=WideCharToMultiByte(CP_ACP,NULL,wFilePath,-1,NULL,0,NULL,FALSE);
	char *filePath=new char[bufSize];
	WideCharToMultiByte(CP_ACP,NULL,wFilePath,-1,filePath,bufSize,NULL,FALSE);

	//sprintf_s(tempMsgBuf, "file path = %s", filePath);
	//str_msg = tempMsgBuf;
	//AfxMessageBox(str_msg);
	av_register_all();
	avformat_network_init();
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
	video_stream=audio_stream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++) {
		if(pFormatCtx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
			video_stream=i;
		}
		if(pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO){
			audio_stream = i;
		}
	}
		if(audio_stream == -1 && video_stream == -1){
			AfxMessageBox(_T("Didn't find a video/audio stream."));
			return -1;
		}

		//��Ƶ�������
		if(video_stream != -1){
			pCodecCtx=pFormatCtx->streams[video_stream]->codec;
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
		//��Ƶ�������
		if(audio_stream != -1){
			pCodecCtx_au = pFormatCtx->streams[audio_stream]->codec;
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
		//duration����΢��Ϊ��λ
		timelong_temp = pFormatCtx->duration / 1000000;
		int tns, thh, tmm, tss;
		tns = pFormatCtx->duration /1000000;
		thh = tns /3600;
		tmm = (tns % 3600)/ 60;
		tss = (tns % 60);
		timelong.Format(_T("%02d:%02d:%02d"), thh, tmm, tss);
		//timelong.Format(_T("pFormatCtx->duration = %lld,  ::%02d:%02d:%02d"),pFormatCtx->duration, thh, tmm, tss);
		AfxMessageBox(timelong);
		dlg->m_sta_duration.SetWindowText(timelong);

		pFrame=av_frame_alloc();//����ԭʼ��֡
		pFrameYUV=av_frame_alloc();//����ת�����֡

		out_buffer=(unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_YUV420P,  pCodecCtx->width, pCodecCtx->height,1));//����ԭʼ����
		av_image_fill_arrays(pFrameYUV->data, pFrameYUV->linesize,out_buffer,
			AV_PIX_FMT_YUV420P,pCodecCtx->width, pCodecCtx->height,1);//��֡����������ڴ���

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
		//��ʾ�ڵ�������
		//screen = SDL_CreateWindow("Simplest ffmpeg player's Window", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		//	screen_w, screen_h,SDL_WINDOW_OPENGL);
		//��ʾ��MFC�ؼ���
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

					if(packet->stream_index==video_stream)
						break;
				}
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
//============================================================
//���ŵ��߳�
UINT Thread_Play(LPVOID lpParam){
	CNimaPlayerDlg *dlg = (CNimaPlayerDlg *)lpParam;
	ffmpegPlay(lpParam);
	return 0;
}
void CNimaPlayerDlg::OnBnClickedBtnPlay()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//NET_SDK_CLIENTINFO clientInfo;
	pThreadPlay = AfxBeginThread(Thread_Play, this);//�����߳�
}



void CNimaPlayerDlg::OnBnClickedBtnFileopen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CString filePathName;
	CFileDialog dlg(TRUE, NULL, NULL, NULL, NULL);// TRUE ΪOPEN�Ի���
	if(dlg.DoModal() == IDOK){
		filePathName = dlg.GetPathName();
		m_edit_url.SetWindowTextW(filePathName);
	}
}


void CNimaPlayerDlg::OnBnClickedBtnPause()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	thread_pause = !thread_pause;
}



void CNimaPlayerDlg::OnBnClickedBtnStop()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	thread_exit = 1;
}
