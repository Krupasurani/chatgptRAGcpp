#pragma once

#define defTableDOCUMENT strCreateDOCUMENTTable
#define defTableCOMMENTS strCreateCOMMENTSTable


// 




// 



#define DOCUMENT_TABLENAME					L"DOCUMENT"
#define COMMENTS_TABLENAME					L"COMMENTS"


#define strTEXT_NOT_NULL					L" NO NULL"


#define DB_FIELD_NAME_UID				L"UID"
#define DB_FILED_NAME_LASTUPDATE		L"LASTUPDATE"
#define DB_FIELD_NAME_DOCUMENTID		L"DocumentID"
#define DB_FIELD_NAME_DOCUMENTNAME		L"DocumentName"
#define DB_FIELD_NAME_DOCUMENTPATH		L"DocumentPath"

#define DOCUMENT_COMMENTS				L"COMMENTS"
#define DB_FIELD_NAME_COMMENTDATE		L"CommentDate"
#define DB_FIELD_NAME_COMMENTAUTHOR		L"CommentAuthor"
#define DB_FIELD_NAME_COMMENTTEXT		L"CommentText"
#define DB_FIELD_NAME_ASSOCIATEDTEXT	L"AssociatedText"
#define DB_FIELD_NAME_AUTOAI_QUERY		L"AutoAIQuery"
#define DB_FIELD_NAME_AUTOAI_RESPONSE	L"AutoAIResponse"
/*

The following source code is proprietary of Secured Globe, Inc. and is used under
source code license given to this project only.
(c)2015-2020 Secured Globe, Inc.

*/
extern wchar_t DBFILENAME[1024];
extern CppSQLite3DB SG_DB_Handle;

#define LOCALDB_SQL_NEEDBRACKETS(f) ((f) == SG_DBDataType::SFDBDataString)

using namespace std;
// SecureHash basic data type
struct User
{
	int id = 0;
	std::wstring name;
	std::wstring hash;
	bool valid = false;
};


// Secured Globe DB Data Type
typedef enum _SG_DBDataType
{
	SG_DBDataInt,			// Integer
	SG_DBDataReal,			// Real / Double
	SFDBDataString,			// String
	SG_DBDataBool,			// Boolean
	SG_DBDataErr			// Error
}SG_DBDataType;

// Secured Globe DB Field
typedef struct _SG_DBField
{
	LPCWSTR FieldName;		// Field Name
	LPCWSTR FieldValue;		// Field Value (as String)
	SG_DBDataType ValueType;// Field Type (SG_DBDataType)
	_SG_DBField()			// struct's constractors
	{
		ValueType = SFDBDataString;
	};
	_SG_DBField(LPCWSTR FN, LPCWSTR FV, SG_DBDataType FVType = SFDBDataString)
	{
		FieldName = FN;
		FieldValue = FV;
		ValueType = FVType;
	};
} SG_DBField;

// Secured Globe DB Field Values
typedef struct _SG_DBFieldValues
{
	LPCWSTR FieldName;		// Field Name
	void* pValueVector;		// Pointer to vector of value
	SG_DBDataType ValueType;// Value type
	_SG_DBFieldValues()
	{
		ValueType = SFDBDataString;
		pValueVector = NULL;
	};
	_SG_DBFieldValues(LPCWSTR FN, void* pV, SG_DBDataType FVType = SFDBDataString)
	{
		FieldName = FN;
		pValueVector = pV;
		ValueType = FVType;
	};
} SG_DBFieldValues;

/* End Secured Globe proprietary */
namespace CDataProvider
{

	bool AddCommentAndTextToDB(std::wstring Comment, std::wstring Text);
	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<std::wstring> *ReturnValues);
	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<int> *ReturnValues);
	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<std::wstring>* ReturnValues, vector<SG_DBField> SearchPair);
	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<int>* ReturnValues, vector<SG_DBField> SearchPair);

	
	bool FindStringValueInTable(LPWSTR TableName, vector<SG_DBField> SearchPair);
	bool GetUniqueStrValue(LPWSTR TableName, LPWSTR FieldName, TCHAR *ReturnValue, int nBuffLen);
	bool GetUniqueIntValue(LPWSTR TableName, LPWSTR FieldName, int *ReturnValue);

	
	void DeleteRecordsFromTable(CString TableName, vector<SG_DBField> SearchPair, BOOL bEqual = TRUE);

	void InsertRecord(LPCWSTR TableName, int argc, ...);
	BOOL RunQuery(CString p_szQuery);


	void BeginTransaction();
	void EndTransaction();

	CString MakeWhereStatement(const vector<SG_DBField> Data, BOOL bEqual = TRUE);


	BOOL AddValueToTable(CString Table, CString FieldName, CString Value, BOOL Unique = FALSE, CString UniqueField = L"", SG_DBDataType = SG_DBDataType::SFDBDataString);
	BOOL UpdateValueToTable(CString Table, int nIndex, CString FieldName, CString Value);
	BOOL UpdateValueToTable(CString Table, int nIndex, CString FieldName, int Value);
	BOOL UpdateValueToTableWithName(CString Table, CString SearchFieldName,
		CString SearchValue, CString FieldName, CString Value);
	BOOL UpdateValueToTableWithName(CString Table, CString SearchFieldName,
		CString SearchValue, CString FieldName, int Value);

	CString MakeInsertStatement(const CString& TableName, const CString& Fields, const CString& Values);
	CString MakeInsertStatementOrg(const CString &TableName, const CString &Fields, const CString &Values);

	void MakePairs(CString &Pairs, const int argc, va_list vl);
		 
	void MakeFieldsValues(CString &Fields, CString &Values, const int argc, va_list vl);
	int InsertOrUpdate(LPCWSTR TableName, vector<SG_DBField> UniquePairs, int argc, ...);

	void InsertRecordUnique(LPCWSTR TableName, SG_DBField UniquePair, int argc, ...);
	int InsertRecordUniqueOrg(LPCWSTR TableName, SG_DBField UniquePair, int argc, ...);

	CString MakeUpdateStatement(const CString &TableName, const vector<SG_DBField> Data);
	CString MakeUpdateStatement(const CString &TableName, const CString &Pairs);

	void InitStrings();

	void DoEvents();

	CppSQLite3Query execQueryWithWait(LPCTSTR p_szSQLQuery, BOOL p_bDML = FALSE);
	CAtlString PrepareValue(const CAtlString &value, BOOL bEqual = TRUE);
	void DeleteDB();
	BOOL OpenDB();
	void CloseDB();
	void InitDB();
	void UpdateRecordUnique(LPCWSTR TableName, SG_DBField UniquePair, vector<SG_DBField> DBF);
	void UpdateRecordUnique(LPCWSTR TableName, vector<SG_DBField> UniquePair, vector<SG_DBField> DBF);


}