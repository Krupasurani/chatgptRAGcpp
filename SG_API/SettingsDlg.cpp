#include "pch.h"
#include "..\utils.h"
#include "..\AutoChat.h"
#include "..\REST\JSON.h"
#include "..\REST\JSONValue.h"
#include "..\include\nlohmann\json.hpp"

#include <locale>
#include <string>
#include <fstream>
#include "SettingsDlg.h"

using json = nlohmann::json;

using namespace std;

CSettingsDlg::CSettingsDlg(const std::wstring& configFilePath, CWnd* pParent /*=nullptr*/)
    : CDialog(IDD_SETTINGS_DIALOG, pParent), m_configFilePath(configFilePath)
{
    m_configData = LoadConfigData(m_configFilePath); // Load existing config
}

void CSettingsDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_API_KEY, m_apiKey);
    DDX_Text(pDX, IDC_EDIT_SEFARIA_PROMPT, m_sefariaPrompt);
    DDX_Text(pDX, IDC_EDIT_EXPLANATION_REQUEST, m_explanationRequest);
}

BEGIN_MESSAGE_MAP(CSettingsDlg, CDialog)
    ON_BN_CLICKED(IDOK, &CSettingsDlg::OnBnClickedOk)
END_MESSAGE_MAP()

BOOL CSettingsDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    LoadConfigData(m_configFilePath);
    // Load current settings into UI controls
    m_apiKey = m_configData.apiKey.c_str();  // Already using CString
    m_sefariaPrompt = gSefariaPrompt.c_str();  // Already using CString
    m_explanationRequest = gExplanationRequest.c_str();  // Already using CString

    UpdateData(FALSE); // Populate dialog fields
    return TRUE;       // Return TRUE unless setting focus to a control
}

void CSettingsDlg::OnBnClickedOk()
{
    UpdateData(TRUE); // Get data from dialog fields

    // Save changes back to the config file
    m_configData.apiKey = convertWstringToString(m_apiKey.GetString());  // Convert CString to std::string
    m_configData.sefariaPrompt = std::wstring(m_sefariaPrompt);  // Convert CString to std::wstring
    m_configData.explanationRequest = std::wstring(m_explanationRequest);  // Convert CString to std::wstring

    SaveConfigData(m_configFilePath,m_configData);
    CDialog::OnOK();
}
