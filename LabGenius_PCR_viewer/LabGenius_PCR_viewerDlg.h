
// LabGenius_PCR_viewerDlg.h : 헤더 파일
//

#pragma once

#include ".\Lib\sqlite3.h"
#include ".\Lib\UseRegistry.h"
#include "Chart.h"

// CLabGenius_PCR_viewerDlg 대화 상자
class CLabGenius_PCR_viewerDlg : public CDialogEx
{
// 생성입니다.
public:
	CLabGenius_PCR_viewerDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	virtual ~CLabGenius_PCR_viewerDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_LABGENIUS_PCR_VIEWER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.


// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

	CMenu menu;
	int resource[10];
	int img_resource[4];

public:
	afx_msg void OnFileCsvexport();
	afx_msg void OnFileFOpen();
	afx_msg void OnBnClickedCheckCy5();
	afx_msg void OnBnClickedCheckHex();
	afx_msg void OnBnClickedCheckRox();
	afx_msg void OnBnClickedCheckFam();

	int CLabGenius_PCR_viewerDlg::OpenDbFile(CString path);
	void CLabGenius_PCR_viewerDlg::ReadDbFile();
	void CLabGenius_PCR_viewerDlg::Clear();
	void CLabGenius_PCR_viewerDlg::DataInputToGUI(int table_no, char** args);
	void CLabGenius_PCR_viewerDlg::DrawGraphToValue();

	CListCtrl m_cProtocolList;
	int list_index;

	CString m_argLGFilePath;
	UseRegistry m_Reg;

	sqlite3 *pSQLite3; // SQLite DB 객체 저장 변수
	char    *szErrMsg; // Error 발생시 메세지를 저장하는 변수

	CXYChart m_Chart;
	vector< int > sensorValues_no;
	vector< int > sensorValues_time;
	vector< double > sensorValues_fam;
	vector< double > sensorValues_hex;
	vector< double > sensorValues_rox;
	vector< double > sensorValues_cy5;

	bool filter_flag[4];

	int m_cGraphYMin;
	int m_cGraphYMax;

	CBitmap filter_img[5];
};