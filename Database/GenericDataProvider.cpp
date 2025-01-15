#include "..\stdafx.h"
#include "..\utils.h"
#include "CppSQLite3U.h"
#include "GenericDataProvider.h"

using namespace std;

wchar_t DBFILENAME[1024]{ L"sg.db" };
wchar_t strCreateDOCUMENTTable[1024];
wchar_t strCreateCOMMENTSTable[1024];

//static CCritSec SqlQueueCritSect;
CppSQLite3DB SG_DB_Handle;
BOOL DB_Open = FALSE;



namespace CDataProvider
{

	/*

	GetAllValues()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<std::wstring> *ReturnValues)
	{
		WriteLogFile(L"Getting all values from %s -> %s", TableName, FieldName);
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		(*ReturnValues).clear();
		sql.Format(L"SELECT %s FROM %s", FieldName, TableName);
		//WriteLogFile(L"Running query %s",sql.GetBuffer());

		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
				e.errorCodeAsString(e.errorCode()), sql);
			Sleep(1500);

			return 0;
		}
		if (SqlQuery.eof())
		{
			WriteLogFile(L"No results were found\n");
		}
		while (!SqlQuery.eof())	// we assume there is always one file with one code
		{
			//WriteLogFile(L"Found field %s value %s", FieldName, SqlQuery.getStringField(FieldName));

			(*ReturnValues).push_back((LPWSTR)SqlQuery.getStringField(FieldName));
			SqlQuery.nextRow();
		}
		//WriteLogFile(L"Getting all values success");

		return true;
	}

	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<int>* ReturnValues)
	{
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		(*ReturnValues).clear();
		sql.Format(L"SELECT %s FROM %s", FieldName, TableName);
		//WriteLogFile(L"Running query %s", sql.GetBuffer());

		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
				e.errorCodeAsString(e.errorCode()), sql);
			Sleep(1500);

			return 0;
		}
		while (!SqlQuery.eof())	// we assume there is always one file with one code
		{
			WriteLogFile(L"Found field %s value %s", FieldName, SqlQuery.getStringField(FieldName));

			(*ReturnValues).push_back(SqlQuery.getIntField(FieldName));
			SqlQuery.nextRow();
		}
		return true;
	}

	bool AddCommentAndTextToDB(std::wstring Comment, std::wstring Text)
	{
		bool result{ false };
		int Updated = InsertOrUpdate(
			COMMENTS_TABLENAME,
			{ SG_DBField(DB_FIELD_NAME_COMMENTTEXT, Comment.c_str()) },
			2,
			SG_DBField(DB_FIELD_NAME_COMMENTTEXT, Comment.c_str()),
			SG_DBField(DB_FIELD_NAME_ASSOCIATEDTEXT, Text.c_str())
		);
		return (Updated)?1:0;
	}
	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<std::wstring>* ReturnValues, vector<SG_DBField> SearchPair)
	{
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		CString w_szWhere;
		(*ReturnValues).clear();
		w_szWhere = MakeWhereStatement(SearchPair);
		sql.Format(L"SELECT %s FROM %s %s", FieldName, TableName, w_szWhere);
		//WriteLogFile(L"Running query %s", sql.GetBuffer());

		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
				e.errorCodeAsString(e.errorCode()), sql);
			Sleep(1500);

			return 0;
		}
		while (!SqlQuery.eof())	// we assume there is always one file with one code
		{
			WriteLogFile(L"Found field %s value %s", FieldName, SqlQuery.getStringField(FieldName));

			(*ReturnValues).push_back((LPWSTR)SqlQuery.getStringField(FieldName));
			SqlQuery.nextRow();
		}
		return true;
	}

	bool GetAllValues(LPWSTR TableName, LPWSTR FieldName, std::vector<int>* ReturnValues, vector<SG_DBField> SearchPair)
	{
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		CString w_szWhere;
		(*ReturnValues).clear();
		w_szWhere = MakeWhereStatement(SearchPair);
		sql.Format(L"SELECT %s FROM %s %s", FieldName, TableName, w_szWhere);
		//WriteLogFile(L"Running query %s", sql.GetBuffer());

		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
				e.errorCodeAsString(e.errorCode()), sql);
			Sleep(1500);

			return 0;
		}
		while (!SqlQuery.eof())	// we assume there is always one file with one code
		{
			WriteLogFile(L"Found field %s value %s", FieldName, SqlQuery.getStringField(FieldName));

			(*ReturnValues).push_back(SqlQuery.getIntField(FieldName));
			SqlQuery.nextRow();
		}
		return true;
	}





	CString GetLatestSyncDate(const CString& TableName)
	{
		CString sql;
		CppSQLite3Query w_sqlResult;

		sql.Format(L"SELECT MAX(LASTSYNCDATE) FROM %s", TableName);
		try
		{
			WriteLogFile(L"Running query: '%s'", sql.GetBuffer());

			w_sqlResult = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"SQL error when getting out of sync rows for %s: %s\n", TableName, e.errorCodeAsString(e.errorCode()));
			return L"";
		}

		if (w_sqlResult.eof())
		{
			return L"0";
		}
		else
		{
			CString w_szResult = w_sqlResult.fieldValue(0);
			if (w_szResult == L"")
				return L"0";
			return w_szResult;
		}
	}

	/*

	FindStringValueInTable()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	bool FindStringValueInTable(LPWSTR TableName, vector<SG_DBField> SearchPair)
	{
		BOOL Found = FALSE;
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		CString w_szWhere;

		w_szWhere = MakeWhereStatement(SearchPair);
		sql.Format(L"SELECT * FROM %s %s", TableName, w_szWhere);

		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"SQL Error %s in query", e.errorCodeAsString(e.errorCode()));
			return 0;
		}

		while (!SqlQuery.eof())	// we assume there is always one file with one code
		{
			LPWSTR ReturnValue = (LPWSTR)SqlQuery.getStringField(SearchPair[0].FieldName);
			if (wcscmp(ReturnValue, SearchPair[0].FieldValue) == 0)
			{
				Found = TRUE;
				return TRUE;
			}
			SqlQuery.nextRow();
		}
		return false;
	}
	/*

	GetUniqueStrValue()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	bool GetUniqueStrValue(LPWSTR TableName, LPWSTR FieldName, TCHAR *ReturnValue, int nBuffLen)
	{
		CppSQLite3Query SqlQuery;
		CString sql;
		sql.Format(L"SELECT %s FROM %s", FieldName, TableName);
		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
				e.errorCodeAsString(e.errorCode()), sql);
			return 0;
		}
		if (!SqlQuery.eof())	// we assume there is always one file with one code
		{
			_tcscpy_s(ReturnValue, nBuffLen, SqlQuery.getStringField(FieldName));

			return true;
		}
		else
		{
			return false;
		}
	}

	/*

	GetUniqueIntValue()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	bool GetUniqueIntValue(LPWSTR TableName, LPWSTR FieldName, int *ReturnValue)
	{
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		sql.Format(L"SELECT %s FROM %s", FieldName, TableName);
		try
		{
			SqlQuery = execQueryWithWait(sql.GetBuffer());
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
				e.errorCodeAsString(e.errorCode()), sql);
			return 0;
		}
		if (!SqlQuery.eof())	// we assume there is always one file with one code
		{
			*ReturnValue = _wtol(SqlQuery.getStringField(FieldName));
			return true;
		}
		else
		{
			return false;
		}
	}




	void DeleteRecordsFromTable(CString TableName, vector<SG_DBField> SearchPair, BOOL bEqual/* = TRUE*/)
	{
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		CString w_szWhere;

		w_szWhere = MakeWhereStatement(SearchPair, bEqual);
		sql.Format(L"DELETE FROM %s %s", TableName, w_szWhere);
		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
				e.errorCodeAsString(e.errorCode()), sql);
			return;
		}


	}

	void InsertRecord(LPCWSTR TableName, int argc, ...)
	{
		OpenDB();
		CString Fields, Values, Pairs;

		va_list argptr;
		va_start(argptr, argc);
		MakeFieldsValues(Fields, Values, argc, argptr);
		MakePairs(Pairs, argc, argptr);
		va_end(argptr);

		CString sql;
		wstring insertQuery;

		insertQuery = MakeInsertStatement(TableName, Fields, Values);

		try
		{
			execQueryWithWait(insertQuery.c_str(), TRUE);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"	 when getting list of tables: %s\n", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

		}

	}


	/*

	RunQuery()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/


	BOOL RunQuery(CString p_szQuery)
	{
		OpenDB();

		WriteLogFile(L"Running query: '%s'", p_szQuery.GetBuffer());
		try
		{
			execQueryWithWait(p_szQuery.GetBuffer(), TRUE);
			p_szQuery.ReleaseBuffer();
			//logfunc(L"Success");
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Quey execution failed %s", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

			return FALSE;
		}
		CloseDB();
		return TRUE;
	}

	void BeginTransaction()
	{
		//try
		//{
		//	execQueryWithWait(L"BEGIN TRANSACTION");
		//}
		//catch (CppSQLite3Exception &e)
		//{
		//}
	}

	void EndTransaction()
	{
		//try
		//{
		//	execQueryWithWait(L"COMMIT");
		//}
		//catch (CppSQLite3Exception &e)
		//{
		//}
	}



	
	
	
	void CloseDB()
	{
		try
		{
			if (SG_DB_Handle.mpDB)
				SG_DB_Handle.close();
		}
		catch (CppSQLite3Exception & e)
		{
			WriteLogFile(L"Error closing db. Error %s", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

		}
		DB_Open = false;
	}



	CString MakeWhereStatement(const vector<SG_DBField> Data, BOOL bEqual/* = TRUE*/)
	{
		CString result = L"";
		result.Format(L" WHERE ");
		for (size_t i = 0; i < Data.size(); i++)
		{
			CString CleanValue{ L"" };
			if (LOCALDB_SQL_NEEDBRACKETS(Data[i].ValueType)) // Why not be positive? 
				CleanValue += PrepareValue(Data[i].FieldValue, bEqual);
			else
				CleanValue += Data[i].FieldValue;

			if ((bEqual) ||
				(LOCALDB_SQL_NEEDBRACKETS(Data[i].ValueType) == FALSE))
			{
				result += (CString)Data[i].FieldName + L" = " + CleanValue;
			}
			else
			{
				result += (CString)Data[i].FieldName + L" LIKE " + CleanValue;
			}
			if (i < Data.size() - 1) result += L" and ";
		}
		return result;
	}


	BOOL AddValueToTable(CString Table, CString FieldName, CString Value, BOOL Unique, CString UniqueField, SG_DBDataType type)
	{
		OpenDB();
		CString InsertQuery;
		if (LOCALDB_SQL_NEEDBRACKETS(type))
			Value = PrepareValue(Value);
		if (Unique && UniqueField != L"")
		{
			InsertQuery.Format(L"insert into %s(%s)Select %s Where not exists(select * from %s where %s=%s)",
				Table.GetBuffer(),
				FieldName.GetBuffer(),
				Value.GetBuffer(),
				Table.GetBuffer(),
				FieldName.GetBuffer(),
				Value.GetBuffer());
		}
		else
		{
			InsertQuery.Format(L"insert into %s(%s) values(%s)",
				Table.GetBuffer(),
				FieldName.GetBuffer(),
				Value.GetBuffer());

		}
		WriteLogFile(L"Running an INSERT query: '%s'", InsertQuery);

		try
		{
			execQueryWithWait(InsertQuery.GetBuffer(), TRUE);
			WriteLogFile(L"Success");
		}
		catch (CppSQLite3Exception & e)
		{
			WriteLogFile(L"Quey execution failed %s", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

			return FALSE;
		}
		CloseDB();
		return TRUE;
	}


	/*

	UpdateValueToTable()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	BOOL UpdateValueToTable(CString Table, int nIndex, CString FieldName, CString Value)
	{
		OpenDB();

		CString UpdateQuery;
		UpdateQuery.Format(L"UPDATE %s SET %s='%s' WHERE UID=%d",
			Table.GetBuffer(),
			FieldName.GetBuffer(),
			Value.GetBuffer(),
			nIndex);

		WriteLogFile(L"Running an UPDATE query: '%s'", UpdateQuery);

		try
		{
			execQueryWithWait(UpdateQuery.GetBuffer(), TRUE);
			WriteLogFile(L"Success");
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Quey execution failed %s", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

			return FALSE;
		}
		CloseDB();
		return TRUE;
	}

	/*

	UpdateValueToTable()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	BOOL UpdateValueToTable(CString Table, int nIndex, CString FieldName, int Value)
	{
		OpenDB();

		CString UpdateQuery;

		UpdateQuery.Format(L"UPDATE %s SET %s=%d WHERE UID=%d",
			Table.GetBuffer(),
			FieldName.GetBuffer(),
			Value,
			nIndex);

		WriteLogFile(L"Running an UPDATE query: '%s'", UpdateQuery);

		try
		{
			execQueryWithWait(UpdateQuery.GetBuffer(), TRUE);
			WriteLogFile(L"Success");
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Quey execution failed %s", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

			return FALSE;
		}
		CloseDB();
		return TRUE;
	}

	/*

	UpdateValueToTableWithName()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	BOOL UpdateValueToTableWithName(CString Table, CString SearchFieldName,
		CString SearchValue, CString FieldName, CString Value)
	{
		OpenDB();

		CString UpdateQuery;

		UpdateQuery.Format(L"UPDATE %s SET %s='%s' WHERE %s='%s'",
			Table.GetBuffer(),
			FieldName.GetBuffer(),
			Value.GetBuffer(),
			SearchFieldName.GetBuffer(),
			SearchValue.GetBuffer());

		WriteLogFile(L"Running an UPDATE query: '%s'", UpdateQuery);

		try
		{
			execQueryWithWait(UpdateQuery.GetBuffer(), TRUE);
			WriteLogFile(L"Success");
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Quey execution failed %s", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

			return FALSE;
		}
		CloseDB();
		return TRUE;
	}

	/*

	UpdateValueToTableWithName()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/


	BOOL UpdateValueToTableWithName(CString Table, CString SearchFieldName,
		CString SearchValue, CString FieldName, int Value)
	{
		OpenDB();

		CString UpdateQuery;

		UpdateQuery.Format(L"UPDATE %s SET %s=%d WHERE %s='%s'",
			Table.GetBuffer(),
			FieldName.GetBuffer(),
			Value,
			SearchFieldName.GetBuffer(),
			SearchValue.GetBuffer());

		WriteLogFile(L"Running an UPDATE query: '%s'", UpdateQuery);

		try
		{
			execQueryWithWait(UpdateQuery.GetBuffer(), TRUE);
			WriteLogFile(L"Success");
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Quey execution failed %s", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

			return FALSE;
		}
		CloseDB();
		return TRUE;
	}



	CString MakeInsertStatementOrg(const CString &TableName, const CString &Fields, const CString &Values)
	{
		CString result = L"";
		CString CleanValues = Values;
		if (CleanValues.Right(1) == L",")
		{
			CleanValues = CleanValues.Left(CleanValues.GetLength() - 1);
		}
		if (CleanValues.Left(1) == L",")
		{
			CleanValues = CleanValues.Mid(2);
		}

		result.Format(L"INSERT INTO %s ( %s ,%s ) VALUES (%s , '%s')",
			TableName, Fields, DB_FILED_NAME_LASTUPDATE,CleanValues,CurrentTimeForUpdate().c_str());

		return result;
	}



	/*

	MakePairs()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	void MakePairs(CString &Pairs, const int argc, va_list vl)
	{
		Pairs.Empty();
		for (int i = 0; i < argc; i++)
		{
			SG_DBField DBF = va_arg(vl, SG_DBField);

			if (DBF.ValueType == SG_DBDataType::SFDBDataString)
				Pairs += (CAtlString)DBF.FieldName + L"=" + PrepareValue(DBF.FieldValue);
			else
				Pairs += (CAtlString)DBF.FieldName + L"=" + DBF.FieldValue;

			if (i < (argc - 1))
			{
				Pairs += L",";
			}
		}
	}



	/*

	MakeFieldsValues()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	void MakeFieldsValues(CString &Fields, CString &Values, const int argc, va_list vl)
	{
		Fields.Empty();
		Values.Empty();
		for (int i = 0; i < argc; i++)
		{
			SG_DBField DBF = va_arg(vl, SG_DBField);
			Fields += DBF.FieldName;
			if (LOCALDB_SQL_NEEDBRACKETS(DBF.ValueType)) // Why not be positive? 
				Values += PrepareValue(DBF.FieldValue);
			else
				Values += DBF.FieldValue;
			if (i < (argc - 1))
			{
				Fields += L",";
				Values += L",";
			}
		}
	}



	int InsertOrUpdate(LPCWSTR TableName, vector<SG_DBField> UniquePairs, int argc, ...)
	{
		OpenDB();
		CString Fields, Values, Pairs;

		va_list argptr;
		va_start(argptr, argc);
		MakeFieldsValues(Fields, Values, argc, argptr);
		MakePairs(Pairs, argc, argptr);
		va_end(argptr);
		CString where = L"";

		if(UniquePairs.size())
		{
			if (UniquePairs[0].ValueType == SG_DBDataType::SFDBDataString)
				where += (CString)L" WHERE " + UniquePairs[0].FieldName + L" = " + PrepareValue(UniquePairs[0].FieldValue);
			else
				where += (CString)L" WHERE " + UniquePairs[0].FieldName + L" = " + UniquePairs[0].FieldValue;
			for(int i=1; i< UniquePairs.size(); i++)
			{
				where += L" OR ";
				if (UniquePairs[i].ValueType == SG_DBDataType::SFDBDataString)
					where += UniquePairs[i].FieldName + (CString)L" = " + PrepareValue(UniquePairs[i].FieldValue);
				else
					where += UniquePairs[i].FieldName + (CString)L" = " + UniquePairs[i].FieldValue;
			}
			WriteLogFile(L"InsertOrUpdate: UniquePairs: %s",where.GetString());
		}
		else
		{
			WriteLogFile(L"InsertOrUpdate: No UniquePairs were sent");
			return 0;
		}
		CppSQLite3Query queryResult;
		CString sql;
		wstring insertQuery;
		insertQuery = MakeInsertStatementOrg(TableName, Fields, Values);

		wstring updateQuery;
		updateQuery = MakeUpdateStatement(TableName, Pairs);
		updateQuery += L" ";
		updateQuery += where;

		sql.Format(L"SELECT * FROM %s %s", TableName, where);
		WriteLogFile(L"Running query '%s'", sql.GetBuffer());
		try
		{
			queryResult = execQueryWithWait(sql);
			CString msg;
			if (!queryResult.eof())
			{
				bool bRecExists{false};
				for(int i=0;i<UniquePairs.size();i++)
				{
					CString name = queryResult.fieldValue(UniquePairs[i].FieldName);
					msg+= UniquePairs[i].FieldName;
					msg+= L" = ";
					msg+= UniquePairs[i].FieldValue;
					msg+= L"\n";
					if (name == UniquePairs[i].FieldValue) 
					{
						bRecExists = true;
						break;
					}
					else
						msg=L"";
					
				}
				
				if(bRecExists)
				{
					WriteLogFile(L"Record already exists %s", msg);
					va_start(argptr, argc);

					for (int i = 0; i < argc; i++)
					{
						SG_DBField DBF = va_arg(argptr, SG_DBField);
						CString temp = queryResult.fieldValue(DBF.FieldName);
						if (wcscmp(temp.GetBuffer(), DBF.FieldValue))
						{
							WriteLogFile(L"Changes detected\nField %s was changed: Current value %s. New Value %s\nRunning update query %s",
								DBF.FieldName,
								queryResult.fieldValue(DBF.FieldName),
								DBF.FieldValue,
								updateQuery.c_str());
							try
							{
								execQueryWithWait(updateQuery.c_str(), TRUE);
							}
							catch (CppSQLite3Exception& e)
							{
								WriteLogFile(L"Error running query '%s': %s\n", updateQuery.c_str(), e.errorCodeAsString(e.errorCode()));
								Sleep(1500);
							}
							
							va_end(argptr);
	
							return 1;
						}
						
						else
						{
							WriteLogFile(L"no changes detected\nField %s was not changed: Current value %s. New Value %s",
								DBF.FieldName,
								queryResult.fieldValue(DBF.FieldName),
								DBF.FieldValue);
						}
					}
					va_end(argptr);

					return 0;
				}
			}
			WriteLogFile(L"Entirely new record, running an insert query %s", insertQuery.c_str());

			execQueryWithWait(insertQuery.c_str(), TRUE);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Error running query '%s': %s\n", insertQuery.c_str(), e.errorCodeAsString(e.errorCode()));
			Sleep(1500);
			return 2;
		}

		return 0;
	}


	/*

	MakeUpdateStatement()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/


	CString MakeUpdateStatement(const CString &TableName, const vector<SG_DBField> Data)
	{
		CString result = L"";
		result.Format(L"UPDATE %s SET ", TableName);
		for (size_t i = 0; i < Data.size(); i++)
		{
			CString CleanValue{ L"" };
			if (LOCALDB_SQL_NEEDBRACKETS(Data[i].ValueType)) // Why not be positive? 
				CleanValue += PrepareValue(Data[i].FieldValue);
			else
				CleanValue += Data[i].FieldValue;

			result += (CString)Data[i].FieldName + L" = " + CleanValue;
			if (i < Data.size() - 1) result += L", ";
		}
		return result;
	}

	CString MakeUpdateStatement(const CString &TableName, const CString &Pairs)
	{
		return L"UPDATE " + TableName + L" SET " + L" LASTUPDATE='" + CurrentTimeForUpdate().c_str() + L"'," + Pairs;
	}



	/*
	InsertRecordUnique()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.
	*/

	void InsertRecordUnique(LPCWSTR TableName, SG_DBField UniquePair, int argc, ...)
	{
		OpenDB();
		CString Fields, Values, Pairs;

		va_list argptr;
		va_start(argptr, argc);
		MakeFieldsValues(Fields, Values, argc, argptr);
		MakePairs(Pairs, argc, argptr);
		va_end(argptr);
		CString where = L"";

		if (UniquePair.ValueType == SG_DBDataType::SFDBDataString)
			where += (CString)L" WHERE " + UniquePair.FieldName + L" = " + PrepareValue(UniquePair.FieldValue);
		else
			where += (CString)L" WHERE " + UniquePair.FieldName + L" = " + UniquePair.FieldValue;

		CppSQLite3Query queryResult;
		CString sql;
		wstring insertQuery;

		insertQuery = MakeInsertStatement(TableName, Fields, Values);
		sql.Format(L"SELECT %s FROM %s WHERE UID=(SELECT MAX(UID) FROM %s);", UniquePair.FieldName, TableName, TableName);
		//WriteLogFile(L"Running query '%s'", sql.GetBuffer());
		try
		{
			queryResult = execQueryWithWait(sql);

			if (!queryResult.eof())
			{
				CString name = queryResult.fieldValue(0);
				if (name == UniquePair.FieldValue)
				{
					WriteLogFile(L"Record already exists %s", name);

					return;
				}
			}
			execQueryWithWait(insertQuery.c_str(), TRUE);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"	 when getting list of tables: %s\n", e.errorCodeAsString(e.errorCode()));
			Sleep(1500);
		}
	}

	int InsertRecordUniqueOrg(LPCWSTR TableName, SG_DBField UniquePair, int argc, ...)
	{
		OpenDB();
		CString Fields, Values, Pairs;

		va_list argptr;
		va_start(argptr, argc);
		MakeFieldsValues(Fields, Values, argc, argptr);
		MakePairs(Pairs, argc, argptr);
		va_end(argptr);
		CString where = L"";

		if (UniquePair.ValueType == SG_DBDataType::SFDBDataString)
			where += (CString)L" WHERE " + UniquePair.FieldName + L" = " + PrepareValue(UniquePair.FieldValue);
		else
			where += (CString)L" WHERE " + UniquePair.FieldName + L" = " + UniquePair.FieldValue;

		CppSQLite3Query queryResult;
		CString sql;
		wstring insertQuery;

		insertQuery = MakeInsertStatementOrg(TableName, Fields, Values);
		sql.Format(L"SELECT %s FROM %s WHERE UID=(SELECT MAX(UID) FROM %s);", UniquePair.FieldName, TableName, TableName);
		//WriteLogFile(L"Running query '%s'", sql.GetBuffer());
		try
		{
			queryResult = execQueryWithWait(sql);

			if (!queryResult.eof())
			{
				CString name = queryResult.fieldValue(UniquePair.FieldName);
				if (name == UniquePair.FieldValue)
				{
					WriteLogFile(L"Record already exists %s", name);

					return 1;
				}
			}
			execQueryWithWait(insertQuery.c_str(), TRUE);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Error running query '%s': %s\n", insertQuery.c_str(), e.errorCodeAsString(e.errorCode()));
			Sleep(1500);
			return 2;
		}

		return 0;
	}

	/*

	MakeInsertStatement()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	CString MakeInsertStatement(const CString& TableName, const CString& Fields, const CString& Values)
	{
		CString result = L"";
		CAtlString CleanDate = CurrentDate(), CleanValues = Values;
		if (CleanValues.Right(1) == L",")
		{
			CleanValues = CleanValues.Left(CleanValues.GetLength() - 1);
		}
		if (CleanValues.Left(1) == L",")
		{
			CleanValues = CleanValues.Mid(2);
		}

		result.Format(L"INSERT INTO %s (%s, %s, %s ) VALUES (NULL, %s, %s)",
			TableName, DB_FIELD_NAME_UID, DB_FILED_NAME_LASTUPDATE, Fields, CleanDate, CleanValues);

		return result;
	}

	/*
	
#define DOCUMENT_COMMENTS				L"COMMENTS"
#define DB_FIELD_NAME_COMMENTDATE		L"CommentDate"
#define DB_FIELD_NAME_COMMENTAUTHOR		L"CommentAuthor"
#define DB_FIELD_NAME_COMMENTTEXT		L"CommentText"
#define DB_FIELD_NAME_ASSOCIATEDTEXT	L"AssociatedText"
#define DB_FIELD_NAME_AUTOAI_QUERY		L"AutoAIQuery"
#define DB_FIELD_NAME_AUTOAI_RESPONSE	L"AutoAIResponse"
	*/
	void InitStrings()
	{
		// DOCUMENT Table
		wcscpy(strCreateDOCUMENTTable, L"CREATE TABLE IF NOT EXISTS ");
		wcscat(strCreateDOCUMENTTable, DOCUMENT_TABLENAME);
		wcscat(strCreateDOCUMENTTable, L"(");
		wcscat(strCreateDOCUMENTTable, DB_FIELD_NAME_UID);					// UID
		wcscat(strCreateDOCUMENTTable, L" INTEGER PRIMARY KEY AUTOINCREMENT, ");
		wcscat(strCreateDOCUMENTTable, DB_FILED_NAME_LASTUPDATE);			// Last Update date
		wcscat(strCreateDOCUMENTTable, L" DATE, ");
		wcscat(strCreateDOCUMENTTable, DB_FIELD_NAME_DOCUMENTNAME);			// Document name
		wcscat(strCreateDOCUMENTTable, L" TEXT, ");
		wcscat(strCreateDOCUMENTTable, DB_FIELD_NAME_DOCUMENTPATH);			// Document file path
		wcscat(strCreateDOCUMENTTable, L" TEXT);");

		// COMMENTS Table
		wcscpy(strCreateCOMMENTSTable, L"CREATE TABLE IF NOT EXISTS ");
		wcscat(strCreateCOMMENTSTable, COMMENTS_TABLENAME);
		wcscat(strCreateCOMMENTSTable, L"(");
		wcscat(strCreateCOMMENTSTable, DB_FIELD_NAME_UID);
		// UID
		wcscat(strCreateCOMMENTSTable, L" INTEGER PRIMARY KEY AUTOINCREMENT, ");
		wcscat(strCreateCOMMENTSTable, DB_FIELD_NAME_COMMENTDATE);			// Date of comment 
		wcscat(strCreateCOMMENTSTable, L" DATE, ");
		wcscat(strCreateCOMMENTSTable, DB_FILED_NAME_LASTUPDATE);			// Last Update date
		wcscat(strCreateCOMMENTSTable, L" DATE, ");
		wcscat(strCreateCOMMENTSTable, DB_FIELD_NAME_COMMENTAUTHOR);		// Author of comment
		wcscat(strCreateCOMMENTSTable, L" TEXT, ");
		wcscat(strCreateCOMMENTSTable, DB_FIELD_NAME_COMMENTTEXT);			// Text of comment
		wcscat(strCreateCOMMENTSTable, L" TEXT, ");
		wcscat(strCreateCOMMENTSTable, DB_FIELD_NAME_ASSOCIATEDTEXT);		// Associated text
		wcscat(strCreateCOMMENTSTable, L" TEXT, ");
		wcscat(strCreateCOMMENTSTable, DB_FIELD_NAME_AUTOAI_QUERY);			// Query sent to ChatGPT
		wcscat(strCreateCOMMENTSTable, L" TEXT, ");
		wcscat(strCreateCOMMENTSTable, DB_FIELD_NAME_AUTOAI_RESPONSE);			// Response received from ChatGPT
		wcscat(strCreateCOMMENTSTable, L" TEXT);");

	}


	void DoEvents()
	{
		MSG	msg;

		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// exec query
	CppSQLite3Query execQueryWithWait(LPCTSTR p_szSQLQuery, BOOL p_bDML/* = FALSE*/)
	{
		//WriteLogFile(L"execQueryWithWait '%s'", p_szSQLQuery);
		OpenDB();
		//CAutoLock w_crsLockLog(&SqlQueueCritSect);
		CppSQLite3Query w_queryResult{};

		if (SG_DB_Handle.mpDB == NULL) SG_DB_Handle.open(DBFILENAME);

		if (p_bDML)
			SG_DB_Handle.execDML(p_szSQLQuery);
		else
			w_queryResult = SG_DB_Handle.execQuery(p_szSQLQuery);

		return w_queryResult;
	}


	BOOL create_DB(BOOL deleteOld)
	{
		WCHAR *zErrMsg = 0;
		// if DB exists return TRUE without creating it
		if (PathFileExists(DBFILENAME))
		{
			if (deleteOld)
			{
				// Delete database it if already exists
				int result = DeleteFile(DBFILENAME);
				if (result == NULL)
				{
					WriteLogFile(L"Can't delete old DB %s", DBFILENAME);
				}

			}
			else
			{
				return TRUE;
			}
		}


		try
		{
			SG_DB_Handle.open(DBFILENAME);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"SQL Error: %s", e.errorCodeAsString(e.errorCode()));
			return false;
		}
		DB_Open = true;

		// Create all tables 
		BOOL result = FALSE;
		try
		{
			if (!SG_DB_Handle.tableExists(DOCUMENT_TABLENAME))
			{
				WriteLogFile(L"Creating table %s query: '%s'", 
					DOCUMENT_TABLENAME, 
					defTableDOCUMENT);
				execQueryWithWait(defTableDOCUMENT, TRUE);
			}
			if (!SG_DB_Handle.tableExists(COMMENTS_TABLENAME))
			{
				WriteLogFile(L"Creating table %s query: '%s'", 
					COMMENTS_TABLENAME, 
					defTableCOMMENTS);
				execQueryWithWait(defTableCOMMENTS, TRUE);
			}

		}
		catch (CppSQLite3Exception & e)
		{
			WriteLogFile(L"Error Creating database %s. Error: %s",
				DBFILENAME,
				e.errorCodeAsString(e.errorCode()));
			result = FALSE;
		}
		return true;
	}

	/*

	PrepareValue()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	CAtlString PrepareValue(const CAtlString &value, BOOL bEqual/* = TRUE*/)
	{
		CAtlString retVal;

		if (value == L"")
		{
			return (CString)L"'" + (CString)L"'";
		}
		CString CleanedValue = value;
		CleanedValue.Replace(L"'", L"''");
		CleanedValue.Replace(L",", L"");

		if (bEqual)
			retVal = (CString)L"'" + CleanedValue + (CString)L"'";
		else
			retVal = (CString)L"'%" + CleanedValue + (CString)L"%'";

		return retVal;
	}


	void DeleteDB()
	{
		WriteLogFile(L"Deleting Database [%s]",DBFILENAME);
		if (PathFileExists(DBFILENAME))	
		{
			DeleteFile(DBFILENAME);
		}
	}
	BOOL OpenDB()
	{
		bool result = FALSE;
		InitStrings();
		if (PathFileExists(DBFILENAME) == FALSE)	// needs to create DB
		{
			create_DB(FALSE);
		}
		if (DB_Open) goto db_open;
		// Open database 
		try
		{
			SG_DB_Handle.open(DBFILENAME);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"SQL Error: %s", e.errorCodeAsString(e.errorCode()));
			return false;
		}
	db_open:
		DB_Open = true;
		return true;
	}
	
	
	/*

	UpdateRecordUnique()
	The following source code is proprietary of Secured Globe, Inc. and is used under
	source code license given to this project only.
	(c)2015-2020 Secured Globe, Inc.

	*/

	void UpdateRecordUnique(LPCWSTR TableName, SG_DBField UniquePair, vector<SG_DBField> DBF)
	{
		OpenDB();
		CString Fields = UniquePair.FieldName, Values, Pairs;

		if (LOCALDB_SQL_NEEDBRACKETS(UniquePair.ValueType)) // Why not be positive? 
			Values += PrepareValue(UniquePair.FieldValue);
		else
			Values += UniquePair.FieldValue;

		CString where = L"";

		if (UniquePair.ValueType == SG_DBDataType::SFDBDataString)
			where += (CString)L" WHERE " + UniquePair.FieldName + L" = " + PrepareValue(UniquePair.FieldValue);
		else
			where += (CString)L" WHERE " + UniquePair.FieldName + L" = " + UniquePair.FieldValue;

		CppSQLite3Query queryResult;
		CString sql;
		sql = MakeUpdateStatement(TableName, DBF);
		sql += where;

		WriteLogFile(L"Execute SQL: '%s'", sql);

		try
		{
			queryResult = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"Error (Query: '%s' %s\n", sql, e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

		}
		if (queryResult.eof())
		{
			WriteLogFile(L"Updated\n");

		}

	}

	void InitDB()
	{
		OpenDB();
	}
	void UpdateRecordUnique(LPCWSTR TableName, vector<SG_DBField> UniquePair, vector<SG_DBField> DBF)
	{
		CString where = L"";

		OpenDB();

		CppSQLite3Query queryResult;
		CString sql;
		sql = MakeUpdateStatement(TableName, DBF);
		where = MakeWhereStatement(UniquePair);
		sql += where;

		WriteLogFile(L"Execute SQL: '%s'", sql);

		try
		{
			queryResult = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception& e)
		{
			WriteLogFile(L"Error (Query: '%s' %s\n", sql, e.errorCodeAsString(e.errorCode()));
			Sleep(1500);

		}
	}

	
	std::wstring convertMinutes(const std::wstring& inputMinutes) 
	{
		int minutes = std::stoi(inputMinutes);

		int days = minutes / 1440; // 1440 minutes in a day
		minutes %= 1440;
		int hours = minutes / 60;
		minutes %= 60;

		std::wstring result = std::to_wstring(days) + L":" + std::to_wstring(hours) + L":" + std::to_wstring(minutes);
		return result;
	}
	std::wstring formatDate(const std::wstring& dateTimeStr) 
	{
		// Convert input wstring to tm struct
		std::tm timeStruct = {};
		std::wistringstream ss(dateTimeStr);
		ss >> std::get_time(&timeStruct, L"%Y%m%d%H%M%S");

		// Format output wstring
		wchar_t buffer[80];
		std::wcsftime(buffer, 80, L"%d %b, %Y %T", &timeStruct);

		return std::wstring(buffer);
	}


	
	bool GetFieldNames(vector<wstring>&data)
	{
		data.clear();
		bool result = false;
		OpenDB();
		CppSQLite3Query SqlQuery;
		CString sql;
		sql.Format(L"SELECT * FROM 'DOCUMENT'");
		try
		{
			SqlQuery = execQueryWithWait(sql);
		}
		catch (CppSQLite3Exception &e)
		{
			WriteLogFile(L"SQL Error %s in query %s",
					e.errorCodeAsString(e.errorCode()), sql);
			Sleep(1500);

			return false;
		}
		for(int i=0;i<SqlQuery.numFields();i++)
		{
			data.insert(data.end(),SqlQuery.fieldName(i));
		}
		return true;
	}
	
	
}
