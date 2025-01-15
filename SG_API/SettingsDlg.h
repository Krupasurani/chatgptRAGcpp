#pragma once
#include "pch.h"
#include "resource.h"
#include "..\AutoChat.h"

class CSettingsDlg : public CDialog
{
public:
    CSettingsDlg(const std::wstring& configFilePath, CWnd* pParent = nullptr);

    // Dialog Data
    enum { IDD = IDD_SETTINGS_DIALOG };

protected:
    virtual void DoDataExchange(CDataExchange* pDX) override; // DDX/DDV support
    virtual BOOL OnInitDialog() override;
    afx_msg void OnBnClickedOk();

    DECLARE_MESSAGE_MAP()

private:
    std::wstring m_configFilePath;
    ConfigData m_configData;

    // UI Variables
    CString m_apiKey;
    CString m_sefariaPrompt;
    CString m_explanationRequest;
};
