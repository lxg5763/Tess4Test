#pragma once
#include "windows.h"
#include <corecrt_wstdio.h>
#include <afxstr.h>
#include "tesseract/capi.h"
#include "leptonica/allheaders.h"
#include "string.h"
using namespace std;
//OCRʶ��

//OCR�������
void die(const WCHAR *errstr) {
	fputws(errstr, stderr);
	exit(1);
}

//ת��
void ConvertUtf8ToGBK(CHAR **amp, CHAR *strUtf8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)strUtf8, -1, NULL, 0);
	unsigned short * wszGBK = new unsigned short[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, (LPCSTR)strUtf8, -1, (LPWSTR)wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, NULL, 0, NULL, NULL);
	//WCHAR *szGBK=new WCHAR[len + 1];
	*amp = new CHAR[len + 1];
	memset(*amp, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)wszGBK, -1, *amp, len, NULL, NULL);
}

CString Tocr(CHAR*JPGpath)
{
	TessBaseAPI *handle;
	PIX *img;
	CHAR *text = NULL;
	CHAR PATH[256];
	sprintf(PATH, "%s", JPGpath);

	//��ȡͼƬ��ԭͼ���·��
	if ((img = pixRead(PATH)) == NULL)
		die(L"Error reading image\n");

	handle = TessBaseAPICreate();
	//�����ֿ⼰��������
	if (TessBaseAPIInit3(handle, "../tessdata", "eng+chi_sim") != 0)
		die(L"Error initialising tesseract\n");

	//����ͼƬ��ʶ��
	TessBaseAPISetImage2(handle, img);
	if (TessBaseAPIRecognize(handle, NULL) != 0)
		die(L"Error in Tesseract recognition\n");

	if ((text = TessBaseAPIGetUTF8Text(handle)) == NULL)
		die(L"Error getting text\n");

	CHAR *pResult = NULL;
	ConvertUtf8ToGBK(&pResult, text); //�Խ��ת��
//	cout << pResult << endl;   //���OCRʶ����ı���Ϣ

	TessDeleteText(text);
	TessBaseAPIEnd(handle);
	TessBaseAPIDelete(handle);
	pixDestroy(&img);
	return pResult;
}

void GetFpath(TCHAR *buff)
{
	int Dlen = strlen(buff);
	if (Dlen == 0) return;
	int i = 0;
	while (Dlen > 0)
	{
		if (buff[Dlen - 1] == '\\')
		{
			buff[Dlen - 1] = '\0';
			break;
		}
		else
		{
			buff[Dlen - 1] = '\0';
			Dlen--;
			i++;
		}
	}
}

//��ͼ
HBITMAP ScreenCapture(LPTSTR filename, WORD bitCount, LPRECT lpRect)
{
	HBITMAP hBitmap;
	HDC hScreenDC = CreateDCA("DISPLAY", NULL, NULL, NULL);   //ͨ��ָ��DISPLAY����ȡһ����ʾ�豸�����Ļ���
	HDC hmemDC = CreateCompatibleDC(hScreenDC);            //�ú�������һ����ָ���豸���ݵ��ڴ��豸�����Ļ�����DC��
	int ScreenWidth = GetDeviceCaps(hScreenDC, HORZRES);    //��ȡָ���豸�����ܲ������˴���ȡ��Ļ��ȣ�
	int ScreenHeight = GetDeviceCaps(hScreenDC, VERTRES);   //��ȡָ���豸�����ܲ������˴���ȡ��Ļ�߶ȣ�
	HBITMAP hOldBM;
	PVOID lpData;   //�ڴ����ɹ����ص�ָ���������ڴ����׵�ַָ��
	int startX;     //������Xλ�ÿ�ʼ��ͼ
	int startY;     //������Yλ��
	int width;      //��ͼ���
	int height;     //��ͼ�߶�
	long BitmapSize;
	DWORD BitsOffset;
	DWORD ImageSize;
	DWORD FileSize;
	BITMAPINFOHEADER bmInfo; //BITMAPINFOHEADER�ṹ�������ĳ�Ա������ͼ��ĳߴ硢ԭʼ�豸����ɫ��ʽ���Լ�����ѹ������
	BITMAPFILEHEADER bmFileHeader;
	HANDLE bmFile, hpal, holdpal = NULL;;
	DWORD dwWritten;
	if (lpRect == NULL)
	{
		startX = startY = 0;
		width = ScreenWidth;
		height = ScreenHeight;
	}
	else
	{
		startX = 132;
		//	startY = lpRect->top;
		startY = 212;
		width = 150;
		//	height = lpRect->bottom - lpRect->top;
		height = 40;
	}
	//����һ�ų�width����height�Ļ��������ں������ͼ��
	hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	//�ú���ѡ��һ����ָ�����豸�����Ļ����У����¶����滻��ǰ����ͬ���͵Ķ���
	hOldBM = (HBITMAP)SelectObject(hmemDC, hBitmap);
	//�ú�����ָ����Դ�豸���������е����ؽ���λ�飨bit_block��ת�����Դ��͵�Ŀ���豸������
	BitBlt(hmemDC, 0, 0, width, height, hScreenDC, startX, startY, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hmemDC, hOldBM);
	if (filename == NULL)
	{
		DeleteDC(hScreenDC);
		DeleteDC(hmemDC);
		return hBitmap;
	}
	BitmapSize = ((((width * 32) + 32) / 32) * 4)*height;
	//������ָ���Ķ��Ϸ����ڴ棬���ҷ������ڴ治���ƶ�(HEAP_NO_SERIALIZE ��ʹ��������ȡ)
	lpData = HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE, BitmapSize);
	ZeroMemory(lpData, BitmapSize);
	ZeroMemory(&bmInfo, sizeof(BITMAPINFOHEADER));
	bmInfo.biSize = sizeof(BITMAPINFOHEADER); //λͼ��Ϣ�ṹ���� ,����Ϊ40
	bmInfo.biWidth = width;                   //ͼ���� ��λ������
	bmInfo.biHeight = height;                 //ͼ��߶� ��λ������
	bmInfo.biPlanes = 1;                      //����Ϊ1
	bmInfo.biBitCount = bitCount;             //����ͼ���λ��������8λ��16λ��32λλ��Խ�߷ֱ���Խ��
	bmInfo.biCompression = BI_RGB;            //λͼ�Ƿ�ѹ�� BI_RGBΪ��ѹ��
	ZeroMemory(&bmFileHeader, sizeof(BITMAPFILEHEADER));
	BitsOffset = sizeof(BITMAPFILEHEADER) + bmInfo.biSize;
	ImageSize = ((((bmInfo.biWidth*bmInfo.biBitCount) + 31) / 32) * 4)*bmInfo.biHeight;
	FileSize = BitsOffset + ImageSize;
	bmFileHeader.bfType = 0x4d42;//'B'+('M'<<8);
	bmFileHeader.bfSize = FileSize;
	bmFileHeader.bfOffBits = BitsOffset;
	hpal = GetStockObject(DEFAULT_PALETTE);
	if (hpal)
	{
		holdpal = SelectPalette(hmemDC, (HPALETTE)hpal, false);
		RealizePalette(hmemDC);
	}
	GetDIBits(hmemDC, hBitmap, 0, bmInfo.biHeight, lpData, (BITMAPINFO *)&bmInfo, DIB_RGB_COLORS);
	if (holdpal)
	{
		SelectPalette(hmemDC, (HPALETTE)holdpal, true);
		RealizePalette(hmemDC);
	}
	bmFile = CreateFile(filename, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (bmFile == INVALID_HANDLE_VALUE)
	{
		MessageBoxA(NULL, "Create File Error!", "��ʾ", MB_OK | MB_ICONWARNING);
	}
	WriteFile(bmFile, &bmFileHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(bmFile, &bmInfo, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
	WriteFile(bmFile, lpData, ImageSize, &dwWritten, NULL);
	CloseHandle(bmFile);
	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpData);
	::ReleaseDC(0, hScreenDC);
	DeleteDC(hmemDC);
	printf("����ͼ����ɹ���");
	return hBitmap;
}
//�����ͼ
CString CaptureLolToDesktop(HWND hWnd, string str)
{
	//	Sleep(1 * 1000);
	LPRECT lprc = new RECT;
	GetWindowRect(hWnd, lprc);
	CString strTime;

	// 	WCHAR chDesktopPath[MAX_PATH] = { 0 };
	// 	SHGetSpecialFolderPathA(NULL, chDesktopPath, CSIDL_DESKTOP, 0);//��ȡ��ǰ�û�����·��

	TCHAR Path[MAX_PATH] = { 0 };
	TCHAR tmpPath[MAX_PATH] = { 0 };
	DWORD len = MAX_PATH;
	GetModuleFileName(NULL, Path, len);
	GetFpath(Path);
	strTime.Format("%s\\JT\\%s.png", Path, str.c_str());

	ScreenCapture(strTime.GetBuffer(strTime.GetLength()), 32, lprc);

	delete lprc;
	return strTime;
}