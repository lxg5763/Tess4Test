#pragma once
#include "windows.h"
#include <string>

#include <afxstr.h>
#include "OCR.h"
#include "ģ����.h"

using namespace std;

std::string WhatsAPPTesting(HWND winhd, string data_phone)
{
	CString result_0;
	CString Bildopath;//��ͼ·��
	CreateVcf(data_phone);

	ImportPhone();//�����뵼��ģ����
	Sleep(2000);

	string StopAPP = NoxBinPath + " " + "shell am force-stop com.whatsapp";//�ر�whatsAPP
	WinExec(StopAPP.c_str(), SW_SHOW);

	Sleep(2000);
	string OpenAPP = NoxBinPath + " " + "shell am start com.whatsapp/com.whatsapp.HomeActivity";//��whatsAPP
	WinExec(OpenAPP.c_str(), SW_SHOW);

	Sleep(5000);
	Bildopath = CaptureLolToDesktop(winhd, data_phone);//�����ͼ����·��
	Sleep(1000);

	result_0 = Tocr(Bildopath.GetBuffer(Bildopath.GetLength()));//ʶ��

	string register_result;
	if (result_0 == "")
	{
		register_result = "��";
		printf("%sδע��\r\n", data_phone.c_str());
		return register_result;
	}
	else
	{
		printf("%s��ע��\r\n", data_phone.c_str());
		register_result = "��";
		return register_result;
	}
}
