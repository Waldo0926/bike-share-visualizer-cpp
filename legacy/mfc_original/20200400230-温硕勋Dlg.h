
// 20200400230-温硕勋Dlg.h: 头文件
//

#pragma once


// CMy20200400230温硕勋Dlg 对话框
class CMy20200400230温硕勋Dlg : public CDialogEx
{
// 构造
public:
	CMy20200400230温硕勋Dlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MY20200400230_DIALOG };
#endif

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
	afx_msg void OnBnClickedButton1();
	CListBox SL;
	CStatic TTSN;
	CStatic ISSN;
	CStatic NISSN;
	CStatic ISDN;
	CStatic NISDN;
	CStatic ShowID;
	CEdit ShowAddress;
	CStatic ShowTotalDocks;
	CStatic ShowEnableDocks;
	CStatic LTT;
	CStatic LATT;
	CComboBox SSB;
	afx_msg void OnBnClickedStationsearchbutton();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnBnClickedSearchstationbutton();
	CString IDSS;
	void PaintPoint(int);
	CStatic MAP;
	afx_msg void OnBnClickedPaintall();
	afx_msg void OnBnClickedClear();
	CComboBox Users;
	CButton Check;
	afx_msg void OnBnClickedCheckbutton();
	afx_msg void PaintPoint(int num, int i, int j, int k, int size);
	CStatic Distance;
	afx_msg void CMy20200400230温硕勋Dlg::Clear();
	afx_msg void OnBnClickedButton4();
	afx_msg void OnBnClickedButton5();
};
