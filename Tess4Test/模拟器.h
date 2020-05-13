#pragma once
#include "windows.h"
#include <string>
using namespace std;

//ģ����ADB·��
string NoxBinPath = "D:\\Program Files\\Nox\\bin\\adb.exe";
//���������
void MouseLeftClick(HWND mh, int x, int y)
{
	SetCursorPos(x, y);
	mouse_event(MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
}
//��������
void KeyboardInput(string s)
{
	for (unsigned int  i = 0; i < s.length(); i++)
	{
		keybd_event(s[i], 0, 0, 0);    // ѭ������
		Sleep(10);
	}
}
//����vcfͨѶ¼�ļ�
BOOL CreateVcf(string phone)
{
	HANDLE pFile;
	const char *tmpBuf;
	DWORD dwBytesWrite, dwBytesToWrite;

	pFile = CreateFile("contacts.vcf", GENERIC_WRITE,
		0,
		NULL,
		CREATE_ALWAYS,        //���Ǵ����ļ�
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

	do {                                       //ѭ��д�ļ���ȷ���������ļ���д��
		WriteFile(pFile, tmpBuf, dwBytesToWrite, &dwBytesWrite, NULL);

		dwBytesToWrite -= dwBytesWrite;
		tmpBuf += dwBytesWrite;
	} while (dwBytesToWrite > 0);

	CloseHandle(pFile);

	return TRUE;
}
//����绰����
BOOL ImportPhone()
{
	//ɾ��ͨѶ¼����
	string DeletePhonebook = "shell pm clear com.android.providers.contacts";

	//����D�̸�Ŀ¼ͨѶ¼�ļ���ģ��������
	string ImportPhoneToSimulator = "push contacts.vcf /sdcard/contacts.vcf";

	//����ͨѶ¼�ļ���ͨѶ¼APP
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
