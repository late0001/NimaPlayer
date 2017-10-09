
// NimaPlayerDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CNimaPlayerDlg �Ի���
class CNimaPlayerDlg : public CDialogEx
{
// ����
public:
	CNimaPlayerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_NIMAPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBtnPlay();

protected:
	CWinThread *pThreadPlay;
public:
	CEdit m_edit_url;
	afx_msg void OnBnClickedBtnFileopen();
	afx_msg void OnBnClickedBtnPause();
	afx_msg void OnBnClickedBtnStop();
	CStatic m_sta_duration;
};
