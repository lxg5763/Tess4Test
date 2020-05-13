#pragma once
#include "windows.h"
#include <string>
using namespace std;

//模拟器ADB路径
string NoxBinPath = "D:\\Program Files\\Nox\\bin\\adb.exe";
//鼠标左键点击
void MouseLeftClick(HWND mh, int x, int y)
{
	SetCursorPos(x, y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}
//键盘输入
void KeyboardInput(string s)
{
	for (unsigned int  i = 0; i < s.length(); i++)
	{
		keybd_event(s[i], 0, 0, 0);    // 循环按键
		Sleep(10);
	}
}
//创建vcf通讯录文件
BOOL CreateVcf(string phone)
{
	HANDLE pFile;
	const char *tmpBuf;
	DWORD dwBytesWrite, dwBytesToWrite;

	pFile = CreateFile("contacts.vcf", GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,        //总是创建文件
		FILE_ATTRIBUTE_NORMAL,
		NULL);

	if (pFile == INVALID_HANDLE_VALUE)
	{
		printf("create file error!\n");
		CloseHandle(pFile);
		return FALSE;
	}

	string PhoneNumberVcf = "BEGIN:VCARD\r\n";
	PhoneNumberVcf += "VERSION:2.1\r\n";
	PhoneNumberVcf += "N:+";
	PhoneNumberVcf += phone;
	PhoneNumberVcf += ";;;;\r\n";
	PhoneNumberVcf += "TEL;HOME:+";
	PhoneNumberVcf += phone;
	PhoneNumberVcf += "\r\n";
	PhoneNumberVcf += "END:VCARD";

	dwBytesToWrite = PhoneNumberVcf.length();
	dwBytesWrite = 0;

	tmpBuf = PhoneNumberVcf.c_str();

	do {                                       //循环写文件，确保完整的文件被写入
		WriteFile(pFile, tmpBuf, dwBytesToWrite, &dwBytesWrite, NULL);

		dwBytesToWrite -= dwBytesWrite;
		tmpBuf += dwBytesWrite;
	} while (dwBytesToWrite > 0);

	CloseHandle(pFile);

	return TRUE;
}
//导入电话号码
BOOL ImportPhone()
{
	//删除通讯录命令
	string DeletePhonebook = "shell pm clear com.android.providers.contacts";

	//导入D盘根目录通讯录文件到模拟器命令
	string ImportPhoneToSimulator = "push contacts.vcf /sdcard/contacts.vcf";

	//导入通讯录文件到通讯录APP
	string ImportSimulatorPhoneToAPP = "shell am start -t \"text/x-vcard\" -d \"file:///sdcard/contacts.vcf\" -a android.intent.action.VIEW com.android.contacts";

	string DeletePhoneCommand = NoxBinPath + " " + DeletePhonebook;
	WinExec(DeletePhoneCommand.c_str(), SW_SHOW);

	Sleep(1000);
	string ImportPhoneToSimulatorCommand = NoxBinPath + " " + ImportPhoneToSimulator;
	WinExec(ImportPhoneToSimulatorCommand.c_str(), SW_SHOW);

	Sleep(500);
	string ImportSimulatorPhoneToAPPCommand = NoxBinPath + " " + ImportSimulatorPhoneToAPP;
	WinExec(ImportSimulatorPhoneToAPPCommand.c_str(), SW_SHOW);

	return TRUE;
}
