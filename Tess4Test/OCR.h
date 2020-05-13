#pragma once
#include "windows.h"
#include <corecrt_wstdio.h>
#include <afxstr.h>
#include "tesseract/capi.h"
#include "leptonica/allheaders.h"
#include "string.h"
using namespace std;
//OCR识别

//OCR错误输出
void die(const WCHAR *errstr) {
	fputws(errstr, stderr);
	exit(1);
}

//转码
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

	//读取图片，原图像的路径
	if ((img = pixRead(PATH)) == NULL)
		die(L"Error reading image\n");

	handle = TessBaseAPICreate();
	//加载字库及设置语言
	if (TessBaseAPIInit3(handle, "../tessdata", "eng+chi_sim") != 0)
		die(L"Error initialising tesseract\n");

	//设置图片及识别
	TessBaseAPISetImage2(handle, img);
	if (TessBaseAPIRecognize(handle, NULL) != 0)
		die(L"Error in Tesseract recognition\n");

	if ((text = TessBaseAPIGetUTF8Text(handle)) == NULL)
		die(L"Error getting text\n");

	CHAR *pResult = NULL;
	ConvertUtf8ToGBK(&pResult, text); //对结果转码
//	cout << pResult << endl;   //输出OCR识别的文本信息

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

//截图
HBITMAP ScreenCapture(LPTSTR filename, WORD bitCount, LPRECT lpRect)
{
	HBITMAP hBitmap;
	HDC hScreenDC = CreateDCA("DISPLAY", NULL, NULL, NULL);   //通过指定DISPLAY来获取一个显示设备上下文环境
	HDC hmemDC = CreateCompatibleDC(hScreenDC);            //该函数创建一个与指定设备兼容的内存设备上下文环境（DC）
	int ScreenWidth = GetDeviceCaps(hScreenDC, HORZRES);    //获取指定设备的性能参数（此处获取屏幕宽度）
	int ScreenHeight = GetDeviceCaps(hScreenDC, VERTRES);   //获取指定设备的性能参数（此处获取屏幕高度）
	HBITMAP hOldBM;
	PVOID lpData;   //内存分配成功返回的指向所分配内存块的首地址指针
	int startX;     //从坐标X位置开始截图
	int startY;     //从坐标Y位置
	int width;      //截图宽度
	int height;     //截图高度
	long BitmapSize;
	DWORD BitsOffset;
	DWORD ImageSize;
	DWORD FileSize;
	BITMAPINFOHEADER bmInfo; //BITMAPINFOHEADER结构所包含的成员表明了图像的尺寸、原始设备的颜色格式、以及数据压缩方案
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
	//创建一张长width，宽height的画布，用于后面绘制图形
	hBitmap = CreateCompatibleBitmap(hScreenDC, width, height);
	//该函数选择一对象到指定的设备上下文环境中，该新对象替换先前的相同类型的对象。
	hOldBM = (HBITMAP)SelectObject(hmemDC, hBitmap);
	//该函数对指定的源设备环境区域中的像素进行位块（bit_block）转换，以传送到目标设备环境。
	BitBlt(hmemDC, 0, 0, width, height, hScreenDC, startX, startY, SRCCOPY);
	hBitmap = (HBITMAP)SelectObject(hmemDC, hOldBM);
	if (filename == NULL)
	{
		DeleteDC(hScreenDC);
		DeleteDC(hmemDC);
		return hBitmap;
	}
	BitmapSize = ((((width * 32) + 32) / 32) * 4)*height;
	//用来在指定的堆上分配内存，并且分配后的内存不可移动(HEAP_NO_SERIALIZE 不使用连续存取)
	lpData = HeapAlloc(GetProcessHeap(), HEAP_NO_SERIALIZE, BitmapSize);
	ZeroMemory(lpData, BitmapSize);
	ZeroMemory(&bmInfo, sizeof(BITMAPINFOHEADER));
	bmInfo.biSize = sizeof(BITMAPINFOHEADER); //位图信息结构长度 ,必须为40
	bmInfo.biWidth = width;                   //图像宽度 单位是像素
	bmInfo.biHeight = height;                 //图像高度 单位是像素
	bmInfo.biPlanes = 1;                      //必须为1
	bmInfo.biBitCount = bitCount;             //设置图像的位数。比如8位，16位，32位位数越高分辨率越高
	bmInfo.biCompression = BI_RGB;            //位图是否压缩 BI_RGB为不压缩
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
		MessageBoxA(NULL, "Create File Error!", "提示", MB_OK | MB_ICONWARNING);
	}
	WriteFile(bmFile, &bmFileHeader, sizeof(BITMAPFILEHEADER), &dwWritten, NULL);
	WriteFile(bmFile, &bmInfo, sizeof(BITMAPINFOHEADER), &dwWritten, NULL);
	WriteFile(bmFile, lpData, ImageSize, &dwWritten, NULL);
	CloseHandle(bmFile);
	HeapFree(GetProcessHeap(), HEAP_NO_SERIALIZE, lpData);
	::ReleaseDC(0, hScreenDC);
	DeleteDC(hmemDC);
	printf("【截图保存成功】");
	return hBitmap;
}
//保存截图
CString CaptureLolToDesktop(HWND hWnd, string str)
{
	//	Sleep(1 * 1000);
	LPRECT lprc = new RECT;
	GetWindowRect(hWnd, lprc);
	CString strTime;

	// 	WCHAR chDesktopPath[MAX_PATH] = { 0 };
	// 	SHGetSpecialFolderPathA(NULL, chDesktopPath, CSIDL_DESKTOP, 0);//获取当前用户桌面路径

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