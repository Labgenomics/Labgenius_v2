
// LabGenius_PCR_viewerDlg.h : ��� ����
//

#pragma once

#include ".\Lib\sqlite3.h"
#include ".\Lib\UseRegistry.h"
#include "Chart.h"

// CLabGenius_PCR_viewerDlg ��ȭ ����
class CLabGenius_PCR_viewerDlg : public CDialogEx
{
// �����Դϴ�.
public:
	CLabGenius_PCR_viewerDlg(CWnd* pParent = NULL);	// ǥ�� �������Դϴ�.
	virtual ~CLabGenius_PCR_viewerDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_LABGENIUS_PCR_VIEWER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �����Դϴ�.


// �����Դϴ�.
protected:
	HICON m_hIcon;

	// ������ �޽��� �� �Լ�
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

	sqlite3 *pSQLite3; // SQLite DB ��ü ���� ����
	char    *szErrMsg; // Error �߻��� �޼����� �����ϴ� ����

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