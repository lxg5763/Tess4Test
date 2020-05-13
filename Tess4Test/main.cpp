#include<iostream>
#include <stdio.h>
#include <conio.h>
#include <afx.h>
#include<windows.h>

#include "cado.h"
#include<fstream>
#include <vector>
#include <string>
#include "WhatsAPP.h"
#include "Singal.h"
#include "Viber.h"
#include "VxNtpHelper.h"

#ifdef _WIN32
#include <WinSock2.h>
#endif // _WIN32

using namespace std;

char SQL_server[64] = { "127.0.0.1" };
char SQL_user[64] = { "sa" };
char SQL_pwd[64] = { "zcbADG!#%" };
char SQL_database[64] = { "manage" };

char s_connection[MAX_PATH] = { 0 };

int YEAR = 2022;
int MONTH = 1;
int DAY = 1;

#ifdef _WIN32

/**
* @class vxWSASocketInit
* @brief �Զ� ����/ж�� WinSock ��Ĳ����ࡣ
*/
class vxWSASocketInit
{
	// constructor/destructor
public:
	vxWSASocketInit(x_int32_t xit_main_ver = 2, x_int32_t xit_sub_ver = 0)
	{
		WSAStartup(MAKEWORD(xit_main_ver, xit_sub_ver), &m_wsaData);
	}

	~vxWSASocketInit(x_void_t)
	{
		WSACleanup();
	}

	// class data
protected:
	WSAData      m_wsaData;
};

#endif // _WIN32

void ANSIToUTF8(const char* ansi_str, string *utf8_result)
{
	long unicode_len;
	wchar_t * unicode_buf;

	unicode_len = MultiByteToWideChar(CP_ACP, 0, ansi_str, -1, NULL, 0);
	unicode_buf = (wchar_t *)malloc((unicode_len + 1) * sizeof(wchar_t));
	memset(unicode_buf, 0, (unicode_len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, ansi_str, -1, (LPWSTR)unicode_buf, unicode_len);

	long utf8_len = WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, NULL, 0, NULL, NULL);
	char *utf8_buf = (char *)malloc((utf8_len + 1) * sizeof(char));
	memset(utf8_buf, 0, sizeof(char) * (utf8_len + 1));
	WideCharToMultiByte(CP_UTF8, 0, unicode_buf, -1, utf8_buf, utf8_len, NULL, NULL);

	*utf8_result += utf8_buf;
	free(unicode_buf);
	free(utf8_buf);
}

/**********************************************************/
/**
 * @brief ���ʱ�����Ϣ��
 */
CString ts_output(x_cstring_t xszt_name, const x_ntp_time_context_t * const xtm_ctxt)
{
	printf("  %s : %04d-%02d-%02d_%02d-%02d-%02d.%03d\n",
		xszt_name,
		xtm_ctxt->xut_year,
		xtm_ctxt->xut_month,
		xtm_ctxt->xut_day,
		xtm_ctxt->xut_hour,
		xtm_ctxt->xut_minute,
		xtm_ctxt->xut_second,
		xtm_ctxt->xut_msec);

	CString time_Y, time_M, time_D, time_h, time_m, time_s;

	time_Y.Format(_T("%04d"), xtm_ctxt->xut_year);
	time_M.Format(_T("%02d"), xtm_ctxt->xut_month);
	time_D.Format(_T("%02d"), xtm_ctxt->xut_day);
	time_h.Format(_T("%02d"), xtm_ctxt->xut_hour - 8);
	time_m.Format(_T("%02d"), xtm_ctxt->xut_minute);
	time_s.Format(_T("%02d"), xtm_ctxt->xut_second);

	CString time_R;

	time_R = time_Y;
	time_R += " ";
	time_R += time_h;
	time_R += ":";
	time_R += time_m;
	time_R += ":";
	time_R += time_s;

	return time_R;
}

/************************************************************************/
/* ��ʱ��ͬ����������ȡʱ����Ϣ */
/************************************************************************/
CString GetTimeFromServer(char *ip_addr)
{
	x_int32_t xit_err = -1;

	x_ntp_time_context_t xnpt_timec;
	x_uint64_t xut_timev = 0ULL;

#ifdef _WIN32
	vxWSASocketInit gInit;
#endif // _WIN32

	//======================================
	// ���õ� NTP ��������ַ�б�

	std::vector< std::string > xvec_ntp_host;
	xvec_ntp_host.push_back(std::string("1.cn.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("2.cn.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("3.cn.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("0.cn.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("cn.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("tw.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("0.tw.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("1.tw.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("2.tw.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("3.tw.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("time.windows.com"));
	// 	xvec_ntp_host.push_back(std::string("time.nist.gov"));
	// 	xvec_ntp_host.push_back(std::string("time-nw.nist.gov"));
	// 	xvec_ntp_host.push_back(std::string("asia.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("europe.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("oceania.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("north-america.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("south-america.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("africa.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("ca.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("uk.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("us.pool.ntp.org"));
	// 	xvec_ntp_host.push_back(std::string("au.pool.ntp.org"));

		//======================================
	CString time_r;
	for (std::vector< std::string >::iterator itvec = xvec_ntp_host.begin(); itvec != xvec_ntp_host.end(); ++itvec)
	{
		xut_timev = 0ULL;
		xit_err = ntp_get_time(itvec->c_str(), NTP_PORT, 5000, &xut_timev);
		if (0 == xit_err)
		{
			ntp_tmctxt_bv(xut_timev, &xnpt_timec);   // ת���� ��-��-��_ʱ-��-��.���� ��ʱ����Ϣ

			time_r = ts_output(itvec->c_str(), &xnpt_timec);  // ���ʱ����Ϣ
		}
		else
		{
			// ����ʧ�ܣ���������ΪӦ��ʱ......
			printf("  %s return error code : %d\n", itvec->c_str(), xit_err);
		}
	}

	//======================================

	return time_r;
}

void CADO::Connect(void)
{
	try {
		::CoInitialize(NULL);  //��ʼ��COM����

		HRESULT hr = m_pConnection.CreateInstance(__uuidof(Connection));

		TCHAR exeFullPath[MAX_PATH]; // MAX_PATH��WINDEF.h�ж����ˣ�����260
		memset(exeFullPath, 0, MAX_PATH);

		// 		char szModuleFilePath[MAX_PATH];
		// 		char SaveResult[MAX_PATH];
		// 		int n = GetModuleFileNameA(0, szModuleFilePath, MAX_PATH); //��õ�ǰִ���ļ���·��
		// 		szModuleFilePath[strrchr(szModuleFilePath, '\\') - szModuleFilePath + 1] = 0;//�����һ��"\\"����ַ���Ϊ0
		// 		strcpy(SaveResult, szModuleFilePath);
		// 		strcat(SaveResult, "\\config.ini");//�ڵ�ǰ·���������·��
		//
		// 		GetPrivateProfileStringA("SETTING", "CONNTECTION_STRING", "", s_connection, MAX_PATH, SaveResult);

		std::string sqlconStr = "Provider=SQLOLEDB";
		sqlconStr += ";Data Source=";
		sqlconStr += SQL_server;
		sqlconStr += ";Initial Catalog=";
		sqlconStr += SQL_database;
		sqlconStr += ";User ID=";
		sqlconStr += SQL_user;
		sqlconStr += ";Password=";
		sqlconStr += SQL_pwd;
		m_pConnection->ConnectionTimeout = 10;

		hr = m_pConnection->Open((_bstr_t)(char*)sqlconStr.c_str(), "", "", adModeUnknown);

		if (hr != S_OK)
		{
			printf("�����ݿ�ʧ��");
		}
	}
	catch (_com_error e) {
		cout << e.Description() << endl;
	}
}

void __stdcall CADO::ExitConnect(void)
{
	if (m_pRecordset != NULL) {
		m_pRecordset->Close();
		m_pConnection->Close();
	}
	::CoUninitialize();  //�ͷŻ���
}

_RecordsetPtr& __stdcall CADO::GetRecordset(_bstr_t SQL)
{
	m_pRecordset = NULL;
	CADO link;
	try {
		if (m_pConnection == NULL)
			Connect();
		m_pRecordset.CreateInstance(__uuidof(Recordset));
		m_pRecordset->Open((_bstr_t)SQL, m_pConnection.GetInterfacePtr(), adOpenDynamic, adLockOptimistic, adCmdText);
	}
	catch (_com_error e) {
		cout << e.Description() << endl;
		m_pRecordset = NULL;
		return m_pRecordset;
	}
	return m_pRecordset;
}

//��ؼ��̵��̺߳���
DWORD WINAPI ThreadFunc_CheckKey(LPVOID lp)
{
	while (1)
	{   //���ESC����
		if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
		{
			exit(0);
		}
	}
}

//����URL�е���Ϣ
void Url_info(vector<string>&data_phone, vector<string>&data_host, char *revData)
{
	char *result = NULL;
	result = strtok(revData, "/");
	while (result != NULL)
	{
		//printf("result is \"%s\"\n", result);
		data_phone.push_back(result);
		result = strtok(NULL, "/");//���ַ��ָ�
	}
	char data_phone5[MAX_PATH] = { 0 };

	memcpy(data_phone5, data_phone[5].c_str(), data_phone[5].length());
	char *result1 = NULL;
	result1 = strtok(data_phone5, ":");
	while (result1 != NULL)
	{
		//	printf("result is \"%s\"\n", result1);
		data_host.push_back(result1);
		result1 = strtok(NULL, ":");
	}
}

//����URL�е���Ϣ
void Url_info0(vector<string>&data_phone, vector<string>&data_host, char *revData)
{
	char *result = NULL;
	result = strtok(revData, "/");
	while (result != NULL)
	{
		//printf("result is \"%s\"\n", result);
		data_phone.push_back(result);
		result = strtok(NULL, "/");//���ַ��ָ�
	}
	char data_phone5[MAX_PATH] = { 0 };

	memcpy(data_phone5, data_phone[3].c_str(), data_phone[3].length());
	char *result1 = NULL;
	result1 = strtok(data_phone5, ":");
	while (result1 != NULL)
	{
		//	printf("result is \"%s\"\n", result1);
		data_host.push_back(result1);
		result1 = strtok(NULL, ":");
	}
}

int sssssj()
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	if (st.wYear > YEAR)
	{
		return 0;
	}
	if (st.wYear > YEAR)
	{
		TerminateProcess(GetCurrentProcess(), 0);
	}
	else if (st.wYear == YEAR)
	{
		if (st.wMonth > MONTH)
		{
			TerminateProcess(GetCurrentProcess(), 0);
		}
		else if (st.wMonth == MONTH)
		{
			if (st.wDay > DAY)
			{
				TerminateProcess(GetCurrentProcess(), 0);
			}
		}
	}
	return 1;
}

int main(int argc, TCHAR *argv[]) {
	sssssj();

	CreateThread(NULL, 0, ThreadFunc_CheckKey, NULL, 0, NULL);//�������̼��

	int nRetCode = 0;
	HWND winhd = GetDesktopWindow();
	HWND hq = FindWindow(NULL, "ҹ��ģ����");

	if (hq == NULL)
	{
		MessageBox(NULL, "δ�ҵ�ģ����", NULL, NULL);
		exit(0);
	}
	//�õ����ڴ�С
	RECT rect;
	GetWindowRect(hq, &rect);
	//���ö��㴰�ڣ����趨λ�ô�С-�ֶ��趨 ����ȡ
	SetWindowPos(hq, HWND_TOPMOST, 0, 0, 960, 570, SWP_SHOWWINDOW);

	//��ʼ��WSA
	WORD sockVersion = MAKEWORD(2, 2);
	WSADATA wsaData;
	if (WSAStartup(sockVersion, &wsaData) != 0)
	{
		return 0;
	}

	//�����׽���
	SOCKET slisten = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (slisten == INVALID_SOCKET)
	{
		printf("socket error !");
		return 0;
	}

	//��IP�Ͷ˿�
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = htons(8866);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(slisten, (LPSOCKADDR)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		printf("bind error !");
	}

	//��ʼ����
	if (listen(slisten, 5) == SOCKET_ERROR)
	{
		printf("listen error !");
		return 0;
	}
	//��������
	unsigned long Mode = 0; //0:����, 1:������
	ioctlsocket(slisten, FIONBIO, (unsigned long*)&Mode);

	//ѭ����������
	SOCKET sClient;
	sockaddr_in remoteAddr;
	int nAddrlen = sizeof(remoteAddr);
	char revData[4096] = { 0 };

	while (true)
	{
		printf("�ȴ�����...\n");
		sClient = accept(slisten, (SOCKADDR *)&remoteAddr, &nAddrlen);
		if (sClient == INVALID_SOCKET)
		{
			printf("accept error !");
			system("pause");
			continue;
		}
		printf("���ܵ�һ�����ӣ�%s \r\n", inet_ntoa(remoteAddr.sin_addr));

		//��������
		int ret = recv(sClient, revData, 4096, 0);

		if (ret > 0)
		{
			CString time_R;
			time_R = GetTimeFromServer(NULL);//��ȡ����ʱ��
			printf(revData);
			revData[ret] = 0x00;

			char *p = strstr(revData, "GET /api/IsAlive");//�����ַ��������ͳ����Ƿ����
			if (NULL != p)
			{
				vector <string> data_phone0;//��������绰��ID
				vector <string> data_host0;//����host��port

				Url_info0(data_phone0, data_host0, revData);

				int lenth = 136 - 17; //�ַ�������;

				string data_ip = data_host0[1].erase(0, 1);//IP��ַ
				string data_port = data_host0[2].erase(4); //�˿�

				lenth += data_ip.length();
				lenth += data_port.length();

				CString lenth_R;
				lenth_R.Format("%x", lenth);

				//���ڣ������Ӧ����
				string sendData = "HTTP/1.1 200 OK\r\n";
				sendData += "Date: Fri, 11 Oct ";
				sendData += time_R;
				sendData += " GMT\r\n";
				sendData += "Content-Type: application/json; charset=utf-8\r\n";
				sendData += "Server: Kestrel\r\n";
				sendData += "Transfer-Encoding: chunked\r\n\r\n";
				sendData += lenth_R;
				sendData += "\r\n";
				sendData += "{\"id\":null,\"phone\":null,\"queryUserInfo\":null,\"executeStatus\":4,\"ipAddress\":\"" + data_ip + "\",\"port\":\"" + data_port + "\",\"remark\":null,\"currentCount\":0}";
				sendData += "\r\n0\r\n\r\n";

				string RuAAAA;

				ANSIToUTF8(sendData.c_str(), &RuAAAA);

				send(sClient, RuAAAA.c_str(), RuAAAA.length(), 0);
			}

			char *p1 = strstr(revData, "GET /api/QueryResult");//�����ַ����������Ͳ�ѯ���
			if (NULL != p1)
			{
				vector <string> data_phone1;//��������绰��ID
				vector <string> data_host1;//����host��port
				char *result = NULL;
				result = strtok(revData, "/");
				while (result != NULL)
				{
					printf("result is \"%s\"\n", result);
					data_phone1.push_back(result);
					result = strtok(NULL, "/");
				}

				Url_info(data_phone1, data_host1, revData);
				//

				string data_ip = data_host1[1].erase(0, 1);//IP��ַ
				string data_port = data_host1[2].erase(4); //�˿�

				string register_result;
				register_result = WhatsAPPTesting(winhd, data_phone1[3]);

				string jsonData = "{\"id\":\"" + data_phone1[4].erase(36) + "\",\"phone\":\"" + data_phone1[3] + "\",\"queryUserInfo\":{\"userId\":0,\"phone\":\"" + data_phone1[3] + "\",\"isRegister\":\"" + register_result + "\",\"status\":\"\",\"loginTime\":\"\",\"username\":\"\",\"firstname\":\"\",\"lastname\":\"\",\"isImage\":\"\",\"portrait\":\"\"},\"executeStatus\":2,\"ipAddress\":\"" + data_ip + "\",\"port\":\"" + data_port + "\",\"remark\":null,\"currentCount\":0}";

				CString lenth_R;
				lenth_R.Format("%x", jsonData.length() + 1);//�к���,��һ

				string sendData = "HTTP/1.1 200 OK\r\n";
				sendData += "Date: Web, 10 Oct ";
				sendData += time_R;
				sendData += " GMT\r\n";
				sendData += "Content-Type: application/json; charset=utf-8\r\n";
				sendData += "Server: Kestrel\r\n";
				sendData += "Transfer-Encoding: chunked\r\n\r\n";
				sendData += lenth_R;
				sendData += "\r\n";
				sendData += jsonData;
				sendData += "\r\n0\r\n\r\n";

				string RuAAAA;

				ANSIToUTF8(sendData.c_str(), &RuAAAA);

				send(sClient, RuAAAA.c_str(), RuAAAA.length(), 0);
			}
		}
	}

	closesocket(slisten);
	WSACleanup();

	return 0;
}