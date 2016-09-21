
// LabGenius_PCR_viewerDlg.cpp : 구현 파일
//


#include "stdafx.h"
#include "LabGenius_PCR_viewer.h"
#include "LabGenius_PCR_viewerDlg.h"
#include "afxdialogex.h"

#define INFORMATION			"0"
#define MAGNETO_PROTOCOL	"1"
#define PCR_PROTOCOL		"2"
#define PCR_CONSTANT		"3"
#define TEMPERATURE_PID		"4"
#define PHOTODIODE			"5"
#define TEMPER_PD_RAW		"6"

#ifdef _DEBUG
#pragma comment(linker, "/entry:WinMainCRTStartup /subsystem:console")
#define new DEBUG_NEW
#endif


// CLabGenius_PCR_viewerDlg 대화 상자

CLabGenius_PCR_viewerDlg::~CLabGenius_PCR_viewerDlg()
{
	// 생성한 객체를 소멸자에서 제거해준다.
	if (pSQLite3 != NULL)
		sqlite3_close(pSQLite3);
	if (szErrMsg != NULL)
		sqlite3_free(szErrMsg);
}

CLabGenius_PCR_viewerDlg::CLabGenius_PCR_viewerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLabGenius_PCR_viewerDlg::IDD, pParent)
	, list_index(0)
	, m_cGraphYMin(0)
	, m_cGraphYMax(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	
	pDlg = this;

	resource[0] = IDC_EDIT_EXPDATE;
	resource[1] = IDC_EDIT_TASTSTARTTIME;
	resource[2] = IDC_EDIT_TESKENDTIME;
	resource[3] = IDC_EDIT_TOTALTIME;
	resource[4] = IDC_EDIT_PCRTIME;
	resource[5] = IDC_EDIT_EXTRATION_TIME;
	resource[6] = IDC_CHECK_FAM;
	resource[7] = IDC_CHECK_HEX;
	resource[8] = IDC_CHECK_ROX;
	resource[9] = IDC_CHECK_CY5;

	img_resource[0] = IDC_STATIC_FAM;
	img_resource[1] = IDC_STATIC_HEX;
	img_resource[2] = IDC_STATIC_ROX;
	img_resource[3] = IDC_STATIC_CY5;
}

void CLabGenius_PCR_viewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_PCR_PROTOCOL, m_cProtocolList);
}

BEGIN_MESSAGE_MAP(CLabGenius_PCR_viewerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_FILE_CSVEXPORT, &CLabGenius_PCR_viewerDlg::OnFileCsvexport)
	ON_COMMAND(ID_FILE_F_OPEN, &CLabGenius_PCR_viewerDlg::OnFileFOpen)
	ON_BN_CLICKED(IDC_CHECK_CY5, &CLabGenius_PCR_viewerDlg::OnBnClickedCheckCy5)
	ON_BN_CLICKED(IDC_CHECK_HEX, &CLabGenius_PCR_viewerDlg::OnBnClickedCheckHex)
	ON_BN_CLICKED(IDC_CHECK_ROX, &CLabGenius_PCR_viewerDlg::OnBnClickedCheckRox)
	ON_BN_CLICKED(IDC_CHECK_FAM, &CLabGenius_PCR_viewerDlg::OnBnClickedCheckFam)
END_MESSAGE_MAP()


// CLabGenius_PCR_viewerDlg 메시지 처리기

BOOL CLabGenius_PCR_viewerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.
	

	// 윈도우 레지스트리 확장자 및 확장자 실행 응용프로그램 등록
	CString strValue;
	DWORD dwSize = 1024;
	CRegKey regKey;
	if (regKey.Open(HKEY_CLASSES_ROOT, L".labg") == ERROR_SUCCESS)
	{
		if (regKey.QueryStringValue(NULL, strValue.GetBuffer(dwSize), &dwSize) == ERROR_SUCCESS)
		{
			strValue.ReleaseBuffer();
			regKey.Close();
		}
	}
	else
	{
		if (regKey.Create(HKEY_CLASSES_ROOT, L".labg") == ERROR_SUCCESS)
		{
			regKey.SetStringValue(NULL, L"labgenius_auto");
			regKey.Close();
		}
		if (regKey.Create(HKEY_CLASSES_ROOT, L"labgenius_auto\\Shell\\Open\\Command") == ERROR_SUCCESS)
		{
			regKey.SetStringValue(NULL, L"C:\\LabGenomics\\LabGenius\\LabGenius_PCR_viewer.exe \"%1\"");
			regKey.Close();
		}
	}
	

	// 메뉴 UI 초기화 및 생성
	menu.LoadMenuW(IDR_MENU1);
	SetMenu(&menu);


	// PCR 프로토콜 리스트 뷰 UI 초기화
	CFont font;
	CRect rect;
	CString labels[3] = { L"No", L"Temp.", L"Time" };
	font.CreatePointFont(100, L"Arial", NULL);
	m_cProtocolList.SetFont(&font);
	m_cProtocolList.GetClientRect(&rect);
	for (int i = 0; i<3; ++i)
		m_cProtocolList.InsertColumn(i, labels[i], LVCFMT_CENTER, (rect.Width() / 3));


	// GUI 및 변수 초기화
	Clear();

	filter_img[0].LoadBitmapW(IDB_BITMAP_FAM);
	filter_img[1].LoadBitmapW(IDB_BITMAP_HEX);
	filter_img[2].LoadBitmapW(IDB_BITMAP_ROX);
	filter_img[3].LoadBitmapW(IDB_BITMAP_CY5);
	filter_img[4].LoadBitmapW(IDB_BITMAP_OFF);

	
	// 실행 매개변수 확인 및 초기화
	m_argLGFilePath.Format(L"%s", theApp.m_lpCmdLine);
	m_argLGFilePath.Remove('"');
	if (m_argLGFilePath.Compare(L"") == 0)
	{
		// 매개변수 없으면 바로 File open dialog 출력
		OnFileFOpen();
	}
	else
	{
		// 여기에 매개변수로 들어온 파일 처리
		OpenDbFile(m_argLGFilePath);
		ReadDbFile();
	}
	

	// Chart UI Settings
	CAxis *axis;
	axis = m_Chart.AddAxis(kLocationBottom);
	axis->SetTitle(L"PCR Cycles");
	axis->SetRange(0, 40);
	axis = m_Chart.AddAxis(kLocationLeft);
	axis->SetTitle(L"Sensor Value");
	axis->SetRange(m_cGraphYMin, m_cGraphYMax);

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CLabGenius_PCR_viewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	CDialogEx::OnSysCommand(nID, lParam);
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CLabGenius_PCR_viewerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CPaintDC dc(this); // device context for painting
		// TODO: 여기에 메시지 처리기 코드를 추가합니다.
		// 그리기 메시지에 대해서는 CDialog::OnPaint()을(를) 호출하지 마십시오.

		CRect graphRect;

		int oldMode = dc.SetMapMode(MM_LOMETRIC);

//						  x		y		width	height
		graphRect.SetRect(6,	130,	655,	435);

		dc.DPtoLP((LPPOINT)&graphRect, 2);

		CDC *dc2 = CDC::FromHandle(dc.m_hDC);
		m_Chart.OnDraw(dc2, graphRect, graphRect);

		dc.SetMapMode(oldMode);

		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CLabGenius_PCR_viewerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CLabGenius_PCR_viewerDlg::Clear()
{
	SetDlgItemText(IDC_EDIT_EXPDATE, _T("YYYY-MM-DD"));
	SetDlgItemText(IDC_EDIT_TASTSTARTTIME, _T("00:00:00"));
	SetDlgItemText(IDC_EDIT_TESKENDTIME, _T("00:00:00"));
	SetDlgItemText(IDC_EDIT_TOTALTIME, _T("00:00:00"));
	SetDlgItemText(IDC_EDIT_PCRTIME, _T("00:00:00"));
	SetDlgItemText(IDC_EDIT_EXTRATION_TIME, _T("00:00:00"));

	GetDlgItem(IDC_CHECK_FAM)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_ROX)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_HEX)->EnableWindow(FALSE);
	GetDlgItem(IDC_CHECK_CY5)->EnableWindow(FALSE);

	((CStatic*)GetDlgItem(IDC_STATIC_FAM))->SetBitmap((HBITMAP)filter_img[4]);
	((CStatic*)GetDlgItem(IDC_STATIC_HEX))->SetBitmap((HBITMAP)filter_img[4]);
	((CStatic*)GetDlgItem(IDC_STATIC_ROX))->SetBitmap((HBITMAP)filter_img[4]);
	((CStatic*)GetDlgItem(IDC_STATIC_CY5))->SetBitmap((HBITMAP)filter_img[4]);

	m_cProtocolList.DeleteAllItems();
	list_index = 0;

	sensorValues_no.clear();
	sensorValues_time.clear();
	sensorValues_fam.clear();
	sensorValues_hex.clear();
	sensorValues_rox.clear();
	sensorValues_cy5.clear();
	
	sensorValues_fam.push_back(1.0);
	sensorValues_hex.push_back(1.0);
	sensorValues_rox.push_back(1.0);
	sensorValues_cy5.push_back(1.0);
	
}

void CLabGenius_PCR_viewerDlg::OnFileFOpen()
{
	CFileDialog m_fDlg(TRUE, _T("*.labg"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, _T("LabGenius files (*.labg)|*.labg|"), NULL);
	if (m_fDlg.DoModal() == IDOK)
	{
		OpenDbFile( m_fDlg.GetPathName() );
		ReadDbFile();

		CString title_temp;
		title_temp.Format(L"%s - LabGenius PCR viewer", m_fDlg.GetFileName());
		SetWindowText(title_temp);
	}
}

static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
	if (strcmp(argv[0], INFORMATION) == 0)
	{
		pDlg->DataInputToGUI(atoi(INFORMATION), argv);
		
#ifdef _DEBUG
		printf("Table 'INFORMATION'\n");
		for (int i = 0; i < argc; i++)
			printf("\t%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		printf("\n\n");
#endif
	}

	else if (strcmp(argv[0], PCR_PROTOCOL) == 0)
	{
		pDlg->DataInputToGUI(atoi(PCR_PROTOCOL), argv);

#ifdef _DEBUG
		printf("Table 'PCR_PROTOCOL'\n");
		for (int i = 0; i < argc; i++)
			printf("\t%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		printf("\n\n");
#endif
	}

	else if (strcmp(argv[0], PCR_CONSTANT) == 0)
	{
		pDlg->DataInputToGUI(atoi(PCR_CONSTANT), argv);

#ifdef _DEBUG
		printf("Table 'PCR_CONSTANT'\n");
		for (int i = 0; i < argc; i++)
			printf("\t%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		printf("\n\n");
#endif
	}

	else if (strcmp(argv[0], PHOTODIODE) == 0)
	{
		pDlg->DataInputToGUI(atoi(PHOTODIODE), argv);

#ifdef _DEBUG
		printf("Table 'PHOTODIODE'\n");
		for (int i = 0; i < argc; i++)
			printf("\t%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		printf("\n\n");
#endif
	}

/*	else if (strcmp(argv[0], MAGNETO_PROTOCOL) == 0) {}
	else if (strcmp(argv[0], TEMPERATURE_PID) == 0) {}
	else if (strcmp(argv[0], TEMPER_PD_RAW) == 0) {}
*/
	return 0;
};

void CLabGenius_PCR_viewerDlg::DataInputToGUI(int table_no, char** args)
{
	CString tempString = L"";
	int durs = 0;
	int durm = 0;

	switch (table_no)
	{
	case 0:	// Information table

		for (int i = 0; i < 10; i++)	// Repeat the 'resource' array length
		{
			if (i >= 6)
			{
				if (strcmp(args[i + 1], "0") == 0)
				{
					GetDlgItem(resource[i])->EnableWindow(FALSE);
					((CStatic*)GetDlgItem(img_resource[i-6]))->SetBitmap((HBITMAP)filter_img[4]);
					filter_flag[i - 6] = false;
				}
				else
				{
					GetDlgItem(resource[i])->EnableWindow(TRUE);
					((CStatic*)GetDlgItem(img_resource[i - 6]))->SetBitmap((HBITMAP)filter_img[i-6]);
					filter_flag[i - 6] = true;
				}
			}
			else
			{
				SetDlgItemText(resource[i], (CString)args[i + 1]);
			}
		}
		break;

	case 2:	// PCR protocol table
		m_cProtocolList.InsertItem(list_index, (CString)args[1]);
		
		if (((CString)args[1]).CompareNoCase(L"SHOT") != 0)
		{
			m_cProtocolList.SetItemText(list_index, 1, (CString)args[2]);

			durs = atoi(args[3]);
			// 소수점 값을 초로 환산
			durm = durs / 60;
			durs = durs % 60;

			if (((CString)args[1]).CompareNoCase(L"GOTO") == 0)
			{
				tempString.Format(L"%d repeat", atoi(args[3]));
			}
			else if (durs == 0)
			{
				if (durm == 0) tempString.Format(L"∞");
				else tempString.Format(L"%dm", durm);
			}
			else
			{
				if (durm == 0) tempString.Format(L"%ds", durs);
				else tempString.Format(L"%dm %ds", durm, durs);
			}

			m_cProtocolList.SetItemText(list_index, 2, tempString);
			list_index++;
		}
		else
		{
			m_cProtocolList.SetTextBkColor(RGB(240, 200, 250));
			m_cProtocolList.RedrawItems(list_index, list_index);
			list_index++;
		}
		break;

	case 3:	// PCR constant
		m_cGraphYMin = atoi(args[4]);
		m_cGraphYMax = atoi(args[5]);
		break;

	case 5:	// Photodiode table
		sensorValues_no.push_back(atoi(args[1]));
		sensorValues_time.push_back(atoi(args[2]));
		sensorValues_fam.push_back(atof(args[3]));
		sensorValues_hex.push_back(atof(args[4]));
		sensorValues_rox.push_back(atof(args[5]));
		sensorValues_cy5.push_back(atof(args[6]));
		break;
	}
}

void CLabGenius_PCR_viewerDlg::ReadDbFile()
{
	int result;

	result = sqlite3_exec(pSQLite3, "SELECT * FROM INFORMATION", callback, 0, &szErrMsg);

	result = sqlite3_exec(pSQLite3, "SELECT * FROM PCR_CONSTANT", callback, 0, &szErrMsg);

	result = sqlite3_exec(pSQLite3, "SELECT * FROM PCR_PROTOCOL", callback, 0, &szErrMsg);

	result = sqlite3_exec(pSQLite3, "SELECT * FROM PHOTODIODE", callback, 0, &szErrMsg);
	DrawGraphToValue();

	sqlite3_free(szErrMsg);
	sqlite3_close(pSQLite3);
}

int CLabGenius_PCR_viewerDlg::OpenDbFile(CString path)
{
	Clear();

	USES_CONVERSION;
	int result = sqlite3_open(T2A(path), &pSQLite3);
	if (result)
	{
		CString SQLERR;
		SQLERR.Format(L"Can't open file: %s", sqlite3_errmsg(pSQLite3));
		AfxMessageBox(SQLERR);

		sqlite3_close(pSQLite3);
		pSQLite3 = NULL;
	}

	return result;
}

void CLabGenius_PCR_viewerDlg::DrawGraphToValue()
{
	// 기존에 저장된 차트를 지운 후, 
	// 새로 저장한 double 값 vector 에 저장하여
	// 이 값을 기반으로 다시 그림.

	m_Chart.DeleteAllData();

	int size_fam = sensorValues_fam.size();
	int size_hex = sensorValues_hex.size();
	int size_rox = sensorValues_rox.size();
	int size_cy5 = sensorValues_cy5.size();
	double *data_fam = new double[size_fam * 2];
	double *data_hex = new double[size_hex * 2];
	double *data_rox = new double[size_rox * 2];
	double *data_cy5 = new double[size_cy5 * 2];

	if (filter_flag[0] == true)
	{
		int	nDims_fam = 2, dims_fam[2] = { 2, size_fam };
		for (int i = 0; i<size_fam; ++i)
		{
			data_fam[i] = i;
			data_fam[i + size_fam] = sensorValues_fam[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(data_fam, nDims_fam, dims_fam), RGB(0, 0, 255));
	}

	if (filter_flag[1] == true)
	{
		int	nDims_hex = 2, dims_hex[2] = { 2, size_hex };
		for (int i = 0; i < size_hex; ++i)
		{
			data_hex[i] = i;
			data_hex[i + size_hex] = sensorValues_hex[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(data_hex, nDims_hex, dims_hex), RGB(0, 255, 0));
	}

	if (filter_flag[2] == true)
	{
		int	nDims_rox = 2, dims_rox[2] = { 2, size_rox };
		for (int i = 0; i < size_rox; ++i)
		{
			data_rox[i] = i;
			data_rox[i + size_rox] = sensorValues_rox[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(data_rox, nDims_rox, dims_rox), RGB(0, 128, 0));
	}

	if (filter_flag[3] == true)
	{
		int	nDims_cy5 = 2, dims_cy5[2] = { 2, size_cy5 };
		for (int i = 0; i < size_cy5; ++i)
		{
			data_cy5[i] = i;
			data_cy5[i + size_cy5] = sensorValues_cy5[i];
		}
		m_Chart.SetDataColor(m_Chart.AddData(data_cy5, nDims_cy5, dims_cy5), RGB(255, 0, 0));
	}

	InvalidateRect(&CRect(13, 140, 649, 428));
}

void CLabGenius_PCR_viewerDlg::OnFileCsvexport()
{
	CFileDialog ExportDlg(FALSE, _T("csv"), _T("*.csv"), OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR, _T("CSV Files (*.csv)|*.csv|"), NULL);
	ExportDlg.m_ofn.nFilterIndex = 1;

	if (ExportDlg.DoModal() != IDOK) return;

	FILE* fp = NULL;
	TRY
	{
		USES_CONVERSION;
		fopen_s(&fp, T2A(ExportDlg.GetPathName()), "w+");

		if (fp != NULL)
		{
			CString csv_msg;
			csv_msg.Format(_T("%s,%s,%s,%s,%s,%s,,%s,%s,%s,%s,%s,%s\n"), _T("no"), _T("time"), _T("fam"), _T("hex"), _T("rox"), _T("cy5"), _T("date"), _T("start time"), _T("end time"), _T("total time"), _T("total pcr"), _T("total extraction"));

			fprintf(fp, T2A(csv_msg));

			int vector_size = 0;
			if (filter_flag[0])
				vector_size = sensorValues_fam.size();
			else if (filter_flag[1])
				vector_size = sensorValues_hex.size();
			else if (filter_flag[2])
				vector_size = sensorValues_rox.size();
			else if (filter_flag[3])
				vector_size = sensorValues_cy5.size();

			for (int i = 0; i < vector_size-1; i++)
			{
				if (i != 0)
				{
					csv_msg.Format(_T("%d,%d,%f,%f,%f,%f\n"), sensorValues_no[i], sensorValues_time[i], sensorValues_fam[i + 1], sensorValues_hex[i + 1], sensorValues_rox[i + 1], sensorValues_cy5[i + 1]);
				}
				else
				{
					CString str_date;
					CString str_start;
					CString str_end;
					CString str_total;
					CString str_pcr;
					CString str_extraction;

					GetDlgItemText(IDC_EDIT_EXPDATE, str_date);
					GetDlgItemText(IDC_EDIT_TASTSTARTTIME, str_start);
					GetDlgItemText(IDC_EDIT_TESKENDTIME, str_end);
					GetDlgItemText(IDC_EDIT_TOTALTIME, str_total);
					GetDlgItemText(IDC_EDIT_PCRTIME, str_pcr);
					GetDlgItemText(IDC_EDIT_EXTRATION_TIME, str_extraction);

					csv_msg.Format(_T("%d,%d,%f,%f,%f,%f,,%s,%s,%s,%s,%s,%s\n"), sensorValues_no[i], sensorValues_time[i], sensorValues_fam[i+1], sensorValues_hex[i+1], sensorValues_rox[i+1], sensorValues_cy5[i+1], str_date, str_start, str_end, str_total, str_pcr, str_extraction);
				}
				fprintf(fp, T2A(csv_msg));
			}
		}
		else
		{
			AfxMessageBox(_T("The chosen file is open in another program.\n\nClose the file and try again."), MB_ICONERROR);
		}

		fclose(fp);
		fp = NULL;

		CString str_msg;
		str_msg.Format(_T("%s was saved successfully\n%s"), ExportDlg.GetFileName(), ExportDlg.GetPathName().Mid(0, ExportDlg.GetPathName().GetLength() - ExportDlg.GetFileName().GetLength()));
		AfxMessageBox(str_msg, MB_ICONEXCLAMATION);
	}
	CATCH_ALL(e)
	{
		if (fp != NULL)
			fclose(fp);
		THROW_LAST();
	}
	END_CATCH_ALL
	{
		if (fp != NULL)
		fclose(fp);
	}
}

void CLabGenius_PCR_viewerDlg::OnBnClickedCheckCy5()
{
	filter_flag[3] = !filter_flag[3];
	DrawGraphToValue();
}


void CLabGenius_PCR_viewerDlg::OnBnClickedCheckHex()
{
	filter_flag[2] = !filter_flag[2];
	DrawGraphToValue();
}


void CLabGenius_PCR_viewerDlg::OnBnClickedCheckRox()
{
	filter_flag[1] = !filter_flag[1];
	DrawGraphToValue();
}


void CLabGenius_PCR_viewerDlg::OnBnClickedCheckFam()
{
	filter_flag[0] = !filter_flag[0];
	DrawGraphToValue();
}
