#pragma once
#include "windows.h"
#include <string>

#include <afxstr.h>
#include "OCR.h"
#include "ģ����.h"

using namespace std;

std::string ViberTesting(HWND winhd, string data_phone)
{
	CString result_0;
	CString Bildopath;//��ͼ·��
	CreateVcf(data_phone);
	ImportPhone();//�����뵼��ģ����
	Sleep(2000);

	string StopAPP = NoxBinPath + " " + "shell am force-stop com.viber.voip";//�ر�whatsAPP
	WinExec(StopAPP.c_str(), SW_SHOW);

	Sleep(2000);
	string OpenAPP = NoxBinPath + " " + "shell am start com.viber.voip/com.viber.voip.HomeActivity";//��whatsAPP
	WinExec(OpenAPP.c_str(), SW_SHOW);

	CString JTpath;
	Sleep(2000);
	MouseLeftClick(winhd, 478, 533);//���
	Sleep(3000);
	JTpath = CaptureLolToDesktop(winhd, data_phone);//�����ͼ����·��
	Sleep(1000);

	result_0 = Tocr(JTpath.GetBuffer(JTpath.GetLength()));//ʶ��
	string register_result;
	if (result_0 == "")
	{
		register_result = "��";
		printf("%s��ע��\r\n", data_phone.c_str());
	}
	else
	{
		printf("%sδע��\r\n", data_phone.c_str());
		register_result = "��";
	}
	return register_result;
}