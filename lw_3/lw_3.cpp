#include <windows.h>
#include <string>
#include <iostream>
#include <iterator>
#include <chrono>

#define OPERATIONS_COUNT 20

struct ThreadParams
{
	int threadIndex;
	std::chrono::steady_clock::time_point programStartTime;
};

DWORD WINAPI ThreadProc(CONST LPVOID lpParam)
{
	const ThreadParams threadParams = *static_cast<ThreadParams*>(lpParam);
	delete lpParam;

	for (int i = 1; i < OPERATIONS_COUNT; i++)
	{
		Sleep(50);
		auto stop = std::chrono::high_resolution_clock::now();
		auto duration = std::chrono::duration_cast<std::chrono::microseconds>(stop - threadParams.programStartTime);

		std::cout << std::to_string(threadParams.threadIndex + 1) + "/" + std::to_string(duration.count()) + "\n";
	}

	ExitThread(0);
}

int main(int argc, char* argv[])
{
	try
	{
		getchar();
		std::chrono::steady_clock::time_point programStartTime = std::chrono::high_resolution_clock::now();
		const int threadsCount = 2;
		HANDLE* handles = new HANDLE[threadsCount];
		for (int i = 0; i < threadsCount; i++) {
			ThreadParams* threadParams = new ThreadParams();
			threadParams->threadIndex = i;
			threadParams->programStartTime = programStartTime;
			handles[i] = CreateThread(NULL, 0, &ThreadProc, threadParams, CREATE_SUSPENDED, NULL);
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
