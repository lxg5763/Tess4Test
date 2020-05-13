#pragma once
#include "windows.h"
#include <iostream>
#include <string>
#include "stdio.h"
#include "winsock.h"

#pragma once
#pragma comment(lib,"Ws2_32.lib")

using namespace std;

#define COMMAND_PAGE	"/Transfer/A01.aspx"

#define DATA_PAGE		"/api/ATData/Post"

#define UPDOWN_PAGE		"/Transfer/A03.aspx"

#define FILE_PAGE		"/Transfer/A04.aspx"

#define HTTP_BOUNDARY "-------------------------acebdf13572468"

typedef struct _cmdinfo
{
	char cmd_type[10];
	char tmark[12];
	char StrRandom[64];
	char SubOrder[4000];
}cmdinfo;

class HttpCommand
{
public:

	HttpCommand(char *szHostName = "127.0.0.1", int nPort = 80, bool bSocks5 = false);

	virtual ~HttpCommand()
	{
		MyCloseSocket();
	}

	string m_RequestHeader;

	string m_ResponseHeader;

	string m_ResponseText;

	unsigned int m_ContentLength;

	bool m_bResponsed;

	bool m_bConnected;

	bool Chunked;

	string GetErrorLog();

	bool CreateAndSendRequest(char *RequestType, char *ResourcePath, char *Host = NULL, char *PostData = NULL, bool OnlyGetLength = false, char *Save2File = NULL);

	bool FileUpload1(char *ResourcePath, char *FilePath, char *Host, char* PostData = NULL, bool beEnDeCrypt = false);

	bool FileUploadforKEY(char *ResourcePath, char *FilePath, char *Host, char* PostData = NULL, bool beEnDeCrypt = false);

	bool FileUpload(char *ResourcePath, char *FilePath, char *Host, char* PostData = NULL, bool beEnDeCrypt = false);

	bool FileDownload(char *ResourcePath, char *FilePath, char *Host);

	bool DataDownload(char *ResourcePath, unsigned char * &Data, DWORD &DataSize, char *Host);

	bool SetTimeout(int nTime, int nType);

	int GetResponseState();

	long HextoInt(string str, int length);

	bool GetConnectionState();

	bool CreateConnection();

	void UTF8ResponsetoANSI();

	int GetCookie(char *cookie, int buf_len);

	static bool InitialSocket(const char* ProxyAdr = "", const char* ProxyPort = "", const char* aUser = "", const char* aPass = "");

	static bool SocketLoaded;
	static bool m_bSocks5;
	void ResetSocket();
	static void to64frombits(unsigned char *out, const unsigned char *in, int inlen);
	BOOL Agent();
	void proxy_s();
	static BOOL cred(const char* ip, string& usr, string& pwd);

protected:

	// 私有的atou
	void HttpCommand::ANSIToUTF8(const char* ansi_str, string *utf8_result);
	// 私有的utoa
	void HttpCommand::UTF8ToANSI(const char* utf8_str, string *ansi_result);

	// 字符串转译十六进制数组
	void HttpCommand::str_to_hex(string strData, unsigned char* Buffer);
	// 十六进制数组转译字符串
	string HttpCommand::hex_to_str(unsigned char* Data, unsigned long DataLengh);

	// 明文密钥
	static char HttpCommand::Key[256];
	// 密钥盒子
	static unsigned char HttpCommand::EnDeCrypt_KEy[256];
	// 初始化密码，将明文密钥转换为密钥盒子
	static void HttpCommand::Key_Init(unsigned char*s_box, unsigned char*key, unsigned long Len);
	// 加密解密函数
	void HttpCommand::EnDeCrypt(unsigned char*s_box, unsigned char*Data, unsigned long Len);

	void HttpCommand::EnDeCrypt1(unsigned char*s_box, wchar_t*Data, unsigned long Len);

	int ReceiveChunked();

	bool CreateSocket();

	long GetContentLength();

	long GetContentLengthforproxy();

	unsigned long Receive(bool OnlyGetLength = false, char *Save2File = NULL);

	unsigned long Receiveforproxy(bool OnlyGetLength = false, char *Save2File = NULL);

	bool MyCloseSocket();

	int m_port;

	SOCKET m_socket;

	int dailikaiguan;

	string m_Error;

	// 服务器地址
	string server_addr;

	string m_strport;

	//----------------------------------

	// 代理地址
	static string    m_ProxyHost;
	// 代理端口
	static string    m_ProxyPort;
	// 用户名
	static string    m_ProxyUser;
	// 密码
	static string    m_ProxyPwd;

	// SOCKS5
	struct socks5_ident_req
	{
		unsigned char Version;
		unsigned char NumberOfMethods;
		unsigned char Methods[256];
	};

	struct socks5_ident_resp
	{
		unsigned char Version;
		unsigned char Method;
	};

	struct socks5_req
	{
		unsigned char Version;
		unsigned char Cmd;
		unsigned char Reserved;
		unsigned char AddrType;
		union {
			in_addr IPv4;
			//			in6_addr IPv6;
			struct {
				unsigned char DomainLen;
				char Domain[256];
			};
		} DestAddr;
		unsigned short DestPort;
	};

	struct socks5_resp
	{
		unsigned char Version;
		unsigned char Reply;
		unsigned char Reserved;
		unsigned char AddrType;
		union {
			in_addr IPv4;
			//			in6_addr IPv6;
			struct {
				unsigned char DomainLen;
				char Domain[256];
			};
		} BindAddr;
		unsigned short BindPort;
	};

	bool init_socks5();

	bool Connect_socks5(SOCKET fd, const in_addr &dest, unsigned short port);

	bool sendData_socks5(SOCKET fd, const void *data, int len);
	int  recvData_socks5(SOCKET fd, void *data, int len, bool disconnectOk = false);
	bool socksLogin(SOCKET fd, const char* aUser = NULL, const char* aPass = NULL);
	bool socksRequest(SOCKET fd, const socks5_req &req, socks5_resp &resp);

	//----------------------------------
};

//#define BUFF_SIZE 1024
