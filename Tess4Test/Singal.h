#pragma once
#include "windows.h"
#include <string>

#include <afxstr.h>
#include "OCR.h"
#include "ģ����.h"

using namespace std;

std::string SingalTesting(HWND winhd, string data_phone)
{
	CString result_0;
	CString Bildopath;//��ͼ·��
	CreateVcf(data_phone);
	ImportPhone();//�����뵼��ģ����
	Sleep(2000);

	string StopAPP = NoxBinPath + " " + "shell am force-stop org.thoughtcrime.securesms";//�ر�signal
	WinExec(StopAPP.c_str(), SW_SHOW);

	Sleep(2000);
	string OpenAPP = NoxBinPath + " " + "shell am start org.thoughtcrime.securesms/org.thoughtcrime.securesms.RoutingActivity";//��signal
	WinExec(OpenAPP.c_str(), SW_SHOW);
	;

	CString Signalpath;
	Sleep(5000);
	MouseLeftClick(winhd, 913, 526);//�������
	Sleep(2000);
	MouseLeftClick(winhd, 929, 90);//ˢ��
	Sleep(1000);
	MouseLeftClick(winhd, 929, 90);//ˢ��
	Sleep(5000);
	Signalpath = CaptureLolToDesktop(winhd, data_phone);//�����ͼ����·��
	Sleep(1000);

	result_0 = Tocr(Bildopath.GetBuffer(Bildopath.GetLength()));
	string register_result;
	if (result_0 == "")
	{
		register_result = "��";
		printf("%sδע��\r\n", data_phone.c_str());
	}
	else
	{
		printf("%s��ע��\r\n", data_phone.c_str());
		register_result = "��";
	}
}