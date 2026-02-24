#include "functions.h"
#include "logs.h"

int main()
{
	log(LogType::Info, "Loading dependencies...");
	LoadLibraryA("user32.dll");

	log(LogType::Info, "Searching process...");
	process_id = get_process_id(L"notepad.exe");

	if (!process_id)
	{
		log(LogType::Error, "Process not found");
		return 1;
	}

	log(LogType::Success, "Process found. PID = " + to_string(process_id));

	log(LogType::Info, "Getting module base...");
	base_address = get_module_base_address("notepad.exe");

	if (!base_address)
	{
		log(LogType::Error, "Failed to get module base");
		return 1;
	}

	log(LogType::Success, "Base Address = " + to_hex(base_address));

	auto readAddr = base_address + 0x1000;
	int value = Read<int>(readAddr);
	log(LogType::Success, "Read " + to_hex(readAddr) + " -> " + to_string(value));

	auto writeAddr = base_address + 0x2000;
	int writeValue = 1337;
	write<int>(writeAddr, writeValue);

	log(LogType::Success, "Write " + to_string(writeValue) + " -> " + to_hex(writeAddr));

	system("pause");
	return 0;
}