#include "StdAfx.h"
#include <string>
#include <stdlib.h>

#include "ClassTraduction.h"

#include <winsock2.h>
#include <windows.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")
using namespace std;


using namespace System;
using namespace System::Net;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

void Page(std::string mot, std::string langueSrc, std::string langueDest)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "WSAStartup failed.\n";
	}
	SOCKET Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct hostent *host;
	host = gethostbyname("www.google.com");
	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(80);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)host->h_addr);
	cout << "Connecting...\n";
	if (connect(Socket, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0) {
		cout << "Could not connect";
	}
	cout << "Connected.\n";
	send(Socket, "GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n", strlen("GET / HTTP/1.1\r\nHost: www.google.com\r\nConnection: close\r\n\r\n"), 0);
	char buffer[10000];
	int nDataLength;
	while ((nDataLength = recv(Socket, buffer, 10000, 0)) > 0) {
		int i = 0;
		while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
			cout << buffer[i];
			i += 1;
		}
	}
	closesocket(Socket);
	WSACleanup();

}

std::string CTraduction::GetTraduction(std::string mot, std::string langueSource, std::string langueDest)
{
	if (mot != "")
	{
		String^ motSharp = gcnew String(mot.c_str());
		motSharp = motSharp->Replace(" ", "+");

		String ^langueSrcSharp = gcnew String(langueSource.c_str());
		String ^langueDstSharp = gcnew String(langueDest.c_str());

		System::String^ page = "";

		WebClient^ wc = gcnew WebClient;
		wc->Headers->Add("user-agent", "Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.2; .NET CLR 1.0.3705;)");

		String^ url = String::Format("http://translate.google.com.tr/m?hl=en&sl={0}&tl={1}&ie=ISO-8859-15&prev=_m&q={2}", langueSrcSharp, langueDstSharp, Uri::EscapeUriString(motSharp));


		page = wc->DownloadString(url);


		String^ mot = page;
		String^ first = "<div dir=\"ltr\" class=\"t0\">";
		String^ second = "</div>";
		if (!mot->Contains(first)) return "";

		List<String^>^ liste1 = gcnew List<String^>();
		liste1->Add(first);

		List<String^>^ liste2 = gcnew List<String^>();
		liste2->Add(second);

		String^ afterFirst = mot->Split(liste1->ToArray(), StringSplitOptions::None)[1];

		if (!mot->Contains(second)) return "";

		page = afterFirst->Split(liste2->ToArray(), StringSplitOptions::None)[0];

		const char* cstr = (const char*)(Marshal::StringToHGlobalAnsi(mot)).ToPointer();
		std::string sstr = cstr;
		Marshal::FreeHGlobal(System::IntPtr((void*)cstr));

		return cstr;
	}

	return "";
}

String^ GetStringBetween(String^ mot, String^ first, String^ second)
{
	if (!mot->Contains(first)) return "";

	List<String^>^ liste1 = gcnew List<String^>();
	liste1->Add(first);

	List<String^>^ liste2 = gcnew List<String^>();
	liste2->Add(second);

	String^ afterFirst = mot->Split(liste1->ToArray(), StringSplitOptions::None)[1];

	if (!mot->Contains(second)) return "";

	return afterFirst->Split(liste2->ToArray(), StringSplitOptions::None)[0];
}


std::string toss(String^ mot)
{
	const char* cstr = (const char*)(Marshal::StringToHGlobalAnsi(mot)).ToPointer();
	std::string sstr = cstr;
	Marshal::FreeHGlobal(System::IntPtr((void*)cstr));
	return sstr;
}