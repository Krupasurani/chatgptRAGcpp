
// SG_GPTDlg.h : header file
//
#include <string>
#pragma once
void ShowBalloonMessage(const CString& title, const CString& message, DWORD timeout);


// CSGGPTDlg dialog
class CSGGPTDlg : public CDialogEx
{
	bool m_erase_query{ false };
	void EnableControls(BOOL bEnable);
// Construction
public:
	CSGGPTDlg(CWnd* pParent = nullptr);	// standard constructor
	std::wstring AskChatGPTForSefariaQuery(CString question);

	std::wstring AskChatGPTForSefariaReference(const std::wstring& query);


// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SG_GPT_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	BOOL PreTranslateMessage(MSG* pMsg);
	DECLARE_MESSAGE_MAP()


	afx_msg void OnDestroy();

public:
	CEdit m_Query;
	CRichEditCtrl m_Response;
	afx_msg void OnBnClickedOk();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	void AddTextToResponse(CString text);

	void ApplyRichTextStyles(CRichEditCtrl& m_editControl);
	afx_msg void OnBnClickedRun();
	afx_msg void OnBnClickedAttach();
	CBrush m_CustomBrush;
	afx_msg void OnBnClickedSettings();
};
static DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE pbBuff, LONG cb, LONG* pcb);
