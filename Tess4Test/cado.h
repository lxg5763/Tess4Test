#pragma once
//#import "C:\\Program Files (x86)\\Common Files\\System\ado\\msado15.dll" no_namespace rename("EOF","adoEOF")
#import "../BIN/msado15.dll" no_namespace rename("EOF","adoEOF")
class CADO
{
public:

	_ConnectionPtr m_pConnection;
	_RecordsetPtr m_pRecordset;

	void  Connect(void);
	void __stdcall ExitConnect(void);
	_RecordsetPtr& __stdcall GetRecordset(_bstr_t SQL);

private:
};