#pragma once
#include "windows.h"
#include <string>

#include <afxstr.h>
#include "OCR.h"
#include "模拟器.h"

using namespace std;

std::string SingalTesting(HWND winhd, string data_phone)
{
	CString result_0;
	CString Bildopath;//截图路径
	CreateVcf(data_phone);
	ImportPhone();//将号码导入模拟器
	Sleep(2000);

	string StopAPP = NoxBinPath + " " + "shell am force-stop org.thoughtcrime.securesms";//关闭signal
	WinExec(StopAPP.c_str(), SW_SHOW);

	Sleep(2000);
	string OpenAPP = NoxBinPath + " " + "shell am start org.thoughtcrime.securesms/org.thoughtcrime.securesms.RoutingActivity";//打开signal
	WinExec(OpenAPP.c_str(), SW_SHOW);
	;

	CString Signalpath;
	Sleep(5000);
	MouseLeftClick(winhd, 913, 526);//点击搜索
	Sleep(2000);
	MouseLeftClick(winhd, 929, 90);//刷新
	Sleep(1000);
	MouseLeftClick(winhd, 929, 90);//刷新
	Sleep(5000);
	Signalpath = CaptureLolToDesktop(winhd, data_phone);//保存截图返回路径
	Sleep(1000);

	result_0 = Tocr(Bildopath.GetBuffer(Bildopath.GetLength()));
	string register_result;
	if (result_0 == "")
	{
		register_result = "否";
		printf("%s未注册\r\n", data_phone.c_str());
	}
	else
	{
		printf("%s已注册\r\n", data_phone.c_str());
		register_result = "是";
	}
}