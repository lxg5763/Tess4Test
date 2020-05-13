#pragma once
#include "windows.h"
#include <string>

#include <afxstr.h>
#include "OCR.h"
#include "模拟器.h"

using namespace std;

std::string WhatsAPPTesting(HWND winhd, string data_phone)
{
	CString result_0;
	CString Bildopath;//截图路径
	CreateVcf(data_phone);

	ImportPhone();//将号码导入模拟器
	Sleep(2000);

	string StopAPP = NoxBinPath + " " + "shell am force-stop com.whatsapp";//关闭whatsAPP
	WinExec(StopAPP.c_str(), SW_SHOW);

	Sleep(2000);
	string OpenAPP = NoxBinPath + " " + "shell am start com.whatsapp/com.whatsapp.HomeActivity";//打开whatsAPP
	WinExec(OpenAPP.c_str(), SW_SHOW);

	Sleep(5000);
	Bildopath = CaptureLolToDesktop(winhd, data_phone);//保存截图返回路径
	Sleep(1000);

	result_0 = Tocr(Bildopath.GetBuffer(Bildopath.GetLength()));//识别

	string register_result;
	if (result_0 == "")
	{
		register_result = "否";
		printf("%s未注册\r\n", data_phone.c_str());
		return register_result;
	}
	else
	{
		printf("%s已注册\r\n", data_phone.c_str());
		register_result = "是";
		return register_result;
	}
}
