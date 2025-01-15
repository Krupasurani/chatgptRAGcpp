#ifndef _OFFICE_AUTOMATION_H_
#define _OFFICE_AUTOMATION_H_

#include <windows.h>
#include <ole2.h>
#include <oleauto.h>
#include <comutil.h>
#include <string>
#include "AutoChat.h"
#include "Database/CppSQLite3U.h"
#include "Database/GenericDataProvider.h"
#include "include/nlohmann/json.hpp"

using json = nlohmann::json;

bool SG_GPT_GENERIC(const std::wstring request, std::wstring& response);

void ShowError(HRESULT errorCode)
{
	LPVOID lpMsgBuf = nullptr;
	DWORD dwChars = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			errorCode,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR)&lpMsgBuf,
			0,
			NULL);
	if (lpMsgBuf)
	{
		WriteLogFile(L"Error: %s", (LPTSTR)lpMsgBuf);
	}
	else
	{
		WriteLogFile(L"Error: %d (unknown)", errorCode);
	}
}

HRESULT OLEMethod(int nType, VARIANT* pvResult, IDispatch* pDisp, LPOLESTR ptName, int cArgs...)
{
	if (!pDisp) return E_FAIL;

	va_list marker;
	va_start(marker, cArgs);

	DISPPARAMS dp = { NULL, NULL, 0, 0 };
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPID dispID;
	char szName[200];


	// Convert down to ANSI
	WideCharToMultiByte(CP_ACP, 0, ptName, -1, szName, 256, NULL, NULL);
	HRESULT hr;

	// Get DISPID for name passed...
	try
	{
		hr = pDisp->GetIDsOfNames(IID_NULL, &ptName, 1, LOCALE_USER_DEFAULT, &dispID);
	}

	catch (exception& e)
	{
		return E_FAIL;
	}

	if (FAILED(hr)) {
		return hr;
	}
	// Allocate memory for arguments...
	VARIANT* pArgs = new VARIANT[cArgs + 1];
	// Extract arguments...
	for (int i = 0; i < cArgs; i++) {
		pArgs[i] = va_arg(marker, VARIANT);
	}

	// Build DISPPARAMS
	dp.cArgs = cArgs;
	dp.rgvarg = pArgs;

	// Handle special-case for property-puts!
	if (nType & DISPATCH_PROPERTYPUT) {
		dp.cNamedArgs = 1;
		dp.rgdispidNamedArgs = &dispidNamed;
	}

	// Make the call!
	try
	{
		hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, nType, &dp, pvResult, NULL, NULL);
	}

	catch (exception& e)
	{
		return E_FAIL;
	}

	if (FAILED(hr)) {
		return hr;
	}
	// End variable-argument section...
	va_end(marker);

	delete[] pArgs;
	return hr;
}

class OfficeAutomation
{
protected:
	IDispatch* m_pWApp;
	IDispatch* m_pDocuments;
	IDispatch* m_pActiveDocument;
	IDispatch* pDocApp;
	IDispatch* pComments;
	IDispatch* pComment;
	IDispatch* pCommentRange;
	IDispatch* pRange;
	HRESULT		m_hr;
	HRESULT Initialize(bool ShouldDocBeVisible = true)
	{
		CoInitialize(NULL);
		CLSID clsid;
		m_hr = CLSIDFromProgID(L"Word.Application", &clsid);
		if (SUCCEEDED(m_hr))
		{
			m_hr = CoCreateInstance(clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IDispatch, (void**)&m_pWApp);
			if (FAILED(m_hr)) m_pWApp = NULL;
		}

		m_hr = SetVisible(ShouldDocBeVisible);

		return m_hr;
	}
public:
	//Constructor creates a instance of object
	OfficeAutomation()
	{
		m_pWApp = NULL;
		m_pDocuments = NULL;
		m_pActiveDocument = NULL;
		pComments = NULL;
		pCommentRange = NULL;
		pRange = NULL;
		pComment = NULL;
		Initialize(false);
	}
	//Destructor
	~OfficeAutomation() {}
	//Sets visibility of active document
	HRESULT SetVisible(bool ShouldDocBeVisible = true)
	{
		VARIANT x;
		x.vt = VT_I4;
		x.lVal = ShouldDocBeVisible;
		m_hr = OLEMethod(DISPATCH_PROPERTYPUT, NULL, m_pWApp, (LPOLESTR)L"Visible", 1, x);

		return m_hr;
	}
	//Quits the MS Word. Closes process
	//HRESULT Quit();
	//Opens document and set document's visibility
	HRESULT OpenDocument(LPCTSTR szFilename, bool bVisible = true)
	{
		if (m_pWApp == NULL)
		{
			if (FAILED(m_hr = Initialize(bVisible)))
				return m_hr;
		}
		VARIANT fname;
		VariantInit(&fname);
		fname.vt = VT_BSTR;
		fname.bstrVal = SysAllocString((OLECHAR*)szFilename);

		// GetDocuments
		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &result, m_pWApp, (LPOLESTR)L"Documents", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}

			m_pDocuments = result.pdispVal;
		}
		// OpenDocument
		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_METHOD, &result, m_pDocuments, (LPOLESTR)L"Open", 1, fname);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}

			m_pActiveDocument = result.pdispVal;
		}
		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &result, m_pActiveDocument, (LPOLESTR)L"Application", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}

			pDocApp = result.pdispVal;
		}
		this->SetVisible(bVisible);
		return m_hr;
	}
	//Closes active document with or without saving
	HRESULT CloseActiveDocument()
	{
		if (m_pWApp == NULL) return E_FAIL;
		// Save the active document
		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_METHOD, &result, m_pActiveDocument, (LPOLESTR)L"Save", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}
		}

		// Close the active document
		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_METHOD, &result, m_pActiveDocument, (LPOLESTR)L"Close", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}
		}

		return m_hr;
	}

	HRESULT ToggleTrackChanges(bool enable)
	{
		// Get the current "Track Revisions" status
		VARIANT trackChangesVariant;
		VariantInit(&trackChangesVariant);
		m_hr = OLEMethod(DISPATCH_PROPERTYGET, &trackChangesVariant, m_pActiveDocument, (LPOLESTR)L"TrackRevisions", 0);
		if (FAILED(m_hr))
		{
			ShowError(m_hr);
			return m_hr;
		}

		bool isTrackChangesEnabled = (trackChangesVariant.vt == VT_BOOL && trackChangesVariant.boolVal == VARIANT_TRUE);

		// Toggle "Track Revisions" if necessary
		if ((enable && !isTrackChangesEnabled) || (!enable && isTrackChangesEnabled))
		{
			VARIANT vEnable;
			VariantInit(&vEnable);
			vEnable.vt = VT_BOOL;
			vEnable.boolVal = enable ? VARIANT_TRUE : VARIANT_FALSE;

			m_hr = OLEMethod(DISPATCH_PROPERTYPUT, NULL, m_pActiveDocument, (LPOLESTR)L"TrackRevisions", 1, vEnable);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}
		}

		return S_OK;
	}

	//Prints comments available in the document
	HRESULT FindCommentsAndReply ()
	{
		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &result, m_pActiveDocument, (LPOLESTR)L"Comments", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}

			pComments = result.pdispVal;
		}

		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &result, m_pActiveDocument, (LPOLESTR)L"Content", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return m_hr;
			}
			pRange = result.pdispVal;
		}

		// Loop through each comment
		long commentCount = 0;
		OLECHAR* commentText = nullptr;
		SG_API AutoChat;
		while (true)
		{
			// Get the next comment
			VARIANT index;
			index.vt = VT_I4;
			index.lVal = commentCount + 1;
			VARIANT commentVariant;
			VariantInit(&commentVariant);
			m_hr = OLEMethod(DISPATCH_METHOD, &commentVariant, pComments, (LPOLESTR)L"Item", 1, index);

			if (commentVariant.vt != VT_DISPATCH || commentVariant.pdispVal == nullptr) 
			{
				return S_OK;
			}

			if (FAILED(m_hr))
				return m_hr;

			pComment = commentVariant.pdispVal;

			// Get the comment text
			VARIANT commentTextVariant;
			VariantInit(&commentTextVariant);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &commentTextVariant, pComment, (LPOLESTR)L"Range", 0);
			if (FAILED(m_hr))
				return m_hr;

			if (commentTextVariant.vt == VT_DISPATCH && commentTextVariant.pdispVal != nullptr) 
			{
				pCommentRange = commentTextVariant.pdispVal;
				VARIANT commentTextVariant2;
				VariantInit(&commentTextVariant2);
				m_hr = OLEMethod(DISPATCH_PROPERTYGET, &commentTextVariant2, pCommentRange, (LPOLESTR)L"Text", 0);
				if (FAILED(m_hr))
					return m_hr;

				if (commentTextVariant2.vt == VT_BSTR) 
				{
					commentText = commentTextVariant2.bstrVal;
				}
				pCommentRange->Release();
			}

			// Get the range of the text that the comment is associated with
			VARIANT commentRangeVariant;
			VariantInit(&commentRangeVariant);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &commentRangeVariant, pComment, (LPOLESTR)L"Scope", 0);
			if (FAILED(m_hr))
				return m_hr;

			if (commentRangeVariant.vt == VT_DISPATCH && commentRangeVariant.pdispVal != nullptr) 
			{
				pCommentRange = commentRangeVariant.pdispVal;

				// Get the text of the range
				VARIANT rangeTextVariant;
				VariantInit(&rangeTextVariant);
				m_hr = OLEMethod(DISPATCH_PROPERTYGET, &rangeTextVariant, pCommentRange, (LPOLESTR)L"Text", 0);
				if (FAILED(m_hr))
				{
					pCommentRange->Release();
					commentVariant.pdispVal->Release();
					return m_hr;
				}

				if (rangeTextVariant.vt == VT_BSTR && rangeTextVariant.bstrVal != nullptr) 
				{
					OLECHAR* rangeText = rangeTextVariant.bstrVal;

					std::wstring ResponseGPT;

					// Setting up prompt for GPT request
					wstring prompt{ L"I will send you some text, and an associated comment that tells what changes need to be made in the text. Please start your response with 'Changed text: ' followed by the actual updated text. Here is the original text: '" };
					prompt += rangeText;
					prompt += L"'. And here is the associated comment suggesting the change: '";
					prompt += commentText;
					prompt += L"'. Please do not respond with anything else, only include the changed text and nothing else. If you do not have the correct answer, or don't know what to say, respond with these exact words: 'I do not understand'";

					// Send request to OpenAI, receive response in ResponseGPT. This ResponseGPT will replace the actual text
					bool is_request_success = AutoChat.SG_GPT_GENERIC(prompt, ResponseGPT);

					if (is_request_success == false)
						return E_FAIL;

					if (ResponseGPT.find(L"I do not understand") != wstring::npos)
					{
						pCommentRange->Release();
						commentVariant.pdispVal->Release();
						commentCount++;
						continue;
					}

					CDataProvider::AddCommentAndTextToDB(commentText, rangeText);

					WriteLogFile(L"Sending requested to ChatGPT:\n== Comment: '%S'\n== AssociatedText: '%S\n== Response: '%S'",
						commentText, rangeText,
						ResponseGPT.c_str());

					wstring ChangedText = ResponseGPT;
					ChangedText = ChangedText.substr(strlen("Changed text: "));
					
					if (!ChangedText.empty())
					{
						// Set the text of the range to the new associated text;
						VARIANT vCommentRange;
						VariantInit(&vCommentRange);
						m_hr = OLEMethod(DISPATCH_PROPERTYGET, &vCommentRange, pComment, (LPOLESTR)L"Range", 0);
						if (FAILED(m_hr))
						{
							pCommentRange->Release();
							commentVariant.pdispVal->Release();
							return m_hr;
						}

						VARIANT newText;
						newText.vt = VT_BSTR;
						newText.bstrVal = SysAllocString(ChangedText.c_str());
						m_hr = OLEMethod(DISPATCH_PROPERTYPUT, NULL, pCommentRange, (LPOLESTR)L"Text", 1, newText);
						if (FAILED(m_hr))
						{
							pCommentRange->Release();
							commentVariant.pdispVal->Release();
							return m_hr;
						}

					}

					SysFreeString(rangeText);
					
				}

				pCommentRange->Release();
			}

			commentVariant.pdispVal->Release();
			commentCount++;
		}

		return m_hr;
	}
	
	//Returns number of opened documents in the process. There can be more MS Word processes, and the framework won't see documents controled by other processes.
	int CountDocuments()
	{
		int cnt;
		if (m_pWApp == NULL)
		{
			return -5;
		}

		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &result, m_pWApp, (LPOLESTR)L"Documents", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return -1;
			}

			m_pDocuments = result.pdispVal;
		}

		{
			VARIANT result;
			VariantInit(&result);
			m_hr = OLEMethod(DISPATCH_PROPERTYGET, &result, m_pDocuments, (LPOLESTR)L"Count", 0);
			if (FAILED(m_hr))
			{
				ShowError(m_hr);
				return -1;
			}

			cnt = result.lVal;
		}
		return cnt;
	}

	//Finds first next text in active document as specified 
	//HRESULT FindFirst(LPCTSTR szText);
	//Finds first next text in active document as specified. Returns false if end of document reached.
	//bool FindFirstBool(LPCTSTR szText);
	//Close all documents
	//HRESULT CloseDocuments(bool autoSave = true);
	//Copies selected text into clipboard
	//HRESULT Copy();
	////Pastes from Clipboard to active document
	//HRESULT Paste();
	////Sets selected text. Replace it with specified text.
	//HRESULT SetSelectionText(LPCTSTR szText);
	////Inserts picture from path specified in argument.
	//HRESULT InserPicture(LPCTSTR szFilename);
	////Inserts text in active document.
	//HRESULT InserText(LPCTSTR szText);
	//// Inserts MS Word file into active document with all formating.
	//HRESULT InsertFile(LPCTSTR szFilename);
	////Adds comment in ballon. Text is specified in argument
	//HRESULT AddComment(LPCTSTR szComment);
	////Moves cursor. 2 is forward, 1 i backward. Selection is true then it selects text as it moves
	//HRESULT MoveCursor(int nDirection = 2, bool bSelection = false);
	////Delete char forward or backward
	//HRESULT DeleteChar(bool bBack = false);
	////Sets bold for next inserted chars or for selected text.
	//HRESULT SetBold(bool bBold = false);
	////Sets italic for next inserted chars or for selected text.
	//HRESULT SetItalic(bool bItalic = false);
	////Sets underline for next inserted chars or for selected text.
	//HRESULT SetUnderline(bool bUnderline = false);
	////Check spelling
	//HRESULT CheckSpelling(LPCTSTR szWord, bool& bResult);
	////check grammer
	//HRESULT CheckGrammer(LPCTSTR szString, bool& bResult);
	////sets font as specified
	//HRESULT SetFont(LPCTSTR szFontName, int nSize, bool bBold, bool bItalic, COLORREF crColor);
	////Gets string of specified size
	//std::string GetString(int nlenght);
	////Get selected string
	//std::string GetSelectedString();
	////Saves file in path given by parameter
	//HRESULT SaveFile(LPCTSTR czFileName);
	////sets Align Justify
	//HRESULT AlignJustify();
	////sets Align Left
	//HRESULT AlignLeft();
	////sets Align Right
	//HRESULT AlignRight();
	////sets Align Center
	//HRESULT AlignCenter();
	////Creates document from Template
	//HRESULT AddDoc(LPCTSTR templateName);
	////Selects all content from document
	//HRESULT SelectAll();

};
#endif