
// SG_GPTDlg.cpp : implementation file
//

#include "pch.h"

#include <iostream>
#include <sstream> 

#include "framework.h"
#include "SG_API.h"
#include "SG_APIDlg.h"
#include "..\AutoChat.h"
#include "..\utils.h"
#include "afxdialogex.h"
#include <richedit.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CSGGPTDlg dialog

class CBalloonAlert : public CWnd
{
public:
    void OnShowBalloon(const wchar_t* title, const wchar_t* message)
    {
        NOTIFYICONDATA nid;
        nid.cbSize = sizeof(NOTIFYICONDATA);
        nid.hWnd = m_hWnd;
        nid.uID = 0;
        nid.uFlags = NIF_INFO;
        nid.dwInfoFlags = NIIF_WARNING;
        wcscpy_s(nid.szInfoTitle, title);
        wcscpy_s(nid.szInfo, message);
        Shell_NotifyIcon(NIM_ADD, &nid);
    }
};


void ShowBalloonMessage(const CString& title, const CString& message, DWORD timeout)
{
    CBalloonAlert b;
    b.OnShowBalloon(title, message);
}



CSGGPTDlg::CSGGPTDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_SG_GPT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSGGPTDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_QUERY, m_Query);
	DDX_Control(pDX, IDC_RESPONSE, m_Response);
}

BEGIN_MESSAGE_MAP(CSGGPTDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CSGGPTDlg::OnBnClickedOk)
	ON_WM_TIMER()
    ON_WM_CTLCOLOR()
	ON_WM_KEYDOWN()
	ON_BN_CLICKED(ID_RUN, &CSGGPTDlg::OnBnClickedRun)
	ON_BN_CLICKED(ID_ATTACH, &CSGGPTDlg::OnBnClickedAttach)
    ON_WM_DESTROY()
    ON_BN_CLICKED(ID_SETTINGS, &CSGGPTDlg::OnBnClickedSettings)
END_MESSAGE_MAP()


void CSGGPTDlg::OnDestroy()
{
    
    CDialogEx::OnDestroy();
}
// CSGGPTDlg message handlers
BOOL CSGGPTDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if (m_erase_query)
        {
            m_Query.SetWindowTextW(L"");
            m_erase_query = false;
        }
        if (pMsg->wParam == VK_RETURN/* && GetKeyState(VK_CONTROL) < 0*/)
        {
            // CTRL+ENTER is pressed, handle the action here
            OnBnClickedRun();
            return TRUE; // Suppress further processing of the key event
        }
        else if (pMsg->wParam == 'C' && GetKeyState(VK_CONTROL) < 0)
        {
            // CTRL+C is pressed, handle the action here
            CString selectedText;
            m_Response.GetWindowText(selectedText);
            if (!selectedText.IsEmpty())
            {
                // Copy the selected text to the clipboard
                if (OpenClipboard())
                {
                    EmptyClipboard();

                    HGLOBAL hClipboardData = GlobalAlloc(GMEM_MOVEABLE, (selectedText.GetLength() + 1) * sizeof(TCHAR));
                    if (hClipboardData != NULL)
                    {
                        LPTSTR pClipboardData = static_cast<LPTSTR>(GlobalLock(hClipboardData));
                        if (pClipboardData != NULL)
                        {
                            _tcscpy_s(pClipboardData, selectedText.GetLength() + 1, selectedText.GetString());
                            GlobalUnlock(hClipboardData);
                            SetClipboardData(CF_UNICODETEXT, hClipboardData);
                            ShowBalloonMessage(L"Response copied to clipboard",L"",1000);
                        }
                    }

                    CloseClipboard();
                }
            }
            return TRUE; // Suppress further processing of the key event
        }
    }
    return CDialogEx::PreTranslateMessage(pMsg);
}



const COLORREF SG_GPT_COLOR = RGB(173, 216, 230);


BOOL CSGGPTDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

    // Subclass the CRichEditCtrl to associate it with the member variable
//    m_Response.SubclassDlgItem(IDC_RESPONSE, this);

    m_CustomBrush.CreateSolidBrush(SG_GPT_COLOR);
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    // Get the width of the screen
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);

    // Get the current height and vertical position of the dialog
    CRect rect;
    GetWindowRect(&rect);
    int dialogHeight = rect.Height();
    int dialogTop = rect.top;

    // Set the font for the CEdit control
    CFont font;
    font.CreatePointFont(100, L"Arial");  // Customize the font name and size if desired
    m_Response.SetFont(&font);

    // Resize the dialog width while keeping the height and vertical position unchanged
    SetWindowPos(NULL, 0, dialogTop, screenWidth, dialogHeight, SWP_NOZORDER);

	SetTimer(1000, 500,NULL);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSGGPTDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSGGPTDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CSGGPTDlg::OnBnClickedOk()
{

}


void CSGGPTDlg::OnTimer(UINT_PTR nIDEvent)
{

	GotoDlgCtrl(GetDlgItem(IDC_QUERY));
	m_Query.SetActiveWindow();
//    m_Response.SetWindowText(L"Normal text:\n```some code\nint i=10;```\nMore text\n");
    m_Response.SetWindowText(L"Ready to start a ^bChavrusa%b discussion ...");
    ApplyRichTextStyles(m_Response);

	KillTimer(1000);
	CDialogEx::OnTimer(nIDEvent);
}

SG_API AutoChat;

HBRUSH CSGGPTDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialogEx::OnCtlColor(pDC, pWnd, nCtlColor);

    if (nCtlColor == CTLCOLOR_EDIT && pWnd == &m_Query)
    {
        // Set the desired text and background color for m_Query
        pDC->SetTextColor(RGB(0, 0, 0));   // White text color
        pDC->SetBkColor(SG_GPT_COLOR);       // Blue background color

        // Return the custom brush
        return static_cast<HBRUSH>(m_CustomBrush.GetSafeHandle());
    }

    return hbr;
}



void CSGGPTDlg::EnableControls(BOOL bEnable)
{
    // Disable or enable the controls without hiding or showing them
    m_Response.EnableWindow(bEnable);
    GetDlgItem(ID_RUN)->EnableWindow(bEnable);
    GetDlgItem(ID_ATTACH)->EnableWindow(bEnable);

    // Handle the m_Query control separately
    if (bEnable)
    {
        // Restore focus to the m_Query control before enabling it
        m_Query.SetFocus();
        m_Query.EnableWindow(TRUE);
    }
    else
    {
        // Disable the m_Query control without changing its visibility
        //m_Query.EnableWindow(FALSE);
    }
}

void CSGGPTDlg::ApplyRichTextStyles(CRichEditCtrl& m_editControl)
{
    // Retrieve the text from the RichEdit control
    CString text;
    m_editControl.GetWindowText(text);

    // Set the default font style for the begining of the text
    CHARFORMAT cfDefault;
    cfDefault.cbSize = sizeof(cfDefault);
    cfDefault.dwMask = CFM_FACE | CFM_SIZE;
    cfDefault.dwEffects = 0;  // No bold or color
    cfDefault.yHeight = 300;  // Set the desired font size (adjust as needed)
    cfDefault.bPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
    lstrcpy(cfDefault.szFaceName, L"Arial");  // Set the desired font face name

    // Set the source code-like font style for the code portion
    CHARFORMAT cfCode;
    cfCode.cbSize = sizeof(cfCode);
    cfCode.dwMask = CFM_FACE | CFM_SIZE;
    cfCode.dwEffects = 0;  // No bold or color
    cfCode.yHeight = 200;  // Set the desired font size (adjust as needed)
    cfCode.bPitchAndFamily = FIXED_PITCH | FF_MODERN;
    lstrcpy(cfCode.szFaceName, L"Courier New");  // Set the desired font face name


    // Start processing the text
    int pos = 0;
    int start = 0;
    bool bold = false;

    while (pos < text.GetLength())
    {
        if (text.Mid(pos, 2) == L"^b")
        {
            // Apply current formatting up to this point
            m_editControl.SetSel(start, pos);
            m_editControl.SetSelectionCharFormat(bold ? cfDefault : cfCode);

            // Remove control sequence and switch to bold
            text.Delete(pos, 2);
            bold = true;
            start = pos; // Update the start position
        }
        else if (text.Mid(pos, 2) == L"%b")
        {
            // Apply current formatting up to this point
            m_editControl.SetSel(start, pos);
            m_editControl.SetSelectionCharFormat(bold ? cfDefault : cfCode);

            // Remove control sequence and switch to normal
            text.Delete(pos, 2);
            bold = false;
            start = pos; // Update the start position
        }
        else
        {
            ++pos; // Move to the next character
        }
    }

    // Apply formatting for the remaining text
    m_editControl.SetSel(start, text.GetLength());
    m_editControl.SetSelectionCharFormat(bold ? cfDefault : cfCode);

    // Update the control text to reflect changes
    m_editControl.SetWindowText(text);

    // Deselect any selection to display final formatting
    m_editControl.SetSel(-1, 0);
}


std::wstring CSGGPTDlg::AskChatGPTForSefariaQuery(CString question)
{
    AutoChat.UpdateSettingsFromConfig();

    // Check if gSefariaPrompt is empty
    if (gSefariaPrompt.empty())
    {
        WriteLogFile(L"Error: No gSefariaPrompt provided");
        return L"";
    }
    

    // Final string after concatenation
   CString prompt = (CString)gSefariaPrompt.c_str()+L" "+question;

    std::wstring sefariaQuery;
    AddTextToResponse(L"Generating ChatGPT prompt:\r\n'" + (CString)prompt + (CString)L"'\r\n");  // Ensure proper concatenation

    AutoChat.SG_GPT_GENERIC(prompt.GetString(), sefariaQuery);  // Assuming this generates a Sefaria query

    WriteLogFile(L"Generated Query from SG_GPT_GENERIC: '%s'", sefariaQuery.c_str());

    return sefariaQuery;
}

void CSGGPTDlg::OnBnClickedRun()
{
    std::wstring response;
    CString query;
    m_Query.GetWindowText(query);

    CString CSTRresponse;
    CWaitCursor waitCursor;
    EnableControls(FALSE);

    std::wstring queryStr = query.GetString();
    std::wstring sefariaQuestion = queryStr;

    AddTextToResponse(L"Asking ChatGPT:\r\n'" + (CString)sefariaQuestion.c_str() + (CString)L"'\r\n");  // Ensure proper concatenation

    // Get the Sefaria-friendly query from ChatGPT
    std::wstring sefariaQuery = AskChatGPTForSefariaQuery(sefariaQuestion.c_str());


    if (!sefariaQuery.empty())
    {
        AddTextToResponse(L"I converted your query to:\r\n'" + (CString)sefariaQuery.c_str() + (CString)L"'\r\n");  // Ensure proper concatenation

        // Now, attempt to fetch the response from the Sefaria API
        SG_API sgApi;

        try
        {
            // Call FetchSefariaText with proper handling
            response = sgApi.FetchSefariaText(sefariaQuery);
            if (!response.empty())
            {
                AddTextToResponse(L"\r\nHere is what Sefaria returned:\r\n'```" + (CString)response.c_str() + (CString)L"'```\r\n");
            }
            else
            {
                AddTextToResponse(L"\r\nSefaria returned an empty response.\r\n");
            }
        }
        catch (const std::exception& ex)
        {
            WriteLogFile(L"Error during Sefaria API call: %s", ex.what());
            AddTextToResponse(L"\r\nError calling Sefaria API. Please try again.\r\n");
        }
    }
    else
    {
        AddTextToResponse(L"\r\nFailed to generate a valid Sefaria query from your question.\r\n");
    }

    // Set the final response text to the response control
//    m_Response.SetWindowText(CSTRresponse);
    EnableControls(TRUE);
}

void CSGGPTDlg::AddTextToResponse(CString text)
{
    CString currentText;
    m_Response.GetWindowText(currentText);
    currentText+=L"\r\n";
    currentText+=text;
    m_Response.SetWindowText(currentText);
    UpdateData(FALSE);
}

void CSGGPTDlg::OnBnClickedAttach()
{
    // Create an instance of CFileDialog to allow the user to select a file
    CFileDialog dlg(TRUE, NULL, NULL, OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST, _T("Text Files (*.txt)|*.txt|C++ Files (*.cpp;*.h)|*.cpp;*.h||"), this);

    // Show the dialog and retrieve the selected file path
    if (dlg.DoModal() == IDOK)
    {
        CString filePath = dlg.GetPathName();

        // Open the selected file using CFile
        CFile file;
        if (file.Open(filePath, CFile::modeRead))
        {
            // Get the size of the file and allocate a buffer to store its contents
            ULONGLONG fileSize = file.GetLength();
            char* fileBuffer = new char[fileSize + 1];

            // Read the file contents into the buffer
            file.Read(fileBuffer, fileSize);
            fileBuffer[fileSize] = '\0';

            // Copy the contents of the buffer to a CString, handling different types of new lines
            CString fileContent;
            char* start = fileBuffer;
            char* end = NULL;
            while ((end = strchr(start, '\n')))
            {
                if (end > fileBuffer && *(end - 1) == '\r')
                {
                    // Handle CRLF (Windows) line endings
                    CString line(start, end - start - 1);
                    fileContent.Append(line);
                    fileContent.Append(_T("\r\n"));
                }
                else
                {
                    // Handle LF (Unix) line endings
                    CString line(start, end - start);
                    fileContent.Append(line);
                    fileContent.Append(_T("\n"));
                }
                start = end + 1;
            }
            CString line(start);
            fileContent.Append(line);

            // Clean up the buffer and close the file
            delete[] fileBuffer;
            file.Close();

            // Do something with the file content, such as display it in a text box
            CString temp;
            m_Query.GetWindowText(temp);
            temp.Append(_T("\n"));
            temp.Append(fileContent);
            m_Query.SetWindowText(temp);
            // Set up the edit control to handle multi-line text
            m_Query.SetSel(-1, 0); // Set the selection to the end of the text
            m_Query.SetLimitText(0); // Remove the text length limit
            m_Query.ModifyStyle(0, ES_MULTILINE | ES_AUTOVSCROLL); // Add multi-line and auto-scroll styles
            m_Query.ModifyStyleEx(0, WS_EX_CLIENTEDGE); // Add a 3D border
            m_Query.SetMargins(2, 2); // Add some margin to the text
            m_Query.SetFont(GetFont()); // Use the same font as the dialog

            // Run the query and store the result in m_Response
            OnBnClickedRun();
            CString response;
            m_Response.GetWindowText(response);

            // Save the result to a temporary file
            CString tempFilePath = filePath + _T(".txt");
            CFile tempFile;
            if (tempFile.Open(tempFilePath, CFile::modeCreate | CFile::modeWrite))
            {
                tempFile.Write(response, response.GetLength() * sizeof(TCHAR));
                tempFile.Close();

                // Open the temporary file using ShellExecute
                ShellExecute(NULL, _T("open"), tempFilePath, NULL, NULL, SW_SHOWNORMAL);
            }
        }
    }
}



#include "SettingsDlg.h"
void CSGGPTDlg::OnBnClickedSettings()
{
    // Construct the full path to config.json
    std::wstring exeDir = GetExecutableDir();
    std::wstring configPath = exeDir + L"\\config.json";

    CSettingsDlg dlg(configPath.c_str());
    dlg.DoModal();
    // TODO: Add your control notification handler code here
}
