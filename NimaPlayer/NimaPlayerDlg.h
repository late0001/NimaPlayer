
// NimaPlayerDlg.h : 头文件
//

#pragma once
#include "afxwin.h"


// CNimaPlayerDlg 对话框
class CNimaPlayerDlg : public CDialogEx
{
// 构造
public:
	CNimaPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_NIMAPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
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
