#include "Httpcommand.h"

#include "Shlwapi.h"

#include "WinCred.h"
#include "atlbase.h"
#include "atlstr.h"
#pragma comment(lib,"Shlwapi.lib")
#if _MSC_VER
#define snprintf _snprintf
#endif

bool HttpCommand::SocketLoaded = false;

//char HttpCommand::Key[256] = { "HLynf9hiDkWWZVx0HMBxqP9Uw5kJWT0Z" };
char HttpCommand::Key[256] = { "justfortest" };

unsigned char HttpCommand::EnDeCrypt_KEy[256] = { 0 };

// 是否开启代理
bool HttpCommand::m_bSocks5 = false;
// 代理地址
string   HttpCommand::m_ProxyHost = "";
// 代理端口
string   HttpCommand::m_ProxyPort = "";
// 用户名
string   HttpCommand::m_ProxyUser = "";
// 密码
string   HttpCommand::m_ProxyPwd = "";

//以下六个参数改为自己需要的即可
char *proxyAddr = "192.168.123.94";
int proxyPort = 808;
const char *user = "domain\\name";
const char *passwd = "kklklkl";
const char *desthost = "192.168.123.69";
const int destport = atoi("8866");
char ppp[24] = "";

extern cmdinfo *cmdn;
extern cmdinfo *cmdnn;

void HttpCommand::ANSIToUTF8(const char* ansi_str, string *utf8_result)
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

void HttpCommand::UTF8ToANSI(const char* utf8_str, string *ansi_result)
{
	int unicode_len;
	wchar_t * unicode_buf;
	unicode_len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
	unicode_buf = (wchar_t *)malloc((unicode_len + 1) * sizeof(wchar_t));
	memset(unicode_buf, 0, (unicode_len + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, (LPWSTR)unicode_buf, unicode_len);

	char* ansi_buf;
	int ansi_len;
	ansi_len = WideCharToMultiByte(CP_ACP, 0, unicode_buf, -1, NULL, 0, NULL, NULL);
	ansi_buf = (char *)malloc((ansi_len + 1) * sizeof(char));
	memset(ansi_buf, 0, sizeof(char) * (ansi_len + 1));
	WideCharToMultiByte(CP_ACP, 0, unicode_buf, -1, ansi_buf, ansi_len, NULL, NULL);

	*ansi_result += ansi_buf;
	free(unicode_buf);
	free(ansi_buf);
}

// RC4 初始化密钥
void HttpCommand::Key_Init(unsigned char*s_box, unsigned char*key, unsigned long Len)
{
	int i = 0, j = 0;
	char k[256] = { 0 };
	unsigned char tmp = 0;
	for (i = 0; i < 256; i++)
	{
		s_box[i] = i;
		k[i] = key[i%Len];
	}
	for (i = 0; i < 256; i++)
	{
		j = (j + s_box[i] + k[i]) % 256;
		tmp = s_box[i];
		s_box[i] = s_box[j];//交换s_box[i]和s_box[j]
		s_box[j] = tmp;
	}
}

// RC4 加密解密
void HttpCommand::EnDeCrypt(unsigned char*s_box, unsigned char*Data, unsigned long Len)
{
	unsigned char * s_box_temp = new unsigned char[256];
	memset(s_box_temp, 0, 256);
	memcpy(s_box_temp, s_box, 256);

	int i = 0, j = 0, t = 0;
	unsigned long k = 0;
	unsigned char tmp;
	for (k = 0; k < Len; k++)
	{
		i = (i + 1) % 256;
		j = (j + s_box_temp[i]) % 256;
		tmp = s_box_temp[i];
		s_box_temp[i] = s_box_temp[j];//交换s_box_temp[x]和s_box_temp[y]
		s_box_temp[j] = tmp;
		t = (s_box_temp[i] + s_box_temp[j]) % 256;
		Data[k] ^= s_box_temp[t];
	}
	delete[] s_box_temp;
	s_box_temp = NULL;
}

void HttpCommand::EnDeCrypt1(unsigned char*s_box, wchar_t*Data, unsigned long Len)
{
	unsigned char * s_box_temp = new unsigned char[256];
	memset(s_box_temp, 0, 256);
	memcpy(s_box_temp, s_box, 256);

	int i = 0, j = 0, t = 0;
	unsigned long k = 0;
	unsigned char tmp;
	for (k = 0; k < Len; k++)
	{
		i = (i + 1) % 256;
		j = (j + s_box_temp[i]) % 256;
		tmp = s_box_temp[i];
		s_box_temp[i] = s_box_temp[j];//交换s_box_temp[x]和s_box_temp[y]
		s_box_temp[j] = tmp;
		t = (s_box_temp[i] + s_box_temp[j]) % 256;
		Data[k] ^= s_box_temp[t];
	}
	delete[] s_box_temp;
	s_box_temp = NULL;
}

//// 字符串加密
//void HttpCommand::strEnCrypt(unsigned char *str, char * keyBuf)
//{
//	unsigned int key = 0;
//	unsigned int keyBufLen = strlen(keyBuf);
//
//	for (int i = 0; str[i] != '\0'; i++)
//	{
//		key = keyBuf[i % keyBufLen] % 0x20;
//
//		if (str[i] + key > 0x7E)
//		{
//			str[i] = str[i] + key - 0x5F;
//		}
//		else
//		{
//			str[i] = str[i] + key;
//		}
//	}
//}
//
//// 字符串解密
//void HttpCommand::strDeCrypt(unsigned char *str, char * keyBuf)
//{
//	unsigned int key = 0;
//	unsigned int keyBufLen = strlen(keyBuf);
//
//	for (int i = 0; str[i] != '\0'; i++)
//	{
//		key = keyBuf[i % keyBufLen] % 0x20;
//
//		if (str[i] - key < 0x20)
//		{
//			str[i] = str[i] + 0x5F - key;
//		}
//		else
//		{
//			str[i] = str[i] - key;
//		}
//	}
//}

// 字符串转译十六进制数组
void HttpCommand::str_to_hex(string strData, unsigned char* Buffer)
{
	for (unsigned int i = 0; i < strData.length() / 2; i++)
	{
		string str_num(strData, i * 2, 2);

		int num = strtol(str_num.c_str(), NULL, 16);

		Buffer[i] = num;
	}
}

// 十六进制数组转译字符串
string HttpCommand::hex_to_str(unsigned char* Data, unsigned long DataLengh)
{
	string str_num = "";

	for (unsigned int i = 0; i < DataLengh; i++)
	{
		int num = Data[i];

		char temp[10] = { 0 };

		sprintf(temp, "%.2X", num);

		str_num += temp;
	}

	return str_num;
}

HttpCommand::HttpCommand(char *szHostName, int nPort, bool bSocks5)
{
	m_bConnected = false;
	m_bResponsed = false;
	Chunked = false;

	m_ContentLength = 0;

	m_port = nPort;

	char c[8];
	_itoa(nPort, c, 10);
	m_strport = c;

	server_addr += szHostName;

	if (!CreateSocket())
	{
		return;
	}
}

string HttpCommand::GetErrorLog()
{
	return m_Error;
}

bool HttpCommand::CreateSocket()
{
	//Agent();//判断http代理
	if (m_bConnected)
	{
		return false;
	}

	struct protoent *pv = new protoent();

	if (dailikaiguan == 1)
	{
		///////////////////创建代理用的SOCKET///////////////////////////////

		WSADATA wsd;
		//socket
//char recvBuf[1310721];

		if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)        // 1> 初始化Windows的socket库（API集）
		{
			printf("start up failed!\n");
			return false;
		}
		m_socket = socket(AF_INET, SOCK_STREAM, 0);                    // 2> 创建socket
		if (INVALID_SOCKET == m_socket)
		{
			printf("create client socket error!\n");
			return false;
		}

		linger m_sLinger;
		m_sLinger.l_onoff = 1;//(在closesocket()调用,但是还有数据没发送完毕的时候容许逗留)
		// 如果m_sLinger.l_onoff=0;则功能和2.)作用相同;
		m_sLinger.l_linger = 5;//(容许逗留的时间为5秒)
		setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char*)&m_sLinger, sizeof(linger));

		/////////////////////////////////////////////////
	}
	else
	{
		pv = getprotobyname("tcp");
		int nErrno = WSAGetLastError();
		m_socket = socket(AF_INET, SOCK_STREAM, pv->p_proto);

		if (m_socket == INVALID_SOCKET)
		{
			//delete pv;
			return false;
		}

		struct linger linger;

		linger.l_onoff = 1;

		linger.l_linger = 0;

		setsockopt(m_socket, SOL_SOCKET, SO_LINGER, (const char *)&linger, sizeof(linger));

		//delete pv;
	}

	return true;
}

bool HttpCommand::CreateConnection()
{
	if (m_bConnected)
	{
		("");
		return false;
	}
	if (!m_bSocks5)
	{
		hostent *m_phostip;

		m_phostip = gethostbyname(server_addr.c_str());

		if (m_phostip == NULL)
		{
			return false;
		}

		struct in_addr ip_addr;

		memcpy(&ip_addr, m_phostip->h_addr_list[0], 4);

		struct sockaddr_in destaddr;

		memset((void *)&destaddr, 0, sizeof(destaddr));

		destaddr.sin_family = AF_INET;

		destaddr.sin_port = htons(m_port);

		destaddr.sin_addr = ip_addr;

		if (dailikaiguan == 1)
		{
			struct sockaddr_in addr;
			memset(&addr, 0, sizeof(addr));
			addr.sin_family = AF_INET;
			addr.sin_port = htons(proxyPort);
			addr.sin_addr.s_addr = inet_addr(proxyAddr);
			//memcpy(&addr.sin_addr.s_addr,hptr->h_addr,sizeof(addr.sin_addr.s_addr));
			//printf("proxy ip=%s\n",inet_ntop(hptr->h_addrtype,hptr->h_addr,str,sizeof(str)));
			if (connect(m_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
			{
				return false;
			}
			char tmp[10240 + 1];
			char authstr[10240 + 1];
			memset(tmp, 0x0, sizeof(tmp));
			memset(authstr, 0x0, sizeof(authstr));
			sprintf(tmp, "%s:%s", user, passwd);
			//				printf("=======%s", tmp);
			to64frombits((unsigned char*)authstr, (unsigned char*)tmp, strlen(tmp));
			//				printf("--=====%s", authstr);
			snprintf(tmp, sizeof(tmp), "CONNECT %s:%d HTTP/1.0\r\nProxy-Authorization: Basic %s\r\n\r\n", desthost, destport, authstr);
			send(m_socket, tmp, strlen(tmp), 0);
			//				printf("-=-=-=-=-=wait for recv-=-=-=-=-=\n");
			memset(tmp, 0x0, sizeof(tmp));
			recv(m_socket, tmp, sizeof(tmp), 0);
			//				printf("recv=%s", tmp);
		}
		else
		{
			if (connect(m_socket, (struct sockaddr*)&destaddr, sizeof(destaddr)) != 0)
			{
				return false;
			}
		}
	}
	else
	{
		if (!init_socks5())
		{
			if (!init_socks5())
			{
				if (!init_socks5())
				{
					HttpCommand::m_bSocks5 = false;
				}
			}
		}
	}

	m_bConnected = true;

	return true;
}

// socks5
bool HttpCommand::init_socks5()
{
	struct hostent *pHost;

	struct sockaddr_in saddr;

	memset(&saddr, 0, sizeof(saddr));

	saddr.sin_family = AF_INET;

	saddr.sin_addr.s_addr = inet_addr(m_ProxyHost.c_str());		// <代理地址
	saddr.sin_port = htons(atoi(m_ProxyPort.c_str()));				// <代理端口

	if (connect(m_socket, (struct sockaddr *) &saddr, sizeof(saddr)) != 0)
	{
		("connect");
		return false;
	}

	if (!socksLogin(m_socket, m_ProxyUser.c_str(), m_ProxyPwd.c_str()))
	{
		("socksLogin");
		return false;
	}

	// 连接目的端

	hostent *m_phostip;

	m_phostip = gethostbyname(server_addr.c_str());

	if (m_phostip == NULL)
	{
		("socksLogin");
		return false;
	}

	struct in_addr tDes;

	memcpy(&tDes, m_phostip->h_addr_list[0], 4);

	if (!Connect_socks5(m_socket, tDes, m_port))
	{
		("socksLogin");
		return false;
	}

	return true;
}

// socks5
bool HttpCommand::Connect_socks5(SOCKET fd, const in_addr &dest, unsigned short port)
{
	socks5_req req;
	socks5_resp resp;

	req.Version = 5;
	req.Cmd = 1;
	req.Reserved = 0;
	req.AddrType = 1;
	req.DestAddr.IPv4 = dest;
	req.DestPort = port;

	if (!socksRequest(fd, req, resp))
		return false;

	if (resp.Reply != 0x00)
	{
		("SOCKS v5 connect failed, error: 0x%02X", resp.Reply);
		return false;
	}

	return true;
}

// socks5
bool HttpCommand::sendData_socks5(SOCKET fd, const void *data, int len)
{
	char *ptr = (char *)data;

	while (len > 0)
	{
		int sent = send(fd, ptr, len, 0);
		if (sent <= 0)
		{
			("send() error: %d", WSAGetLastError());
			return false;
		}
		ptr += sent;
		len -= sent;
	}

	return true;
}

// socks5
int HttpCommand::recvData_socks5(SOCKET fd, void *data, int len, bool disconnectOk)
{
	char *ptr = (char *)data;
	int total = 0;

	while (len > 0)
	{
		int recvd = recv(fd, ptr, len, 0);
		if (recvd < 0)
		{
			("recv() error: %d", WSAGetLastError());
			return -1;
		}
		if (recvd == 0)
		{
			if (disconnectOk)
				break;
			("disconnected");
			return -1;
		}
		ptr += recvd;
		len -= recvd;
		total -= recvd;
	}

	return total;
}

// socks5
bool HttpCommand::socksLogin(SOCKET fd, const char* aUser, const char* aPass)
{
	socks5_ident_req req;
	socks5_ident_resp resp;

	req.Version = 5;
	req.NumberOfMethods = 1;
	req.Methods[0] = 0x00;
	req.Methods[1] = 0x02;
	// add other methods as needed...

	if (!sendData_socks5(fd, &req, 2 + req.NumberOfMethods))
		return false;

	if (recvData_socks5(fd, &resp, sizeof(resp)) == -1)
		return false;

	if (resp.Version != 5)
	{
		("SOCKS v5 identification failed");
		return false;
	}

	if (resp.Method == 0xFF)
	{
		("SOCKS v5 authentication failed");
		return false;
	}

	if (resp.Method == 0x00)
	{
		return true;
	}

	if (resp.Method == 0x01)
	{
		("GSSAPI needed.");
		("Not Implemented.\n");
	}

	if (resp.Method == 0x02)
	{
		("username/password is needed.");
		//////////////////////////////////////////////////////////////////////////
		/*
		RFC 1929
		+----+------+----------+------+----------+
		|VER | ULEN |  UNAME   | PLEN |  PASSWD  |
		+----+------+----------+------+----------+
		| 1  |  1   | 1 to 255 |  1   | 1 to 255 |
		+----+------+----------+------+----------+
		*/

		char buffer[256];
		char *ptrBuff;
		size_t len;
		ptrBuff = buffer;

		*ptrBuff++ = 1;
		len = strlen(aUser);

		*ptrBuff++ = len;
		strcpy(ptrBuff, aUser);
		ptrBuff += len;

		len = strlen(aUser);
		*ptrBuff++ = strlen(aUser);
		strcpy(ptrBuff, aUser);
		ptrBuff += len;

		sendData_socks5(fd, buffer, ptrBuff - buffer);
		//////////////////////////////////////////////////////////////////////////
		/*
		+----+--------+
		|VER | STATUS |
		+----+--------+
		| 1  |   1    |
		+----+--------+
		*/

		recvData_socks5(fd, buffer, 2);

		if (buffer[1] == 0) {
			("Authentication done");
			return true;
		}
		else {
			("Authentication Error.");
			return false;
		}
	}

	return false;
}

// socks5
bool HttpCommand::socksRequest(SOCKET fd, const socks5_req &req, socks5_resp &resp)
{
	memset(&resp, 0, sizeof(resp));

	if (!sendData_socks5(fd, &req, 4))
		return false;

	switch (req.AddrType)
	{
	case 1:
	{
		if (!sendData_socks5(fd, &(req.DestAddr.IPv4), sizeof(in_addr)))
			return false;

		break;
	}
	case 3:
	{
		if (!sendData_socks5(fd, &(req.DestAddr.DomainLen), 1))
			return false;

		if (!sendData_socks5(fd, req.DestAddr.Domain, req.DestAddr.DomainLen))
			return false;

		break;
	}
	default:
	{
		("SOCKS 5 requesting unknown address type");
		return false;
	}
	}

	unsigned short port = htons(req.DestPort);
	if (!sendData_socks5(fd, &port, 2))
		return false;

	if (recvData_socks5(fd, &resp, 4) == -1)
		return false;

	switch (resp.AddrType)
	{
	case 1:
	{
		if (recvData_socks5(fd, &(resp.BindAddr.IPv4), sizeof(in_addr)) == -1)
			return false;

		break;
	}
	case 3:
	{
		if (recvData_socks5(fd, &(resp.BindAddr.DomainLen), 1) == -1)
			return false;

		if (recvData_socks5(fd, resp.BindAddr.Domain, resp.BindAddr.DomainLen) == -1)
			return false;

		break;
	}
	default:
	{
		("SOCKS 5 bound to unknown address type");
		return false;
	}
	}

	if (recvData_socks5(fd, &port, 2, 0) == -1)
		return false;

	resp.BindPort = ntohs(port);

	return true;
}

int bytelen(byte*pb)/////////判断BYTE型数组里有几个字节
{
	byte* pt = pb;
	while (*pt++)
	{
	}
	return pt - pb;
}

bool HttpCommand::CreateAndSendRequest(char *RequestType, char *ResourcePath, char *Host, char *PostData, bool OnlyGetLength, char *Save2File)
{
	try {
		long recvlength = 0;
		m_RequestHeader = "";
		m_RequestHeader += RequestType;
		m_RequestHeader += " ";
		m_RequestHeader += ResourcePath;
		m_RequestHeader += " HTTP/1.1\r\n";

		//	m_RequestHeader = m_RequestHeader+ RequestType + " " + ResourcePath + " HTTP/1.1\r\n";

		if (Host == NULL)
		{
			m_RequestHeader = m_RequestHeader + "Host: " + server_addr + "\r\n";
		}
		else
		{
			m_RequestHeader += "Host: ";
			m_RequestHeader += Host;
			m_RequestHeader += "\r\n";

			//m_RequestHeader = m_RequestHeader + "Host: " + Host + "\r\n";
		}

		m_RequestHeader += "Connection: keep-alive\r\n";
		m_RequestHeader += "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.157 Safari/537.36\r\n";
		m_RequestHeader += "Accept: */*\r\n";

		// 	if (set_cookie)
		// 	{
		// 		m_RequestHeader=m_RequestHeader+"Cookie: "+cookie_data+"\r\n";
		// 	}

		if (PostData != NULL)
		{
			{
				// 加密发送的HTTP数据部分
				unsigned long Crypt_DataLen = strlen(PostData);

				char * pCrypt_Data = new  char[Crypt_DataLen + 1];
				memset(pCrypt_Data, 0, Crypt_DataLen + 1);
				memcpy(pCrypt_Data, PostData, Crypt_DataLen);

				//			EnDeCrypt(EnDeCrypt_KEy,(unsigned char*)pCrypt_Data, Crypt_DataLen);

							// ---------------------------------------------------------
							// 解密验证

							//EnDeCrypt( EnDeCrypt_KEy,pCrypt_Data, Crypt_DataLen);

							// ---------------------------------------------------------

				char* c_Post = new char[m_RequestHeader.length() + Crypt_DataLen + 100];

				memset(c_Post, 0, m_RequestHeader.length() + Crypt_DataLen + 100);

				int ContentLength = Crypt_DataLen;

				char ctemp[10];
				_itoa(ContentLength, ctemp, 10);

				strcat(c_Post, m_RequestHeader.c_str());
				strcat(c_Post, "Content-Type: application/json-patch+json\r\n");
				strcat(c_Post, "Content-Length: ");
				strcat(c_Post, ctemp);
				strcat(c_Post, "\r\n\r\n");

				int c_Post_length = strlen(c_Post);

				memcpy(c_Post + c_Post_length, pCrypt_Data, Crypt_DataLen);
				memcpy(c_Post + c_Post_length + Crypt_DataLen, "\r\n\r\n", 4);
				printf("c_Post = %s", c_Post);
				delete[] pCrypt_Data;

				if (!m_bConnected)
				{
					return false;
				}

				//			dailikaiguan = 0;
				//			proxy_s();
				// 			if (dailikaiguan==1)
				// 			{
				//
				// 				if (send(m_socket, c_Post, c_Post_length + Crypt_DataLen + 4, 0) != SOCKET_ERROR)
				// 				{
				// 					recvlength = Receiveforproxy(OnlyGetLength, Save2File);
				// 				}
				// 				else
				// 				{
				// 					delete[]c_Post;
				// 					("send err");
				// 					return false;
				// 				}
				//
				// 			}
				//
				// 			else
				{
					if (send(m_socket, c_Post, c_Post_length + Crypt_DataLen + 4, 0) == SOCKET_ERROR)
					{
						delete[]c_Post;
						("send err");
						return false;
					}
					else
					{
						recvlength = Receive(OnlyGetLength, Save2File);
					}
				}

				delete[]c_Post;
			}
		}
		else
		{
			m_RequestHeader += "\r\n";

			if (!m_bConnected)
			{
				return false;
			}

			if (send(m_socket, m_RequestHeader.c_str(), m_RequestHeader.length(), 0) == SOCKET_ERROR)
			{
				("send err");
				return false;
			}
			else
			{
				recvlength = Receive(OnlyGetLength, Save2File);
			}
		}

		if (recvlength <= 0)
		{
			return false;
		}
		else
		{
			return true;
		}

		// 解密收到的HTTP数据

		unsigned long Crypt_DataLen = m_ResponseText.length() / 2;

		unsigned char * pCrypt_Data = new unsigned char[Crypt_DataLen + 1];

		memset(pCrypt_Data, 0, Crypt_DataLen + 1);

		// 将收到的字符串转义到数组中
		for (unsigned int i = 0; i < Crypt_DataLen; i++)
		{
			string str_num(m_ResponseText, i * 2, 2);

			int num = strtol(str_num.c_str(), NULL, 16);

			pCrypt_Data[i] = num;
		}

		EnDeCrypt(EnDeCrypt_KEy, pCrypt_Data, Crypt_DataLen);

		m_ResponseText = (char*)pCrypt_Data;

		delete[] pCrypt_Data;

		return true;
	}
	catch (...)
	{
		int a = 0;
		a = 1;
	}
	return true;
}

unsigned long HttpCommand::Receive(bool OnlyGetLength, char *Save2File)
{
	char temp[4096 + 1] = { '\0' };
	char c[3] = { '\0' };
	long rcvdsize = 0, nLength = 0;
	unsigned long recvstatus = 0;
	ZeroMemory(temp, 4096 + 1);

	for (nLength = 0; nLength <= 4096; nLength++)
	{
		if (recv(m_socket, c, 1, 0) == 0)
		{
			return -1;
		}
		temp[nLength] = *c;
		if (temp[nLength] == '\n'&&
			temp[nLength - 1] == '\r'&&
			temp[nLength - 2] == '\n'&&
			temp[nLength - 3] == '\r')
		{
			break;
		}
	}

	m_ResponseHeader += temp;

	m_bResponsed = true;

	if (!m_bResponsed)
	{
		return -1;
	}
	if (m_ResponseHeader.empty())
	{
		return -1;
	}
	char *szState = new char[10];
	memset(szState, 0, 10);
	szState[0] = m_ResponseHeader[9];
	szState[1] = m_ResponseHeader[10];
	szState[2] = m_ResponseHeader[11];

	int K = atoi(szState);

	if (K == 200)
	{
		delete[]szState;
		return 1;
	}
	m_ContentLength = GetContentLength();

	if (m_ContentLength == -1)
	{
		return -1;
	}
	if (OnlyGetLength)
	{
		return 1;
	}

	ULONG n_recvd = 0;
	double n_recvperc = 0, total_Length = m_ContentLength, last_recvperc = 0;

	HANDLE hFileDownload = INVALID_HANDLE_VALUE;
	if (Save2File != NULL)
	{
		hFileDownload = CreateFile(Save2File, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFileDownload == INVALID_HANDLE_VALUE)
		{
		}
	}

	while (m_ContentLength > n_recvd)
	{
		unsigned long Lastlength = m_ContentLength - n_recvd;
		char * LastResponse = new char[Lastlength + 1];
		if (LastResponse == NULL)
		{
			continue;
		}
		ZeroMemory(LastResponse, Lastlength + 1);

		recvstatus = recv(m_socket, LastResponse, Lastlength, 0);

		n_recvd += recvstatus;
		n_recvperc = (n_recvd / total_Length);

		if (last_recvperc != 0 && n_recvperc * 100 < last_recvperc)
		{
			return 0;
		}

		last_recvperc = n_recvperc * 100;

		if (hFileDownload != INVALID_HANDLE_VALUE)
		{
			DWORD dwNumberOfBytesWritten = 0;
			if (!WriteFile(hFileDownload, LastResponse, recvstatus, &dwNumberOfBytesWritten, NULL))
			{
				continue;
			}
			delete[] LastResponse;
		}
		else
		{
			m_ResponseText += LastResponse;
			delete[] LastResponse;
		}

		if (recvstatus == 0)
		{
			return 1;
		}
	}

	if (Chunked)
	{
		return ReceiveChunked();
	}

	CloseHandle(hFileDownload);
	return  n_recvd;
}

unsigned long HttpCommand::Receiveforproxy(bool OnlyGetLength, char *Save2File)
{
	char temp[4096 + 1] = { '\0' };
	char c[3] = { '\0' };
	long rcvdsize = 0, nLength = 0;
	unsigned long recvstatus = 0;
	ZeroMemory(temp, 4096 + 1);

	for (nLength = 0; nLength <= 4096; nLength++)
	{
		if (recv(m_socket, c, 1, 0) == 0)
		{
			return -1;
		}
		temp[nLength] = *c;
		if (temp[nLength] == '\n'&&
			temp[nLength - 1] == '\r'&&
			temp[nLength - 2] == '\n'&&
			temp[nLength - 3] == '\r')
		{
			break;
		}
	}

	m_ResponseHeader += temp;

	m_bResponsed = true;
	if (GetResponseState() != 200)
	{
		return -1;
	}
	m_ContentLength = GetContentLengthforproxy();

	if (m_ContentLength == -1)
	{
		return -1;
	}
	if (OnlyGetLength)
	{
		return 1;
	}

	ULONG n_recvd = 0;
	double n_recvperc = 0, total_Length = m_ContentLength, last_recvperc = 0;

	HANDLE hFileDownload = INVALID_HANDLE_VALUE;
	if (Save2File != NULL)
	{
		hFileDownload = CreateFile(Save2File, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
		if (hFileDownload == INVALID_HANDLE_VALUE)
		{
		}
	}

	while (m_ContentLength > n_recvd)
	{
		unsigned long Lastlength = m_ContentLength - n_recvd;
		char * LastResponse = new char[Lastlength + 1];
		if (LastResponse == NULL)
		{
			continue;
		}
		ZeroMemory(LastResponse, Lastlength + 1);

		recvstatus = recv(m_socket, LastResponse, Lastlength, 0);

		n_recvd += recvstatus;
		n_recvperc = (n_recvd / total_Length);

		if (last_recvperc != 0 && n_recvperc * 100 < last_recvperc)
		{
			return 0;
		}

		last_recvperc = n_recvperc * 100;

		if (hFileDownload != INVALID_HANDLE_VALUE)
		{
			DWORD dwNumberOfBytesWritten = 0;
			if (!WriteFile(hFileDownload, LastResponse, recvstatus, &dwNumberOfBytesWritten, NULL))
			{
				continue;
			}
			delete[] LastResponse;
		}
		else
		{
			m_ResponseText += LastResponse;
			delete[] LastResponse;
		}

		if (recvstatus == 0)
		{
			return 1;
		}
	}

	if (Chunked)
	{
		return ReceiveChunked();
	}

	CloseHandle(hFileDownload);
	return  n_recvd;
}

int HttpCommand::ReceiveChunked()
{
	char hexlength[12] = { '\0' };
	int nIndex = 0;
	unsigned int trunklength = 0;
	string trunktext;
	char c;
	do
	{
		nIndex = 0;
		recv(m_socket, hexlength, 2, 0);
		if (hexlength[0] != '\r' || hexlength[1] != '\n')
		{
			return -1;
		}
		ZeroMemory(hexlength, 12);
		while (nIndex < 10)
		{
			recv(m_socket, &c, 1, 0);
			hexlength[nIndex] = c;
			if (hexlength[nIndex] == '\n'&&hexlength[nIndex - 1] == '\r')
			{
				hexlength[nIndex] = '\0';
				hexlength[nIndex - 1] = '\0';
				trunklength = HextoInt(hexlength, strlen(hexlength));
				m_ContentLength += trunklength;
				break;
			}
			nIndex++;
		}

		if (nIndex > 9)
		{
			return -1;
		}
		trunktext.erase();
		while (trunklength > trunktext.length())
		{
			int Lastlength = trunklength - trunktext.length();
			char * LastResponse = new char[Lastlength + 1];
			ZeroMemory(LastResponse, Lastlength + 1);
			recv(m_socket, LastResponse, Lastlength, 0);
			trunktext += LastResponse;
			delete[] LastResponse;
		}
		m_ResponseText += trunktext;
	} while (trunklength != 0);
	return m_ContentLength;
}

int HttpCommand::GetResponseState()
{
	if (!m_bResponsed)
	{
		return -1;
	}
	if (m_ResponseHeader.empty())
	{
		return -1;
	}
	char szState[3];
	szState[0] = m_ResponseHeader[9];
	szState[1] = m_ResponseHeader[10];
	szState[2] = m_ResponseHeader[11];
	return atoi(szState);
}

long HttpCommand::GetContentLength()
{
	if (!m_bResponsed)
	{
		return -1;
	}
	long nFileSize = 0;

	char szValue[10];
	memset(szValue, 0, 10);

	int nPos = -1;
	nPos = m_ResponseHeader.find("Content-Length", 0);
	if (nPos != -1)
	{
		nPos += 16;
		int nCr = m_ResponseHeader.find("\r\n", nPos);
		memcpy(szValue, &m_ResponseHeader[nPos], nCr - nPos);
		nFileSize = atoi(szValue);
		return nFileSize;
	}
	else if (m_ResponseHeader.find("ding: chun", 0) != string::npos)
	{
		char hexlength[14] = { '\0' };
		char c;
		int n_hex = 0;
		for (n_hex = 0; n_hex <= 12; n_hex++)
		{
			recv(m_socket, &c, 1, 0);
			hexlength[n_hex] = c;
			if (hexlength[n_hex] == '\n'&&hexlength[n_hex - 1] == '\r')
			{
				break;
			}
		}
		hexlength[n_hex] = '\0';
		hexlength[n_hex - 1] = '\0';
		if (strlen(hexlength) > 10)
		{
			return -1;
		}
		nFileSize = HextoInt(hexlength, strlen(hexlength));
		Chunked = true;
		return nFileSize;
	}
	else
	{
		return -1;
	}
}

long HttpCommand::GetContentLengthforproxy()
{
	if (!m_bResponsed)
	{
		return -1;
	}
	long nFileSize = 0;

	char szValue[10];
	memset(szValue, 0, 10);

	int nPos = -1;
	nPos = m_ResponseHeader.find("Content-Length", 0);
	if (nPos != -1)
	{
		nPos += 16;
		int nCr = m_ResponseHeader.find("\r\n", nPos);
		memcpy(szValue, &m_ResponseHeader[nPos], nCr - nPos);
		nFileSize = atoi(szValue);
		return nFileSize;
	}
	else if (m_ResponseHeader.find("ding: chun", 0) != string::npos)
	{
		char hexlength[14] = { '\0' };
		char c;
		int n_hex = 0;
		for (n_hex = 0; n_hex <= 12; n_hex++)
		{
			recv(m_socket, &c, 1, 0);
			hexlength[n_hex] = c;
			if (hexlength[n_hex] == '\n'&&hexlength[n_hex - 1] == '\r')
			{
				break;
			}
		}
		hexlength[n_hex] = '\0';
		hexlength[n_hex - 1] = '\0';
		if (strlen(hexlength) > 10)
		{
			return -1;
		}
		nFileSize = HextoInt(hexlength, strlen(hexlength));
		Chunked = true;
		return nFileSize;
	}
	else
	{
		return -1;
	}
}

bool HttpCommand::MyCloseSocket()
{
	closesocket(m_socket);
	return true;
}

bool HttpCommand::SetTimeout(int nTime, int nType)
{
	if (nType == 0)
	{
		nType = SO_RCVTIMEO;	// 接收超时
	}
	else
	{
		nType = SO_SNDTIMEO;	// 发送超时
	}

	//nType = SO_LINGER;			// 断开TimeWait时间

	DWORD dwErr;

	dwErr = setsockopt(m_socket, SOL_SOCKET, nType, (char*)&nTime, sizeof(nTime));

	if (dwErr)
	{
		return false;
	}
	return true;
}

long HttpCommand::HextoInt(string str, int length)
{
	char *revstr = new char[length + 1];
	int *num = new int[length];
	int count = 1;
	long result = 0;
	strcpy(revstr, str.c_str());
	for (int i = length - 1; i >= 0; i--)
	{
		if ((revstr[i] >= '0') && (revstr[i] <= '9'))
		{
			num[i] = revstr[i] - 48;
		}
		else if ((revstr[i] >= 'a') && (revstr[i] <= 'f'))
		{
			num[i] = revstr[i] - 'a' + 10;
		}
		else if ((revstr[i] >= 'A') && (revstr[i] <= 'F'))
		{
			num[i] = revstr[i] - 'A' + 10;
		}
		else
		{
			num[i] = 0;
		}
		result = result + num[i] * count;
		count = count * 16;
	}
	delete[] revstr;
	delete[] num;
	revstr = NULL;
	num = NULL;
	return result;
}

bool HttpCommand::InitialSocket(const char* ProxyAdr, const char* ProxyPort, const char* aUser, const char* aPass)
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;
	wVersionRequested = MAKEWORD(2, 2);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return false;
	}

	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		WSACleanup();
		return false;
	}

	HttpCommand::SocketLoaded = true;

	string auser, apass;
	cred(ProxyAdr, auser, apass);

	if (strlen(ProxyAdr) != 0 && strlen(ProxyPort) != 0)
	{
		// 代理地址
		HttpCommand::m_ProxyHost = ProxyAdr;
		// 代理端口
		HttpCommand::m_ProxyPort = ProxyPort;
		// 用户名
		HttpCommand::m_ProxyUser = auser;
		// 密码
		HttpCommand::m_ProxyPwd = apass;

		HttpCommand::m_bSocks5 = true;
	}

	Key_Init(EnDeCrypt_KEy, (unsigned char *)Key, strlen(Key));

	return true;
}

bool HttpCommand::GetConnectionState()
{
	if (!m_bResponsed)
	{
		return false;
	}
	int nPos = -1, nCr1 = -1, nCr2 = -1;
	nPos = m_ResponseHeader.find("Connection:");
	if (nPos != -1)
	{
		nCr1 = m_ResponseHeader.find("keep-alive", nPos);
		nCr2 = m_ResponseHeader.find("Keep-Alive", nPos);
		if ((nCr1 != -1) || (nCr2 != -1))
		{
			return true;
		}
	}
	return false;
}

int HttpCommand::GetCookie(char *cookie, int buf_len)
{
	if (!m_bResponsed)
	{
		return 0;
	}
	ZeroMemory(cookie, buf_len);

	int nStart = -1, nEnd = -1;

	nStart = m_ResponseHeader.find("PHPSESSID=");
	if (nStart != string::npos)
	{
		nEnd = m_ResponseHeader.find(";", nStart);
		if (nEnd != string::npos&&buf_len >= nEnd - nStart)
		{
			memcpy(cookie, m_ResponseHeader.c_str() + nStart, nEnd - nStart);
			return nEnd - nStart;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}
void HttpCommand::UTF8ResponsetoANSI()
{
	string temp;
	UTF8ToANSI(m_ResponseText.c_str(), &temp);
	m_ResponseText.erase();
	m_ResponseText = temp;
	temp.erase();
}

bool HttpCommand::FileUpload(char *ResourcePath, char *FilePath, char *Host, char* PostData, bool beEnDeCrypt)
{
	FILE *pf = fopen(FilePath, "rb");

	if (pf == NULL)
		return false;

	fseek(pf, 0, SEEK_END);//获取文件末尾的读取位置,即文件大小

	long dwfilesize = ftell(pf);	//将文件流的读取位置还原为原先的值

	fseek(pf, 0, SEEK_SET);

	//if (dwfilesize == 0)
	//{
	//	fclose(pf);
	//	return false;
	//}

	char cfileName[64] = { 0 };

	if (NULL == strrchr(FilePath, '\\'))
		memcpy(cfileName, FilePath, strlen(FilePath));
	else
		memcpy(cfileName, strrchr(FilePath, '\\') + 1, strlen(strrchr(FilePath, '\\') + 1));

	string fileName = "";

	ANSIToUTF8(cfileName, &fileName);

	//----------------------post报头--------------------------------

	m_RequestHeader.erase();

	m_RequestHeader = m_RequestHeader + "POST " + ResourcePath + " HTTP/1.1\r\n";

	if (Host == NULL)
	{
		m_RequestHeader = m_RequestHeader + "Host: " + server_addr + ":" + m_strport + "\r\n";
	}
	else
	{
		m_RequestHeader = m_RequestHeader + "Host: " + Host + "\r\n";
	}
	m_RequestHeader = m_RequestHeader + "Connection: keep-alive\r\n";
	m_RequestHeader = m_RequestHeader + "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.157 Safari/537.36\r\n";
	m_RequestHeader = m_RequestHeader + "Content-Type: multipart/form-data; boundary=-------------------------acebdf13572468\r\n";

	//----------------------post报数据--------------------------------

	std::string content("");

	if (PostData != NULL)
	{
		// 加密文件上传附带的数据

		// ---------------------------------------------------------
		// 加密发送的HTTP数据部分

		unsigned long Crypt_DataLen = strlen(PostData);
		unsigned char * pCrypt_Data = new unsigned char[Crypt_DataLen];
		memset(pCrypt_Data, 0, Crypt_DataLen);
		memcpy(pCrypt_Data, PostData, Crypt_DataLen);

		EnDeCrypt(EnDeCrypt_KEy, pCrypt_Data, Crypt_DataLen);

		// ---------------------------------------------------------
		// 解密验证

		//EnDeCrypt(EnDeCrypt_KEy,pCrypt_Data, Crypt_DataLen);

		// ---------------------------------------------------------

		string strPostData = hex_to_str(pCrypt_Data, Crypt_DataLen);

		delete[] pCrypt_Data;

		// ---------------------------------------------------------

		content += "--";
		content += HTTP_BOUNDARY;
		content += "\r\nContent-Disposition: form-data; name=\"Data\"\r\n\r\n";
		content += strPostData;
		content += "\r\n";
	}

	content += "---------------------------acebdf13572468\r\n";
	content += "Content-Disposition: form-data; name=\"upfile\"; filename=\"";
	content += fileName;
	content += "\"\r\n";
	content += "Content-Type: application/octet-stream\r\n\r\n";

	//post尾时间戳
	std::string strContent("\r\n---------------------------acebdf13572468--\r\n");

	//注意下面这个参数Content-Length，这个参数值是：http请求头长度+请求尾长度+文件总长度
	char temp[64] = { 0 };

	sprintf(temp, "Content-Length: %d\r\n\r\n", content.length() + dwfilesize + strContent.length());

	m_RequestHeader = m_RequestHeader + temp + content;

	//----------------------post头结束-----------------------------------

	//发送post头
	send(m_socket, m_RequestHeader.c_str(), m_RequestHeader.length(), 0);

	// 发送文件

	long ddfilesize = dwfilesize;
	char* pBuffer = new char[4096];
	long long filesize = 0;
	int b = 0;
	while (dwfilesize)
	{
		long readLen = 0;

		memset(pBuffer, 0, 4096);

		readLen = fread(pBuffer, sizeof(char), 4096, pf);

		if (beEnDeCrypt == true)
		{
			//		 加密发送的文件主体
			EnDeCrypt(EnDeCrypt_KEy, (unsigned char *)pBuffer, readLen);
		}

		::send(m_socket, pBuffer, readLen, 0);

		filesize += readLen;

		int a = 0;
		long long c = 0;
		c = filesize * 100;
		a = c / ddfilesize;

		Sleep(0.2);
		b = a;
		dwfilesize = dwfilesize - readLen;
	}

	fclose(pf);

	pf = NULL;

	::send(m_socket, strContent.c_str(), strContent.length(), 0);

	char szBuffer[1024] = { 0 };

	while (true)
	{
		int nRet = ::recv(m_socket, szBuffer, sizeof(szBuffer), 0);
		if (nRet == 0 || nRet == WSAECONNRESET)
		{
			//ATLTRACE("Connection Closed.\n");
			return true;
			break;
		}
		else if (nRet == SOCKET_ERROR)
		{
			//ATLTRACE("socket error\n");
			return true;
			break;
		}
		else
		{
			break;
		}
	}
	::closesocket(m_socket);

	delete[] pBuffer;
	pBuffer = NULL;

	return true;
}

bool HttpCommand::FileDownload(char *ResourcePath, char *FilePath, char *Host)
{
	// ---------------------------
	// 初始化文件

	FILE *pf = fopen(FilePath, "wb");

	if (pf == NULL)
		return false;

	// ---------------------------
	// 初始化数据头

	m_RequestHeader.erase();

	m_RequestHeader = m_RequestHeader + "GET " + ResourcePath + " HTTP/1.1\r\n";

	if (Host == NULL)
	{
		m_RequestHeader = m_RequestHeader + "Host: " + server_addr + ":" + m_strport + "\r\n";
	}
	else
	{
		m_RequestHeader = m_RequestHeader + "Host: " + Host + "\r\n";
	}
	m_RequestHeader = m_RequestHeader + "Connection: keep-alive\r\n";
	m_RequestHeader = m_RequestHeader + "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.157 Safari/537.36\r\n";
	m_RequestHeader = m_RequestHeader + "\r\n\r\n";

	send(m_socket, m_RequestHeader.c_str(), m_RequestHeader.length(), 0);

	// ---------------------------
	// 获取Content-Length

	char *szBuffer = new char[1024];

	memset(szBuffer, 0, sizeof(szBuffer));

	char c;
	int i = 0;

	while (1)
	{
		recv(m_socket, &c, 1, 0);
		szBuffer[i++] = c;
		if (i >= 4)
		{
			if (szBuffer[i - 4] == '\r' && szBuffer[i - 3] == '\n' &&
				szBuffer[i - 2] == '\r' && szBuffer[i - 1] == '\n')
			{
				break;
			}
		}
	}

	szBuffer[i] = '\0';

	char*		pos = NULL;
	DWORD		dwfilesize = 0;

	char strResponse[] = "Content-Length";
	pos = strstr(szBuffer, strResponse);
	pos += (strlen(strResponse) + 2);

	while (*pos != '\r')
	{
		dwfilesize = dwfilesize * 10 + *pos - 48;
		++pos;
	}

	delete[] szBuffer;
	szBuffer = NULL;

	if (/*strstr(szBuffer, "200 OK") == NULL ||*/ dwfilesize <= 0)
	{
		return false;
	}

	// ---------------------------
	// 接收文件数据

	DWORD		DataLen = 0;
	long ddfilesize = 0;
	ddfilesize = dwfilesize;
	long filesize = 0;
	DWORD datalen = 0;
	char*    pBuffer = new char[4096];
	int b = 0;
	while (dwfilesize)
	{
		memset(pBuffer, 0, sizeof(pBuffer));

		DataLen = ::recv(m_socket, pBuffer, sizeof(pBuffer), 0);

		fwrite(pBuffer, sizeof(char), DataLen, pf);
		datalen = DataLen;

		filesize += datalen;

		int a = (filesize * 100) / ddfilesize;

		dwfilesize = dwfilesize - DataLen;

		b = a;

		Sleep(0.2);
	}

	::closesocket(m_socket);

	fclose(pf);
	pf = NULL;

	delete[] pBuffer;
	pBuffer = NULL;

	return true;
}

bool HttpCommand::DataDownload(char *ResourcePath, unsigned char * &Data, DWORD &DataSize, char *Host)
{
	// ---------------------------
	// 初始化数据头

	m_RequestHeader.erase();

	m_RequestHeader = m_RequestHeader + "GET " + ResourcePath + " HTTP/1.1\r\n";

	if (Host == NULL)
	{
		m_RequestHeader = m_RequestHeader + "Host: " + server_addr + ":" + m_strport + "\r\n";
	}
	else
	{
		m_RequestHeader = m_RequestHeader + "Host: " + Host + "\r\n";
	}
	m_RequestHeader = m_RequestHeader + "Connection: keep-alive\r\n";
	m_RequestHeader = m_RequestHeader + "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.157 Safari/537.36\r\n";
	m_RequestHeader = m_RequestHeader + "\r\n\r\n";

	send(m_socket, m_RequestHeader.c_str(), m_RequestHeader.length(), 0);

	// ---------------------------
	// 获取Content-Length

	char *szBuffer = new char[1024];

	memset(szBuffer, 0, sizeof(szBuffer));

	char c;
	int i = 0;

	while (1)
	{
		recv(m_socket, &c, 1, 0);
		szBuffer[i++] = c;
		if (i >= 4)
		{
			if (szBuffer[i - 4] == '\r' && szBuffer[i - 3] == '\n' &&
				szBuffer[i - 2] == '\r' && szBuffer[i - 1] == '\n')
			{
				break;
			}
		}
	}

	szBuffer[i] = '\0';

	char*		pos = NULL;
	DWORD		dwfilesize = 0;

	char strResponse[] = "Content-Length";
	pos = strstr(szBuffer, strResponse);
	pos += (strlen(strResponse) + 2);

	while (*pos != '\r')
	{
		dwfilesize = dwfilesize * 10 + *pos - 48;
		++pos;
	}

	if (strstr(szBuffer, "200 OK") == NULL || dwfilesize <= 0)
	{
		delete[] szBuffer;
		szBuffer = NULL;
		return false;
	}

	delete[] szBuffer;
	szBuffer = NULL;

	// ---------------------------
	// 接收文件数据

	DataSize = dwfilesize;

	Data = new unsigned char[dwfilesize + 1];

	char*	pBuffer = new char[4096];

	DWORD	DataLen = 0;
	DWORD	RecvLen = 0;

	memset(Data, 0, dwfilesize + 1);

	while (DataLen < dwfilesize)
	{
		memset(pBuffer, 0, sizeof(pBuffer));

		RecvLen = ::recv(m_socket, pBuffer, sizeof(pBuffer), 0);

		memcpy(Data + DataLen, pBuffer, RecvLen);

		DataLen = DataLen + RecvLen;

		Sleep(0.2);
	}

	::closesocket(m_socket);

	delete[] pBuffer;
	pBuffer = NULL;

	return true;
}

void HttpCommand::ResetSocket()
{
	m_bConnected = false;
	if (m_socket != INVALID_SOCKET)
	{
		closesocket(m_socket);
		m_socket = INVALID_SOCKET;
	}
}

void HttpCommand::to64frombits(unsigned char *out, const unsigned char *in, int inlen)
{
	const char base64digits[] =
		"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
	for (; inlen >= 3; inlen -= 3)
	{
		*out++ = base64digits[in[0] >> 2];
		*out++ = base64digits[((in[0] << 4) & 0x30) | (in[1] >> 4)];
		*out++ = base64digits[((in[1] << 2) & 0x3c) | (in[2] >> 6)];
		*out++ = base64digits[in[2] & 0x3f];
		in += 3;
	}
	if (inlen > 0)
	{
		unsigned char fragment;
		*out++ = base64digits[in[0] >> 2];
		fragment = (in[0] << 4) & 0x30;
		if (inlen > 1)
			fragment |= in[1] >> 4;
		*out++ = base64digits[fragment];
		*out++ = (inlen < 2) ? '=' : base64digits[(in[1] << 2) & 0x3c];
		*out++ = '=';
	}
	*out = '\0';
}
// 从凭证管理器获取代理用户名密码
BOOL HttpCommand::cred(const char* ip, string& usr, string& pwd)
{
	usr = "";
	DWORD dwCount = 0;
	PCREDENTIAL * pCredArray = NULL;
	if (CredEnumerate(NULL, 0, &dwCount, &pCredArray))
	{
		for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
		{
			PCREDENTIAL pCredential = pCredArray[dwIndex];
			string target = pCredential->TargetName;
			if (target == ip)
			{
				usr = pCredential->UserName;
#ifdef _UNICODE
				pwd = (LPCWSTR)pCredential->CredentialBlob;
#else
				pwd = CW2A((LPCWSTR)pCredential->CredentialBlob);
#endif
				break;
			}
		}

		CredFree(pCredArray);
	}
	return (usr.size() > 0);
}

BOOL HttpCommand::Agent()
{
	char * path111 = "Software\\Microsoft\\Windows\\CurrentVersion\\Internet Settings";
	{
		long lRet;
		HKEY hKey;
		TCHAR tchData[64];
		DWORD dwSize;
		lRet = RegOpenKeyEx(HKEY_CURRENT_USER, path111, 0, KEY_ALL_ACCESS, &hKey);//打开注册表

		//////////////////////

		if (lRet == ERROR_SUCCESS)//读操作成功
		{
			DWORD dwType = REG_BINARY;
			REGSAM mode = KEY_READ;
			BYTE value[256] = { 0 };
			DWORD length = 256;
			RegQueryValueEx(hKey, "ProxyEnable", NULL, &dwType, value, &length); //如果打开成功

			int a = bytelen(value);
			if (a == 1)//判断代理是否打开
			{
				return FALSE;
			}

			dwSize = sizeof(tchData);
			lRet = RegQueryValueEx(hKey, "ProxyServer", NULL, NULL, (LPBYTE)tchData, &dwSize); //如果打开成功，则读
			if (lRet == ERROR_SUCCESS)
			{
				const char *d = ":";
				char *p;
				p = strtok(tchData, d);

				strcpy(ppp, p);
				proxyAddr = ppp;

				p = strtok(NULL, d);
				char pppp[24] = "";
				strcpy(pppp, p);
				proxyPort = atoi(pppp);

				RegCloseKey(hKey);//关闭
				//			Set("192.168.99.201","808");

				dailikaiguan = 1;

				return TRUE;
			}
			//以下是读取失败
			else
			{
				printf("fail/n");
				RegCloseKey(hKey);//关闭
				return FALSE;
			}
		}
		//以下是打开失败
		else
		{
			printf("fail/n");
			RegCloseKey(hKey);//关闭
			return false;
		}
		RegCloseKey(hKey);//关闭
		return TRUE;
	}
}

void HttpCommand::proxy_s()
{
	char online_post_data[4096] = "\0";
	memset(online_post_data, 0, sizeof(online_post_data));
	sprintf(online_post_data, "strOrder=10&tmark=113223606&tName=1&rc_host_name=ADA-PC&rc_host_os=Windows7&tbversion=V2.0");

	///////
	struct sockaddr_in addr;
	WSADATA wsd;
	SOCKET sockClient;                                            //socket
	//char recvBuf[1310721];

	if (WSAStartup(MAKEWORD(2, 2), &wsd) != 0)        // 1> 初始化Windows的socket库（API集）
	{
		printf("start up failed!\n");
		return;
	}
	sockClient = socket(AF_INET, SOCK_STREAM, 0);                    // 2> 创建socket
	if (INVALID_SOCKET == sockClient)
	{
		printf("create client socket error!\n");
		return;
	}
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(proxyPort);
	addr.sin_addr.s_addr = inet_addr(proxyAddr);
	//memcpy(&addr.sin_addr.s_addr,hptr->h_addr,sizeof(addr.sin_addr.s_addr));
	//printf("proxy ip=%s\n",inet_ntop(hptr->h_addrtype,hptr->h_addr,str,sizeof(str)));
	if (connect(sockClient, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		closesocket(sockClient);
		perror("connect() fail.");
	}
	char tmp[10240 + 1];
	char authstr[10240 + 1];
	memset(tmp, 0x0, sizeof(tmp));
	memset(authstr, 0x0, sizeof(authstr));
	sprintf(tmp, "%s:%s", user, passwd);
	printf("=======%s", tmp);
	to64frombits((unsigned char*)authstr, (unsigned char*)tmp, strlen(tmp));
	printf("--=====%s", authstr);
	snprintf(tmp, sizeof(tmp), "CONNECT %s:%d HTTP/1.0\r\nProxy-Authorization: Basic %s\r\n\r\n", desthost, destport, authstr);
	send(sockClient, tmp, strlen(tmp), 0);
	printf("-=-=-=-=-=wait for recv-=-=-=-=-=\n");

	memset(tmp, 0x0, sizeof(tmp));
	recv(sockClient, tmp, sizeof(tmp), 0);
	printf("recv=%s", tmp);
	//通过代理
	snprintf(tmp, sizeof(tmp), "POST %s HTTP/1.0\r\n", DATA_PAGE);
	strcpy(tmp + strlen(tmp), "Host:192.168.11.200\r\n");
	strcpy(tmp + strlen(tmp), "Connection: keep-alive\r\n");
	strcpy(tmp + strlen(tmp), "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.157 Safari/537.36\r\n");
	strcpy(tmp + strlen(tmp), "Accept: */*\r\n");

	unsigned long Crypt_DataLen = strlen(online_post_data);
	char * pCrypt_Data = new  char[Crypt_DataLen + 1];
	memset(pCrypt_Data, 0, Crypt_DataLen + 1);
	memcpy(pCrypt_Data, online_post_data, Crypt_DataLen);

	char Key[256] = { "justfortest" };
	Key_Init(EnDeCrypt_KEy, (unsigned char *)Key, strlen(Key));
	EnDeCrypt(EnDeCrypt_KEy, (unsigned char*)pCrypt_Data, Crypt_DataLen);

	char* c_Post = new char[strlen(tmp) + Crypt_DataLen + 100];

	memset(c_Post, 0, strlen(tmp) + Crypt_DataLen + 100);

	int ContentLength = Crypt_DataLen;

	char ctemp[10];
	_itoa_s(ContentLength, ctemp, 10);

	strcat(c_Post, tmp);
	strcat(c_Post, "Content-Type: application/x-www-form-urlencoded\r\n");
	strcat(c_Post, "Content-Length: ");
	strcat(c_Post, ctemp);
	strcat(c_Post, "\r\n\r\n");
	int c_Post_length = strlen(c_Post);
	memcpy(c_Post + c_Post_length, pCrypt_Data, Crypt_DataLen);
	memcpy(c_Post + c_Post_length + Crypt_DataLen, "\r\n\r\n", 4);
	delete[] pCrypt_Data;

	if (send(sockClient, c_Post, c_Post_length + Crypt_DataLen + 4, 0) == SOCKET_ERROR)
	{
		delete[]c_Post;
		printf("send err");
		return;
	}

	//////////////////////////////////////////////////////////////
	// 	//通过代理get，此处的url一定要是全路径带上http://，否则访问不到
	// 	snprintf(tmp, sizeof(tmp), "GET %s HTTP/1.0\r\n", "http://www.baidu.com/");
	// 	strcpy(tmp + strlen(tmp), "Accept:*/*\r\n");
	// 	strcpy(tmp + strlen(tmp), "User-Agent: Mozilla/5.0 (Windows; U; Windows NT 5.1; en-US; rv:1.7.6)");
	// 	strcpy(tmp + strlen(tmp), "Accept-Language:zh-cn\r\n");
	// 	strcpy(tmp + strlen(tmp), "Connection:close\r\n\r\n");
	// 	send(sockClient, tmp, strlen(tmp), 0);
	/////////////////////////////////////////////////////

	memset(tmp, 0x0, sizeof(tmp));
	Sleep(5);
	recv(sockClient, tmp, sizeof(tmp), 0);
	printf("recv2=%s", tmp);
	closesocket(sockClient);
	return;
}

bool HttpCommand::FileUpload1(char *ResourcePath, char *FilePath, char *Host, char* PostData /*= NULL*/, bool beEnDeCrypt /*= false*/)
{
	FILE *pf = fopen(FilePath, "rb");

	if (pf == NULL)
		return false;

	fseek(pf, 0, SEEK_END);//获取文件末尾的读取位置,即文件大小

	long dwfilesize = ftell(pf);	//将文件流的读取位置还原为原先的值

	fseek(pf, 0, SEEK_SET);

	//if (dwfilesize == 0)
	//{
	//	fclose(pf);
	//	return false;
	//}

	char cfileName[64] = { 0 };

	if (NULL == strrchr(FilePath, '\\'))
		memcpy(cfileName, FilePath, strlen(FilePath));
	else
		memcpy(cfileName, strrchr(FilePath, '\\') + 1, strlen(strrchr(FilePath, '\\') + 1));

	string fileName = "";

	ANSIToUTF8(cfileName, &fileName);

	//----------------------post报头--------------------------------

	m_RequestHeader.erase();

	m_RequestHeader = m_RequestHeader + "POST " + ResourcePath + " HTTP/1.1\r\n";

	if (Host == NULL)
	{
		m_RequestHeader = m_RequestHeader + "Host: " + server_addr + ":" + m_strport + "\r\n";
	}
	else
	{
		m_RequestHeader = m_RequestHeader + "Host: " + Host + "\r\n";
	}
	m_RequestHeader = m_RequestHeader + "Connection: keep-alive\r\n";
	m_RequestHeader = m_RequestHeader + "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.157 Safari/537.36\r\n";
	m_RequestHeader = m_RequestHeader + "Content-Type: multipart/form-data; boundary=-------------------------acebdf13572468\r\n";

	//----------------------post报数据--------------------------------

	std::string content("");

	if (PostData != NULL)
	{
		// 加密文件上传附带的数据

		// ---------------------------------------------------------
		// 加密发送的HTTP数据部分

		unsigned long Crypt_DataLen = strlen(PostData);
		unsigned char * pCrypt_Data = new unsigned char[Crypt_DataLen];
		memset(pCrypt_Data, 0, Crypt_DataLen);
		memcpy(pCrypt_Data, PostData, Crypt_DataLen);

		EnDeCrypt(EnDeCrypt_KEy, pCrypt_Data, Crypt_DataLen);

		// ---------------------------------------------------------
		// 解密验证

		//EnDeCrypt(EnDeCrypt_KEy,pCrypt_Data, Crypt_DataLen);

		// ---------------------------------------------------------

		string strPostData = hex_to_str(pCrypt_Data, Crypt_DataLen);

		delete[] pCrypt_Data;

		// ---------------------------------------------------------

		content += "--";
		content += HTTP_BOUNDARY;
		content += "\r\nContent-Disposition: form-data; name=\"Data\"\r\n\r\n";
		content += strPostData;
		content += "\r\n";
	}

	content += "---------------------------acebdf13572468\r\n";
	content += "Content-Disposition: form-data; name=\"upfile\"; filename=\"";
	content += fileName;
	content += "\"\r\n";
	content += "Content-Type: application/octet-stream\r\n\r\n";

	//post尾时间戳
	std::string strContent("\r\n---------------------------acebdf13572468--\r\n");

	//注意下面这个参数Content-Length，这个参数值是：http请求头长度+请求尾长度+文件总长度
	char temp[64] = { 0 };

	sprintf(temp, "Content-Length: %d\r\n\r\n", content.length() + dwfilesize + strContent.length());

	m_RequestHeader = m_RequestHeader + temp + content;

	//----------------------post头结束-----------------------------------

	//发送post头
	send(m_socket, m_RequestHeader.c_str(), m_RequestHeader.length(), 0);
	//	IsResult;
	// 发送文件

	char* pBuffer = new char[4096];

	while (dwfilesize)
	{
		long readLen = 0;

		memset(pBuffer, 0, 4096);

		readLen = fread(pBuffer, sizeof(char), 4096, pf);

		if (beEnDeCrypt == true)
		{
			// 加密发送的文件主体
			EnDeCrypt(EnDeCrypt_KEy, (unsigned char *)pBuffer, readLen);
		}

		::send(m_socket, pBuffer, readLen, 0);
		Sleep(0.2);

		dwfilesize = dwfilesize - readLen;
	}

	fclose(pf);

	pf = NULL;

	::send(m_socket, strContent.c_str(), strContent.length(), 0);

	char szBuffer[1024] = { 0 };

	while (true)
	{
		int nRet = ::recv(m_socket, szBuffer, sizeof(szBuffer), 0);
		if (nRet == 0 || nRet == WSAECONNRESET)
		{
			//ATLTRACE("Connection Closed.\n");
			return true;
			break;
		}
		else if (nRet == SOCKET_ERROR)
		{
			//ATLTRACE("socket error\n");
			return true;
			break;
		}
		else
		{
			break;
		}
	}
	::closesocket(m_socket);

	delete[] pBuffer;
	pBuffer = NULL;

	return true;
}

bool HttpCommand::FileUploadforKEY(char *ResourcePath, char *FilePath, char *Host, char* PostData /*= NULL*/, bool beEnDeCrypt /*= false*/)
{
	FILE *pf = fopen(FilePath, "rb");

	if (pf == NULL)
		return false;

	fseek(pf, 0, SEEK_END);//获取文件末尾的读取位置,即文件大小

	long dwfilesize = ftell(pf);	//将文件流的读取位置还原为原先的值

	fseek(pf, 0, SEEK_SET);

	//if (dwfilesize == 0)
	//{
	//	fclose(pf);
	//	return false;
	//}

	char cfileName[64] = { 0 };

	if (NULL == strrchr(FilePath, '\\'))
		memcpy(cfileName, FilePath, strlen(FilePath));
	else
		memcpy(cfileName, strrchr(FilePath, '\\') + 1, strlen(strrchr(FilePath, '\\') + 1));

	string fileName = "";

	ANSIToUTF8(cfileName, &fileName);

	//----------------------post报头--------------------------------

	m_RequestHeader.erase();

	m_RequestHeader = m_RequestHeader + "POST " + ResourcePath + " HTTP/1.1\r\n";

	if (Host == NULL)
	{
		m_RequestHeader = m_RequestHeader + "Host: " + server_addr + ":" + m_strport + "\r\n";
	}
	else
	{
		m_RequestHeader = m_RequestHeader + "Host: " + Host + "\r\n";
	}
	m_RequestHeader = m_RequestHeader + "Connection: keep-alive\r\n";
	m_RequestHeader = m_RequestHeader + "User-Agent: Mozilla/5.0 (Windows NT 10.0; WOW64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/35.0.1916.157 Safari/537.36\r\n";
	m_RequestHeader = m_RequestHeader + "Content-Type: multipart/form-data; boundary=-------------------------acebdf13572468\r\n";

	//----------------------post报数据--------------------------------

	std::string content("");

	if (PostData != NULL)
	{
		// 加密文件上传附带的数据

		// ---------------------------------------------------------
		// 加密发送的HTTP数据部分

		unsigned long Crypt_DataLen = strlen(PostData);
		unsigned char * pCrypt_Data = new unsigned char[Crypt_DataLen];
		memset(pCrypt_Data, 0, Crypt_DataLen);
		memcpy(pCrypt_Data, PostData, Crypt_DataLen);

		EnDeCrypt(EnDeCrypt_KEy, pCrypt_Data, Crypt_DataLen);

		// ---------------------------------------------------------
		// 解密验证

		//EnDeCrypt(EnDeCrypt_KEy,pCrypt_Data, Crypt_DataLen);

		// ---------------------------------------------------------

		string strPostData = hex_to_str(pCrypt_Data, Crypt_DataLen);

		delete[] pCrypt_Data;

		// ---------------------------------------------------------

		content += "--";
		content += HTTP_BOUNDARY;
		content += "\r\nContent-Disposition: form-data; name=\"Data\"\r\n\r\n";
		content += strPostData;
		content += "\r\n";
	}

	content += "---------------------------acebdf13572468\r\n";
	content += "Content-Disposition: form-data; name=\"upfile\"; filename=\"";
	content += fileName;
	content += "\"\r\n";
	content += "Content-Type: application/octet-stream\r\n\r\n";

	//post尾时间戳
	std::string strContent("\r\n---------------------------acebdf13572468--\r\n");

	//注意下面这个参数Content-Length，这个参数值是：http请求头长度+请求尾长度+文件总长度
	char temp[64] = { 0 };

	sprintf(temp, "Content-Length: %d\r\n\r\n", content.length() + dwfilesize + strContent.length());

	m_RequestHeader = m_RequestHeader + temp + content;

	//----------------------post头结束-----------------------------------

	//发送post头
	send(m_socket, m_RequestHeader.c_str(), m_RequestHeader.length(), 0);
	//	IsResult;
	// 发送文件

	char* pBuffer = new char[4096];
	/*	const char*pBufferA = new const char[4096];*/

	while (dwfilesize)
	{
		long readLen = 0;

		memset(pBuffer, 0, 4096);

		readLen = fread(pBuffer, sizeof(char), 4096, pf);

		string ppp = "";

		ANSIToUTF8(pBuffer, &ppp);

		/*	pBufferA = ppp.c_str();*/

		if (beEnDeCrypt == true)
		{
			// 加密发送的文件主体
			EnDeCrypt(EnDeCrypt_KEy, (unsigned char *)ppp.c_str(), readLen);
		}

		::send(m_socket, ppp.c_str(), readLen, 0);
		Sleep(0.2);

		dwfilesize = dwfilesize - readLen;
	}

	fclose(pf);

	pf = NULL;

	::send(m_socket, strContent.c_str(), strContent.length(), 0);

	char szBuffer[1024] = { 0 };

	while (true)
	{
		int nRet = ::recv(m_socket, szBuffer, sizeof(szBuffer), 0);
		if (nRet == 0 || nRet == WSAECONNRESET)
		{
			//ATLTRACE("Connection Closed.\n");
			return true;
			break;
		}
		else if (nRet == SOCKET_ERROR)
		{
			//ATLTRACE("socket error\n");
			return true;
			break;
		}
		else
		{
			break;
		}
	}
	::closesocket(m_socket);

	delete[] pBuffer;

	pBuffer = NULL;

	return true;
}