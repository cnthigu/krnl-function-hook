#pragma once
#include "functions.h"

enum class LogType
{
	Info,
	Success,
	Error,
	Warning
};

void log(LogType type, const string& msg)
{
	string prefix;

	switch (type)
	{
	case LogType::Info:    prefix = "[*] "; break;
	case LogType::Success: prefix = "[+] "; break;
	case LogType::Error:   prefix = "[-] "; break;
	case LogType::Warning: prefix = "[!] "; break;
	}

	cout << prefix << msg << endl;
}

string to_hex(uintptr_t addr)
{
	stringstream ss;
	ss << "0x" << hex << addr;
	return ss.str();
}