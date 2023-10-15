#include <windows.h>
#include <string>
#include <iostream>
#include <iterator>

#define MAX_THREADS_COUNT 10

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	const int threadIndex = *static_cast<int*>(lpParam);
	delete lpParam;

	std::cout << "Thread number" + std::to_string(threadIndex) + " is executing\n";

	ExitThread(0);
}

int ProcessArgv(int argc, char* argv[])
{
	if (argc - 1 != 1)
	{
		throw std::length_error("There should be one argument showing number of threads");
	}

	const int threadsCount = std::stoi(argv[1]);
	if (threadsCount < 1 || threadsCount > MAX_THREADS_COUNT)
	{
		const std::string errorMsg = "Threads count should be less than " +
			std::to_string(MAX_THREADS_COUNT + 1) +
			" and more than 0";
		throw std::invalid_argument(errorMsg);
	}

	return threadsCount;
}

int main(int argc, char* argv[])
{
	try
	{
		const int threadsCount = ProcessArgv(argc, argv);
		HANDLE* handles = new HANDLE[threadsCount];
		for (int i = 0; i < threadsCount; i++)
		{
			int* threadIndex = new int(i);
			handles[i] = CreateThread(NULL, 0, &ThreadProc, threadIndex, CREATE_SUSPENDED, NULL);
		}

		for (int i = 0; i < threadsCount; i++) {
			ResumeThread(handles[i]);
		}

		WaitForMultipleObjects(threadsCount, handles, true, INFINITE);
	}
	catch (std::exception& e)
	{
		std::cout << e.what() << std::endl;

		return 1;
	}

	return 0;
}
