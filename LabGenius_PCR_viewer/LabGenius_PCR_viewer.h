
// LabGenius_PCR_viewer.h : PROJECT_NAME ���� ���α׷��� ���� �� ��� �����Դϴ�.
//

#pragma once

#ifndef __AFXWIN_H__
	#error "PCH�� ���� �� ������ �����ϱ� ���� 'stdafx.h'�� �����մϴ�."
#endif

#include "resource.h"		// �� ��ȣ�Դϴ�.


// CLabGenius_PCR_viewerApp:
// �� Ŭ������ ������ ���ؼ��� LabGenius_PCR_viewer.cpp�� �����Ͻʽÿ�.
//

class CLabGenius_PCR_viewerApp : public CWinApp
{
public:
	CLabGenius_PCR_viewerApp();

// �������Դϴ�.
public:
	virtual BOOL InitInstance();

// �����Դϴ�.

	DECLARE_MESSAGE_MAP()
};

extern CLabGenius_PCR_viewerApp theApp;