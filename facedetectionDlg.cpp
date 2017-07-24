
// facedetectionDlg.cpp : ʵ���ļ�
//
#include "stdafx.h"
#include "facedetection.h"
#include "facedetectionDlg.h"
#include "afxdialogex.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <opencv2/opencv.hpp>
#include "facedetect-dll.h"
#include <opencv2/core/core.hpp>  
#include <opencv2/imgproc/imgproc.hpp>  
#include <opencv2/highgui/highgui.hpp>
#include<string>
#include <afxwin.h>
#include "BCnnAPI.h"
#include <io.h>
#include <fcntl.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define DETECT_BUFFER_SIZE 0x20000
#define threshold_distance 1.9
#pragma comment(lib,"libfacedetect-x64.lib")

//define the buffer size. Do not change the size!

using namespace cv;
using namespace std;
CString FOLK[57];
CRITICAL_SECTION cs;
CRITICAL_SECTION cs1;
//Mat resize_roi_of_card;
//Mat resize_roi_of_camera;
bool brun;
bool brun1;
// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���
//unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CAboutDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CfacedetectionDlg �Ի���



CfacedetectionDlg::CfacedetectionDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_FACEDETECTION_DIALOG, pParent)
, match_result(_T(""))
, similarity(0)
, match_of_distance(0)
, m_strName(_T(""))
, m_strSex(_T(""))
, m_strFolk(_T(""))
, m_strBirth(_T(""))
, m_strAddr(_T(""))
, m_strIdnum(_T(""))
, m_strDep(_T(""))
, m_strBegin(_T(""))
, m_strEnd(_T(""))
, m_strTimer(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}
LRESULT CfacedetectionDlg::OnUpdateData(WPARAM wParam, LPARAM lParam)
{
	UpdateData(wParam);
	return 0;
}
void CfacedetectionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_MATCH_RESULT, match_result);
	DDX_Text(pDX, IDC_SIMILARITY, similarity);
	DDV_MinMaxDouble(pDX, similarity, 0.0, 1.0);
	DDX_Text(pDX, IDC_MATCH_DISTANCE, match_of_distance);
	DDV_MinMaxDouble(pDX, match_of_distance, 0, 2.0);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_SEX, m_strSex);
	DDX_Text(pDX, IDC_EDIT_FOLK, m_strFolk);
	DDX_Text(pDX, IDC_EDIT_BIRTH, m_strBirth);
	DDX_Text(pDX, IDC_EDIT_ADDR, m_strAddr);
	DDX_Text(pDX, IDC_EDIT_IDNUM, m_strIdnum);
	DDX_Text(pDX, IDC_EDIT_DEP, m_strDep);
	DDX_Text(pDX, IDC_EDIT_DEGIN, m_strBegin);
	DDX_Text(pDX, IDC_EDIT_END, m_strEnd);
	DDX_Control(pDX, IDC_CARD, m_ctPicCard);
	DDX_Control(pDX, IDC_roi_of_card, m_ctCardroi);
	DDX_Text(pDX, IDC_TIMER, m_strTimer);
}

BEGIN_MESSAGE_MAP(CfacedetectionDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CfacedetectionDlg::OnBnClickedButton1)
    ON_BN_CLICKED(IDC_BUTTON2_EXIT, &CfacedetectionDlg::OnBnClickedButton2Exit)
    ON_MESSAGE(WM_UPDATEDATA, OnUpdateData)
    ON_WM_TIMER()
END_MESSAGE_MAP()


// CfacedetectionDlg ��Ϣ�������
void InitConsoleWindow()
{
	AllocConsole();
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	int hCrt = _open_osfhandle((long)handle, _O_TEXT);
	FILE * hf = _fdopen(hCrt, "w");
	*stdout = *hf;
}

BOOL CfacedetectionDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�
	InitConsoleWindow();  // add  
	printf("str = %s\n ", "Debug output goes to terminal\n");
	freopen("log.txt", "w", stdout);
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

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	//m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��
	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	SetTimer(1, 1000, NULL);
	FOLK[1] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[2] = "�ɹ���";
	FOLK[3] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[4] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[5] = "ά�����";
	FOLK[6] = "����";
	FOLK[7] = "����";
	FOLK[8] = "׳��";
	FOLK[9] = "����";
	FOLK[10] = "����";
	FOLK[11] = "����";
	FOLK[12] = "����";
	FOLK[13] = "������";//ע�� ����������� �����꣬����ֻ������
	FOLK[14] = "������";
	FOLK[15] = "��������";
	FOLK[16] = "����";//ע�� ����������� �����꣬����ֻ������
	FOLK[17] = "����";
	FOLK[18] = "���";
	FOLK[19] = "��ɽ��";//ע�� ����������� �����꣬����ֻ������
	FOLK[20] = "������";
	FOLK[21] = "ˮ��";
	FOLK[22] = "������";//ע�� ����������� �����꣬����ֻ������
	FOLK[23] = "������";
	FOLK[24] = "������";
	FOLK[25] = "���Ӷ���";
	FOLK[26] = "������";
	FOLK[27] = "Ǽ��";
	FOLK[28] = "������";
	FOLK[29] = "������";
	FOLK[30] = "ë����";
	FOLK[31] = "������";
	FOLK[32] = "������";
	FOLK[33] = "��������";
	FOLK[34] = "ŭ��";
	FOLK[35] = "���α����";
	FOLK[36] = "����˹��";
	FOLK[37] = "���¿���";
	FOLK[38] = "�°���";
	FOLK[39] = "������";
	FOLK[40] = "ԣ����";
	FOLK[41] = "������";
	FOLK[42] = "���״���";
	FOLK[43] = "������";
	FOLK[44] = "�Ű���";
	FOLK[45] = "�����";
	FOLK[46] = "��ŵ��";
	FOLK[47] = "������";
	FOLK[48] = "������";
	FOLK[49] = "������";
	FOLK[50] = "��������";
	FOLK[51] = "������";
	FOLK[52] = "����";
	FOLK[53] = "�¶�������";
	FOLK[54] = "������";
	FOLK[55] = "����";
	FOLK[56] = "����";
	CString ss;
	ss.Format("%s", "���������֤ϵͳ");
	CDialog::SetWindowText(ss);
	CFont *m_Font;
	CFont *m_Font1;
	m_Font = new CFont;
	m_Font1 = new CFont;
	m_Font->CreateFont(50, 20, 0, 0, 100,
		FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "����");
	m_Font1->CreateFont(20, 10, 0, 0, 100,
		FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
		CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, FF_SWISS, "New Times Rom");
	CEdit *m_Edit = (CEdit *)GetDlgItem(IDC_MATCH_RESULT);
	m_Edit->SetFont(m_Font, FALSE);
	GetDlgItem(IDC_MATCH_RESULT)->SetFont(m_Font);

	CStatic *m_Static = (CStatic *)GetDlgItem(IDC_STATIC_RESULT);
	m_Static->SetFont(m_Font1, FALSE);
	GetDlgItem(IDC_STATIC_RESULT)->SetFont(m_Font1);
	//delete m_Font; //�����У����������С����
	CRect rect;
	CRect rect1;
	GetDlgItem(IDC_roi_of_face)->GetWindowRect(&rect);  //IDC_WAVE_DRAWΪPicture Control��ID  
	GetDlgItem(IDC_roi_of_card)->GetWindowRect(&rect1);
	ScreenToClient(&rect);
	ScreenToClient(&rect1);
	GetDlgItem(IDC_roi_of_face)->MoveWindow(rect.left, rect.top, 102, 126 , true);   //�̶�
	GetDlgItem(IDC_roi_of_card)->MoveWindow(rect1.left, rect1.top, 102, 126, true);
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}
void CfacedetectionDlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CfacedetectionDlg::OnPaint()
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
HCURSOR CfacedetectionDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CAboutDlg::OnBnClickedOk()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	CDialogEx::OnOK();
}

void CfacedetectionDlg::OnBnClickedLoadPicBut()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	/*CBitmap bitmap;  // CBitmap�������ڼ���λͼ   
	HBITMAP hBmp;    // ����CBitmap���ص�λͼ�ľ��   

	bitmap.LoadBitmap(IDB_BITMAP1);  // ��λͼIDB_BITMAP1���ص�bitmap   
	hBmp = (HBITMAP)bitmap.GetSafeHandle();  // ��ȡbitmap����λͼ�ľ��   
	card.SetBitmap(hBmp); */   // ����ͼƬ�ؼ�m_jzmPicture��λͼͼƬΪIDB_BITMAP1   
}
void detectAndDisplay(unsigned char *pBuffer,Mat frame,Mat roi,Mat resize_roi)
{  
	int resize_height = 102;
	int resize_width = 126;
	Mat gray;
	cvtColor(frame, gray, CV_BGR2GRAY);
	int * pResults = NULL;
	//unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
	pResults = facedetect_frontal(pBuffer,(unsigned char*)(gray.ptr(0)), gray.cols, gray.rows, gray.step,
		1.2f, 5, 24);
	printf("%d faces detected.\n", (pResults ? *pResults : 0));//�ظ�����  
															   //print the detection results  
	for (int i = 0; i < (pResults ? *pResults : 0); i++)
	{

		short * p = ((short*)(pResults + 1)) + 142 * i;
		int x = p[0];
		int y = p[1];
		int w = p[2];
		int h = p[3];
		int neighbors = p[4];
		printf("face_rect=[%d, %d, %d, %d], neighbors=%d\n", x, y, w, h, neighbors);
		Point left(x, y);
		Point right(x + w, y + h);
		rectangle(frame, left, right, Scalar(230, 255, 0), 4);
		Rect roi(x, y, w, h);
		Mat roi_of_frame = frame(roi);
		resize(roi_of_frame, resize_roi, Size(resize_width, resize_height), (0, 0), (0, 0), INTER_LINEAR);
		imshow("roi_of_face", resize_roi);
		}
	//imshow("face", frame);
	free(pBuffer);
}

void CfacedetectionDlg::OnBnClickedButton1()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	namedWindow("face", WINDOW_AUTOSIZE);
	HWND hWnd = (HWND)cvGetWindowHandle("face");
	HWND hParent = ::GetParent(hWnd);
	::SetParent(hWnd, GetDlgItem(IDC_face)->m_hWnd);
	::ShowWindow(hParent, SW_HIDE);

	namedWindow("roi_of_face", WINDOW_AUTOSIZE);
	HWND hWnd1 = (HWND)cvGetWindowHandle("roi_of_face");
	HWND hParent1 = ::GetParent(hWnd1);
	::SetParent(hWnd1, GetDlgItem(IDC_roi_of_face)->m_hWnd);
	::ShowWindow(hParent1, SW_HIDE);

	namedWindow("roi_of_card", WINDOW_AUTOSIZE);
	HWND hWnd2 = (HWND)cvGetWindowHandle("roi_of_card");
	HWND hParent2 = ::GetParent(hWnd2);
	::SetParent(hWnd2, GetDlgItem(IDC_roi_of_card)->m_hWnd);
	::ShowWindow(hParent2, SW_HIDE);

	namedWindow("card", WINDOW_AUTOSIZE);
	HWND hWnd3 = (HWND)cvGetWindowHandle("card");
	HWND hParent3 = ::GetParent(hWnd3);
	::SetParent(hWnd3, GetDlgItem(IDC_CARD)->m_hWnd);
	::ShowWindow(hParent3, SW_HIDE);
	///////////////////////////////////////////
	Mat camera_frame;//from camera
	Mat roi_of_camera;//roi of camera_frame
	Mat resize_roi_of_camera;//resize camera roi
	int resize_height = 126;
	int resize_width = 102;

	InitializeCriticalSection(&cs);//��ʼ���ṹ����

	pThread = AfxBeginThread(ThreadFunc, this);
	if (pThread == NULL)
	{
		AfxMessageBox("���ؿ���Ϣ�߳�����ʧ��!", MB_OK | MB_ICONERROR);
		return;
	}
	pThread1 = AfxBeginThread(ThreadFunc1, this);
	if (pThread1 == NULL)
	{
		AfxMessageBox("������ͷ�߳�����ʧ��!", MB_OK | MB_ICONERROR);
		return;
	}
	pThread2 = AfxBeginThread(ThreadFunc2, this);
	if (pThread2 == NULL)
	{
		AfxMessageBox("�����߳�����ʧ��!", MB_OK | MB_ICONERROR);
		return;
	}
}

void CfacedetectionDlg::OnBnClickedButton2Exit()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	//CDialog::OnOK();
	OnCancel();
	exit(0);
}

UINT ThreadFunc(LPVOID pParm) {
	while (true) {
		//EnterCriticalSection(&cs);
		CfacedetectionDlg *pDlg1 = (CfacedetectionDlg*)pParm;
		Mat card_frame; //from card
		Mat roi_of_card;//roi of card
		Mat resize_roi_of_card;//resize card roi
		int resize_height = 126;
		int resize_width = 102;
		Mat card_image;
		CFile f_name;
		char byName[32], bySex[4], byFolk[6], byBirth[18], byAddr[72], byIDNum[38], byDep[32], byBegin[18], byEnd[18];

		//
		if (f_name.Open("C:/Users/10838/Documents/Visual Studio 2013/Projects/ss628/Debug/wz.txt", CFile::modeRead))
		{

			f_name.SeekToBegin();
			f_name.Read(&byName, 30);
			byName[30] = 0x0; byName[31] = 0x0;
			f_name.Read(&bySex, 2);
			bySex[2] = 0x0; bySex[3] = 0x0;
			f_name.Read(&byFolk, 4);
			byFolk[4] = 0x0; byFolk[5] = 0x0;
			f_name.Read(&byBirth, 16);
			byBirth[16] = 0x0; byBirth[17] = 0x0;
			f_name.Read(&byAddr, 70);
			byAddr[70] = 0x0; byAddr[71] = 0x0;
			f_name.Read(&byIDNum, 36);
			byIDNum[36] = 0x0; byIDNum[37] = 0x0;
			f_name.Read(&byDep, 30);
			byDep[30] = 0x0; byDep[31] = 0x0;
			f_name.Read(&byBegin, 16);
			byBegin[16] = 0x0; byBegin[17] = 0x0;
			f_name.Read(&byEnd, 16);
			byEnd[16] = 0x0; byEnd[17] = 0x0;
			pDlg1->m_strName = ::SysAllocString((LPCWSTR)&byName);
			pDlg1->m_strName.TrimRight();
			pDlg1->m_strSex = ::SysAllocString((LPCWSTR)&bySex); //(CW2A((LPCWSTR)&bySex));
			pDlg1->m_strSex.TrimRight();
			pDlg1->m_strFolk = ::SysAllocString((LPCWSTR)&byFolk);
			pDlg1->m_strFolk.TrimRight();
			pDlg1->m_strBirth = ::SysAllocString((LPCWSTR)&byBirth);
			pDlg1->m_strBirth.TrimRight();
			pDlg1->m_strAddr = ::SysAllocString((LPCWSTR)&byAddr);
			pDlg1->m_strAddr.TrimRight();
			pDlg1->m_strIdnum = ::SysAllocString((LPCWSTR)&byIDNum);
			pDlg1->m_strIdnum.TrimRight();
			pDlg1->m_strDep = ::SysAllocString((LPCWSTR)&byDep);
			pDlg1->m_strDep.TrimRight();
			pDlg1->m_strBegin = ::SysAllocString((LPCWSTR)&byBegin);
			pDlg1->m_strBegin.TrimRight();
			pDlg1->m_strEnd = ::SysAllocString((LPCWSTR)&byEnd);
			pDlg1->m_strEnd.TrimRight();

			if (pDlg1->m_strSex == "1")
			{
				pDlg1->m_strSex = "��";
			}
			else
			{
				pDlg1->m_strSex = "Ů";
			}
			pDlg1->m_strBirth.Insert(4, "-");
			pDlg1->m_strBirth.Insert(7, "-");
			pDlg1->m_strBegin.Insert(4, "-");
			pDlg1->m_strBegin.Insert(7, "-");
			pDlg1->m_strEnd.Insert(4, "-");
			pDlg1->m_strEnd.Insert(7, "-");
			pDlg1->m_strFolk = FOLK[atoi(pDlg1->m_strFolk.GetBuffer(pDlg1->m_strFolk.GetLength()))];
			pDlg1->UpdateData(FALSE);
			f_name.Close();
		}
		    EnterCriticalSection(&cs);
		    card_frame = imread("C:/Users/10838/Documents/Visual Studio 2013/Projects/ss628/Debug/zp.bmp");
			imshow("card", card_frame);
			Mat gray1;
			cvtColor(card_frame, gray1, CV_BGR2GRAY);
			int * pResults = NULL;
			unsigned char * pBuffer1 = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
			pResults = facedetect_frontal(pBuffer1, (unsigned char*)(gray1.ptr(0)), gray1.cols, gray1.rows, gray1.step,
				1.2f, 2, 48);//5 24
		//	printf("%d faces detected.\n", (pResults ? *pResults : 0));//�ظ�����  													   //print the detection results  %
			for (int i = 0; i < (pResults ? *pResults : 0); i++)
			{
				short * p = ((short*)(pResults + 1)) + 142 * i;
				int x = p[0];
				int y = p[1];
				int w = p[2];
				int h = p[3];
				//int neighbors = p[4];

				//	printf("face_rect=[%d, %d, %d, %d], neighbors=%d\n", x, y, w, h, neighbors);
				//	Point left(x, y);
				//	Point right(x + w, y + h);
				//	rectangle(card_frame, left, right, Scalar(0, 255, 0), 2);//1,8,0
				Rect roi(x, y, (w + x > 102 ? (102 - x) : w), (y + h > 126 ? (126 - y) : h));
				//Rect roi(x, y, w, h);
				roi_of_card = card_frame(roi);
				roi_of_card = card_frame(roi).clone();
				resize(roi_of_card, resize_roi_of_card, Size(resize_width, resize_height), (0, 0), (0, 0), INTER_LINEAR);
				if (resize_roi_of_card.rows != 0)
				{
					imshow("roi_of_card", resize_roi_of_card);
					imwrite("card.bmp", resize_roi_of_card);
					LeaveCriticalSection(&cs);
					brun = 1;
				}
				else
				{
					continue;
				}
			}
			free(pBuffer1);
		//LeaveCriticalSection(&cs);
		Sleep(100);
	}	
	ExitThread(0);
	return 0;
}

UINT ThreadFunc1(LPVOID pParm) {
	Mat camera_frame;//from camera
	Mat roi_of_camera;//roi of camera_frame
	Mat resize_roi_of_camera;//resize camera roi
	int resize_height = 126;
	int resize_width = 102;
	VideoCapture cap(0);
	if (!cap.isOpened()) {

		// MessageBox(hWnd,"Cannot open camera!","camera", MB_OKCANCEL);
		//	return -1 ;
	}
	//	imshow("face", frame);
	for (;;) {
		//EnterCriticalSection(&cs);//����
		cap >> camera_frame;
			Mat gray;
			cvtColor(camera_frame, gray, CV_BGR2GRAY);
			int * pResults = NULL;
			unsigned char * pBuffer = (unsigned char *)malloc(DETECT_BUFFER_SIZE);
			pResults = facedetect_frontal(pBuffer, (unsigned char*)(gray.ptr(0)), gray.cols, gray.rows, gray.step,
				1.2f, 2, 48);
			printf("%d faces detected.\n", (pResults ? *pResults : 0));//�ظ�����  
			//print the detection results  
			if ((pResults ? *pResults : 0) != 0) {
				int x=0, y=0, w=0, h=0;
				for (int i = 0; i < (pResults ? *pResults : 0); i++)
				{
					short * p = ((short*)(pResults + 1)) + 142 * i;
					int x1 = p[0];
					int y1 = p[1];
					int w1 = p[2];
					int h1 = p[3];
					if (i == 0) {
						w = w1;
						h = h1;
						x = x1;
						y = y1;
					}
					int neighbors = p[4];
					if (w1*h1 >= w*h) {
						w = w1;
						h = h1;
						x = x1;
						y = y1;
					}
				}
				printf("face_rect=[%d, %d, %d, %d]\n", x, y, w, h);
				Point left(x, y);
				Point right(x + w, y + h);
				rectangle(camera_frame, left, right, Scalar(0, 255, 0), 2);
				//imshow("face", camera_frame);
				Rect roi(x, y, (w+x>640?(640-x):w), (y+h>480?(480-y):h));
				roi_of_camera = camera_frame(roi);
				roi_of_camera = camera_frame(roi).clone();
				resize(roi_of_camera, resize_roi_of_camera, Size(resize_width, resize_height), (0, 0), (0, 0), INTER_LINEAR);
				//imshow("roi_of_face", resize_roi_of_camera);
				//imwrite("camera.jpg", resize_roi_of_camera);
				if (resize_roi_of_camera.rows!=0 && roi_of_camera.rows!=0)
				{
					imshow("roi_of_face", resize_roi_of_camera);
					EnterCriticalSection(&cs);
					imwrite("camera.jpg", resize_roi_of_camera);
					LeaveCriticalSection(&cs);
					brun1 = 1;
				}
				else
				{
					//MessageBox("û�м�⵽����");
				continue;
				}
			}
			free(pBuffer);	
		imshow("face", camera_frame);
		int delay = 30; // ms  
		int key = waitKey(delay);
		if (27 == key || 'Q' == key || 'q' == key)
			break;
		//LeaveCriticalSection(&cs);
		}
	ExitThread(0);
	return 0;
}


UINT ThreadFunc2(LPVOID pParm) {
	//string param_file = "C:/zhong/model/param_model.xml";
	//INIT_BCNN_MODEL(param_file);
	CfacedetectionDlg *pDlg = (CfacedetectionDlg*)pParm;
	pDlg->my_accept();
	ExitThread(0);
	return 0;

}
void  CfacedetectionDlg::my_accept(void)
{
	string param_file = "C:/zhong/model/param_model.xml";
	INIT_BCNN_MODEL(param_file);
	Mat card;
	Mat camera;
	while (1) {	
		if (brun == 1 && brun1 == 1) {
			EnterCriticalSection(&cs);
			card = imread("card.bmp");
			camera = imread("camera.jpg");
			LeaveCriticalSection(&cs);
			if (!camera.empty() && !card.empty())
			{
				float distance = bcnn_compare_objects(card, camera, "fc9_1", EUCLIDEAN);
				match_of_distance = distance;
				if (distance < threshold_distance)
				{
					match_result = "����";
					similarity = (1 - (threshold_distance - distance) / threshold_distance) * 100;
				}
				else {
					match_result = "�Ǳ���";
					similarity = -(distance - threshold_distance) / threshold_distance * 100;
				}
			}
			else
			{
				continue;
			}
		}
		else {
			continue;

		}
	}		
			//UpdateData(false);
		SendMessage(WM_UPDATEDATA, FALSE);
		//LeaveCriticalSection(&cs1);
}
void CfacedetectionDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: �ڴ������Ϣ�����������/�����Ĭ��ֵ
	CString str;
	CTime t = CTime::GetCurrentTime();
	//m_strTimer.SetPaneText(1, t.Format(_T("%Y��%m��%d�� ����%w %H:%M:%S")));
	m_strTimer = t.Format(_T("%Y��%m��%d�� ����%w %H:%M:%S"));
	CDialog::OnTimer(nIDEvent);
	CDialogEx::OnTimer(nIDEvent);
	CDialogEx::OnTimer(nIDEvent);
	CDialogEx::OnTimer(nIDEvent);
	UpdateData(false);
}
int number = 0;

void thread() {

	while (true) {
		//
		//
		number++;

	}
}

void thread1() {
	while (1) {
		number++;
	}
}
