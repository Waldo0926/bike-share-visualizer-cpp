
// 20200400230-温硕勋Dlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "20200400230-温硕勋.h"
#include "20200400230-温硕勋Dlg.h"
#include "afxdialogex.h"
#include<math.h>
#include "HelpDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//定义导入文件时需要用到的变量
CStringArray ID;
CStringArray Address;
CStringArray TD;
CStringArray DIS;
CStringArray Status;
CStringArray Latitude;
CStringArray Longitude;
CStringArray StationName;
int flag1 = 0;
int InServiceStationNum = 0;
int NotInServiceStationNum = 0;
int InServiceDocks = 0;
int NotInServiceDocks = 0;
int TotalStationNum = 0;
int TotalDocksNum = 0;



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMy20200400230温硕勋Dlg 对话框



CMy20200400230温硕勋Dlg::CMy20200400230温硕勋Dlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_MY20200400230_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy20200400230温硕勋Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, StationList, SL);
	DDX_Control(pDX, IDTTSN, TTSN);
	DDX_Control(pDX, IDISSN, ISSN);
	DDX_Control(pDX, IDNISSN, NISSN);
	DDX_Control(pDX, IDISDN, ISDN);
	DDX_Control(pDX, IDNISDN, NISDN);
	DDX_Control(pDX, IDStationID, ShowID);
	DDX_Control(pDX, IDStationAddress, ShowAddress);
	DDX_Control(pDX, IDTotalDocks, ShowTotalDocks);
	DDX_Control(pDX, IDEnableDocks, ShowEnableDocks);
	DDX_Control(pDX, IDLongitude, LTT);
	DDX_Control(pDX, IDLatitude, LATT);
	DDX_Control(pDX, StationSearchBox, SSB);
	DDX_Text(pDX, IDSearchStation, IDSS);
	DDX_Control(pDX, IDMAP, MAP);
	DDX_Control(pDX, UsersBox, Users);
	DDX_Control(pDX, CheckButton, Check);
	DDX_Control(pDX, IDDistance, Distance);
}

BEGIN_MESSAGE_MAP(CMy20200400230温硕勋Dlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CMy20200400230温硕勋Dlg::OnBnClickedButton1)
	ON_BN_CLICKED(StationSearchButton, &CMy20200400230温硕勋Dlg::OnBnClickedStationsearchbutton)
	ON_BN_CLICKED(IDSearchStationButton, &CMy20200400230温硕勋Dlg::OnBnClickedSearchstationbutton)
	ON_BN_CLICKED(PaintAll, &CMy20200400230温硕勋Dlg::OnBnClickedPaintall)
	ON_BN_CLICKED(IDClear, &CMy20200400230温硕勋Dlg::OnBnClickedClear)
	ON_BN_CLICKED(CheckButton, &CMy20200400230温硕勋Dlg::OnBnClickedCheckbutton)
	ON_BN_CLICKED(IDC_BUTTON4, &CMy20200400230温硕勋Dlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON5, &CMy20200400230温硕勋Dlg::OnBnClickedButton5)
END_MESSAGE_MAP()


// CMy20200400230温硕勋Dlg 消息处理程序

BOOL CMy20200400230温硕勋Dlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CMy20200400230温硕勋Dlg::OnSysCommand(UINT nID, LPARAM lParam)
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
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMy20200400230温硕勋Dlg::OnPaint()
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
HCURSOR CMy20200400230温硕勋Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMy20200400230温硕勋Dlg::OnBnClickedButton1()
{
	
	CString defaultDir = L"D:\\";  // 默认打开文件路径
	CString fileName = L"";  // 默认打开文件名
	CString Filter = L"文件(*.txt; *.csv)|*.txt; *.csv;||";  // 文件过滤类型
	CFileDialog importfiledlg(TRUE, defaultDir, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Filter, NULL);
	INT_PTR result = importfiledlg.DoModal();  // 弹出导入框并记录导入结果
	if (result == IDOK)
	{
		fileName = importfiledlg.GetPathName();
		CStdioFile f;
		CFileException e;
		if (!f.Open(fileName, CFile::modeRead, &e))
		{
			TRACE(_T("您好！文件无法打开，请检查后重试。 %d\n"), e.m_cause);
		}
			CString str = NULL;
			f.ReadString(str);
			int i = 0;
			while (str != "" && i++ < 589)
			{
				SL.AddString(str);  // 将导入的所有数据显示在站点信息总览中
				f.ReadString(str);
			}
			int pos = 0;
			int next_pos = 0;
			int line = 0;
			int case_id = 0;
			CString temp;
			CString s;
			while (line < 589)
			{
				// 数据初始化
				case_id = 0;
				pos = 0;
				next_pos = 0;
				ASSERT(line < 589);  // 行数超过589时报错
				SL.GetText(line, s);
				while (case_id < 8)
				{
					next_pos = s.Find(',', pos);  // 从当前逗号出现的位置开始寻找下一个‘，’出现的位置存入next_pos
					temp = s.Mid(pos, next_pos - pos); // 截取从当前逗号位置pos开始到下个逗号出现位置间的字符串并存入temp
					pos = next_pos + 1;                 // 当前逗号位置更新
					switch (case_id)
					{
					case 0:
						ID.Add(temp);
						break;
					case 1:
						StationName.Add(temp);
						SSB.AddString(temp);  // 将站点名字导入到各个查询列表中
						break;
					case 2:
						Address.Add(temp);
						break;
					case 3:
						TD.Add(temp);
						TotalDocksNum += _wtoi(temp);
						break;
					case 4:
						DIS.Add(temp);
						InServiceDocks += _wtoi(temp);
						break;
					case 5:
						Status.Add(temp);
						if (temp == "In Service")  InServiceStationNum++;
						break;
					case 6:
						Latitude.Add(temp);
						break;
					case 7:
						Longitude.Add(temp);
						break;
					}
					case_id++;
				}
				line++;
				TotalStationNum++;
			}
			MessageBox(_T("导入成功！"));
			flag1 = 1;
			NotInServiceStationNum = TotalStationNum - InServiceStationNum;
			NotInServiceDocks = TotalDocksNum - InServiceDocks;
		
			
			temp.Format(_T("%d"), InServiceStationNum);  // 将整型转化为字符串
			GetDlgItem(IDISSN)->SetWindowText(temp);
			temp.Format(_T("%d"), NotInServiceStationNum);
			GetDlgItem(IDNISSN)->SetWindowText(temp);
			temp.Format(_T("%d"), TotalStationNum);
			GetDlgItem(IDTTSN)->SetWindowText(temp);
			temp.Format(_T("%d"), InServiceDocks);
			GetDlgItem(IDISDN)->SetWindowText(temp);
			temp.Format(_T("%d"), NotInServiceDocks);
			GetDlgItem(IDNISDN)->SetWindowText(temp);
		}	// TODO: 在此添加控件通知处理程序代码
	}



void CMy20200400230温硕勋Dlg::OnBnClickedStationsearchbutton()

	{
		// TODO: 在此添加控件通知处理程序代码
		if (flag1 == 0)
		{
			MessageBox(_T("在查询具体站点前,请先导入所有站点的数据！"));
		}
		else
		{
			CString temp;  // 用于临时存放字符串
			int tempID = SSB.GetCurSel();  // 得到下拉框当前选项的ID
			if (tempID == -1)
			{
				MessageBox(_T("查询前请先选择需要查询的站点！"));  // 警告
			}
			else
			{
				SSB.GetLBText(tempID, temp);  // 得到下拉框当前ID下的内容并存入temp中
				int num = SSB.FindStringExact(0, temp);  // 得到对应内容的索引
				// 各个控件的信息显示
				GetDlgItem(IDStationID)->SetWindowText(ID[num]);
				GetDlgItem(IDStationAddress)->SetWindowText(Address[num]);
				GetDlgItem(IDTotalDocks)->SetWindowText(TD[num]);
				GetDlgItem(IDEnableDocks)->SetWindowText(DIS[num]);
				GetDlgItem(IDStatus)->SetWindowText(Status[num]);
				GetDlgItem(IDLongitude)->SetWindowText(Longitude[num]);
				GetDlgItem(IDLatitude)->SetWindowText(Latitude[num]);

				PaintPoint(num);
				// TODO: 在此添加控件通知处理程序代码
			}


		}
	}

void CMy20200400230温硕勋Dlg::OnBnClickedSearchstationbutton()
{
	
	
	CString str;
	UpdateData(TRUE); //变量IDSS已经设置CString类型 可直接使用
	str = IDSS;
	int jug = 0;
	if (flag1 == 0)
	{
		MessageBox(_T("在查询具体站点前,请先导入所有站点的数据！"));
	}
	//void tempID = IDSS.Format(str);  // 得到下拉框当前选项的ID
	//IDSS.Format(str);
	//
	//if (str = NULL)
	//{
	//	MessageBox(_T("查询前请先选择需要查询的站点！"));  // 警告
	//}
	
	else
	{
		for (int temp = 0; temp < SSB.GetCount(); temp++)
		{
		
			if (str == ID[temp])
			{
				GetDlgItem(IDStationID)->SetWindowText(ID[temp]);
				GetDlgItem(IDStationAddress)->SetWindowText(Address[temp]);
				GetDlgItem(IDTotalDocks)->SetWindowText(TD[temp]);
				GetDlgItem(IDEnableDocks)->SetWindowText(DIS[temp]);
				GetDlgItem(IDStatus)->SetWindowText(Status[temp]);
				GetDlgItem(IDLongitude)->SetWindowText(Longitude[temp]);
				GetDlgItem(IDLatitude)->SetWindowText(Latitude[temp]);
				PaintPoint(temp);
				jug = 1;
				break;

			}
			
			
			
		}	
		if (jug == 0)
		{
			MessageBox(_T("无此数据！"));
		
		}
	/*	for (int temp = 0; temp < SSB.GetCount(); temp++)
		{
			if (str != ID[temp])
			{
				MessageBox(_T("无此数据！请检查ID后重试！"));
				break;
			}
		}*/
		

	}
}

void CMy20200400230温硕勋Dlg::PaintPoint(int num)  // 显示站点信息的画点函数
{
	CBitmap bitmap;
	HBITMAP hBmp;
	bitmap.LoadBitmap(IDB_BITMAP1);
	hBmp = (HBITMAP)bitmap.GetSafeHandle();
	MAP.SetBitmap(hBmp);
	//以上为刷新图片的功能，查询完一个站点后自动清除痕迹。
	//以下为站点描点。
	CDC* pDC;
	pDC = MAP.GetDC();
	CPen pen;
	pen.CreatePen(1, 10, RGB(154, 82, 44));
	pDC->SelectObject(&pen);
	int x = ((87.8 + _wtof(Longitude[num])) * 1460.0), y = ((42.1 - _wtof(Latitude[num])) * 1960.0); // 将经纬度坐标转换为图片的像素（_wtof将CString类字符串转换为浮点型数据）
	pDC->MoveTo(x, y);
	pDC->LineTo(x, y);  // 画一个点
	CString str;
	str = StationName[num];
	while (str.GetLength() * 7 + x + 5 > 359)
	{
		x = x - 30;
	}
	pDC->DrawText(str, CRect(x + 15, y + 5, x + 20 + str.GetLength() * 7, y + 20), DT_SINGLELINE | DT_LEFT | DT_VCENTER);
}

void CMy20200400230温硕勋Dlg::OnBnClickedPaintall()
{
	
	CDC* pDC;
	pDC = MAP.GetDC();
	CPen pen;
	pen.CreatePen(1, 5, RGB(235, 0, 0));
	pDC->SelectObject(&pen);
	
	double x = 0.0; double y = 0.0;
	int i = 0;
	while(i < SSB.GetCount())
	{
		x = ((87.8 + _wtof(Longitude[i])) * 1460.0), y = ((42.1 - _wtof(Latitude[i])) * 1960.0);
		
		pDC->MoveTo(x, y);
		pDC->LineTo(x, y);
		i++;
	}
	ReleaseDC(pDC);
}
	// TODO: 在此添加控件通知处理程序代码



void CMy20200400230温硕勋Dlg::OnBnClickedClear()
{
	CBitmap bitmap;  // CBitmap对象，用于加载位图   
	HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄   

	bitmap.LoadBitmap(IDB_BITMAP1);  // 将位图IDB_BITMAP1加载到bitmap   
	hBmp = (HBITMAP)bitmap.GetSafeHandle();  // 获取bitmap加载位图的句柄   
	MAP.SetBitmap(hBmp);    // 复原位图
							// TODO: 在此添加控件通知处理程序代码
}


void CMy20200400230温硕勋Dlg::PaintPoint(int num, int i, int j, int k, int size)  // 不显示站点信息并且可自定义点颜色和大小的画点函数
{
	CDC* pDC;
	pDC = MAP.GetDC();
	CPen pen;
	pen.CreatePen(1, size, RGB(i, j, k));
	pDC->SelectObject(&pen);
	int x = ((87.8 + _wtof(Longitude[num])) * 1460.0), y = ((42.1 - _wtof(Latitude[num])) * 1960.0); // 将经纬度坐标转换为图片的像素（_wtof将CString类字符串转换为浮点型数据）
	pDC->MoveTo(x, y);
	pDC->LineTo(x, y);  // 画一个点
	ReleaseDC(pDC);
}

void CMy20200400230温硕勋Dlg::OnBnClickedCheckbutton()

// TODO: 在此添加控件通知处理程序代码

{
	// TODO: 在此添加控件通知处理程序代码
	if (flag1 == 0)
	{
		MessageBox(_T("抱歉！此功能需要您先导入站点数据和用户信息。导入数据后，请在旁边的下拉框中选中需要查询的用户，感谢您的理解！"));
	}
	else
	{
		
		
		Clear();
		int flag2 = 0;
		int num1, num2;
		int x1, y1, x2, y2;
		while (flag2 == 0)  // 为了达到生成两个随机点的效果。
		{
			num1 = rand() % 589;
			num2 = rand() % 589;
			if (num1 == num2)
			{
				num2 = num2 % 100;
			}
			x1 = ((87.8 + _wtof(Longitude[num1])) * 1460.0), y1 = ((42.1 - _wtof(Latitude[num1])) * 1960.0);
			x2 = ((87.8 + _wtof(Longitude[num2])) * 1460.0), y2 = ((42.1 - _wtof(Latitude[num2])) * 1960.0);
			int xdifference = x1 - x2;
			int ydifference = y1 - y2;
			if (abs(ydifference - xdifference) < 30 && abs(ydifference) > 100 && abs(xdifference) > 100)
			{
				flag2 = 1;
			}
		
		}

		// 显示站点名字
		GetDlgItem(IDBeginStation)->SetWindowTextW(StationName[num1]);
		GetDlgItem(IDEndStation)->SetWindowTextW(StationName[num2]);
		//在图上标出两个点
		PaintPoint(num1, 29, 222, 57, 13);
		PaintPoint(num2, 224, 105, 27, 13);
		//计算两点间距离：
		int distance;
		distance = (fabs(_wtof(Longitude[num1]) - _wtof(Longitude[num2])) * 1460 + fabs(_wtof(Latitude[num1]) - _wtof(Latitude[num2])) * 1960) / 5.0;
		CString t;
		t.Format(_T("%d"), distance);  // 将整型转化为字符串
		GetDlgItem(IDDistance)->SetWindowText(t);

		//画出两点间的轨迹（动画）
		int m1, n1, m2, n2, x, y, l;
		int a = 0, b = 0, k = 0, a1 = 0, b1 = 0;
		int mid1, mid2;
		int j;
		m1 = (int)x1;
		n1 = (int)y1;
		m2 = (int)x2;
		n2 = (int)y2;
		mid1 = m1; mid2 = n1;
		CDC* pDC;
		pDC = MAP.GetDC();
		CPen pen;
		pen.CreatePen(1, 3, RGB(50, 115, 235));
		pDC->SelectObject(&pen);
		x = abs(m1 - m2);
		y = abs(n1 - n2);
		l = abs((int)x1 - (int)x2) + abs((int)y1 - (int)y2);
		for (int i = 10; i > 0; i--)
		{
			if ((x % i == 0) && (a1 == 0))
			{
				a = i;
				a1 = 1;
			}
			if ((y % i == 0) && (b1 == 0))
			{
				b = i;
				b1 = 1;
			}
		}
		if ((a + b) % 2 == 0)
		{
			j = 1;
		}
		else {
			j = 0;
		}
		for (int i = 0; i < l; i++)
		{
			if (mid1 != m2 && mid2 != n2)
			{

				if (j % 2 == 0)
				{
					if (n1 > n2)
					{
						pDC->MoveTo(mid1, mid2);
						mid2--;
						Sleep(3);
						pDC->LineTo(mid1, mid2);
						k++;
					}
					if (n1 < n2)
					{
						pDC->MoveTo(mid1, mid2);
						mid2++;
						Sleep(3);
						pDC->LineTo(mid1, mid2);
						k++;
					}
					if (mid2 != n2 && k == y / b)
					{
						j++;
						k = 0;
					}
				}
				else
				{
					if (m1 > m2)
					{
						pDC->MoveTo(mid1, mid2);
						mid1--;
						Sleep(3);
						pDC->LineTo(mid1, mid2);
						k++;
					}
					if (m1 < m2)
					{
						pDC->MoveTo(mid1, mid2);
						mid1++;
						Sleep(3);
						pDC->LineTo(mid1, mid2);
						k++;
					}
					if (k == x / a)
					{
						j++;
						k = 0;
					}
				}
			}
			else {
				pDC->MoveTo(mid1, mid2);
				Sleep(3);
				if (m1 > m2 && mid1 != m2)mid1--;
				if (m1 < m2 && mid1 != m2)mid1++;
				if (n1 > n2 && mid2 != n2)mid2--;
				if (n1 < n2 && mid2 != n2)mid2++;
				pDC->LineTo(mid1, mid2);
			}
		}
	}
}// TODO: 在此添加控件通知处理程序代码
	
		void CMy20200400230温硕勋Dlg::Clear()
		{
			CBitmap bitmap;  // CBitmap对象，用于加载位图   
			HBITMAP hBmp;    // 保存CBitmap加载的位图的句柄   

			bitmap.LoadBitmap(IDB_BITMAP1);  // 将位图IDB_BITMAP1加载到bitmap   
			hBmp = (HBITMAP)bitmap.GetSafeHandle();  // 获取bitmap加载位图的句柄   
			MAP.SetBitmap(hBmp);    // 复原位图
									// TODO: 在此添加控件通知处理程序代码
		}


		void CMy20200400230温硕勋Dlg::OnBnClickedButton4()

		{
			
				CString defaultDir = L"D:\\";  // 默认打开文件路径
				CString fileName = L"";  // 默认打开文件名
				CString Filter = L"文件(*.txt; *.csv)|*.txt; *.csv;||";  // 文件过滤类型
				CFileDialog importfiledlg(TRUE, defaultDir, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, Filter, NULL);
				INT_PTR result = importfiledlg.DoModal();  // 弹出导入框并记录导入结果
				if (result == IDOK)
				{
					fileName = importfiledlg.GetPathName();
					CStdioFile f;
					CFileException e;
					if (!f.Open(fileName, CFile::modeRead, &e))
					{
						TRACE(_T("您好！文件无法打开，请检查后重试。 %d\n"), e.m_cause);
					}
					CString str = NULL;
					f.ReadString(str);
					int i = 0;

					while (str != "" && i++ < 50)
					{
						Users.AddString(str);                    
						f.ReadString(str);
					}
					MessageBox(_T("导入成功！"));
					flag1 = 1;
					

					// TODO: 在此添加控件通知处理程序代码
				}
			}


		void CMy20200400230温硕勋Dlg::OnBnClickedButton5()
		{
			HelpDlg dlg;
			dlg.DoModal();
		}
