#include <Windows.h>
#include <tchar.h>

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _UNICODE
#define _UNICODE
#endif

#define BUF_SIZE 10

struct circBuf {
	int buf[BUF_SIZE];
	int iW;
	int iR;
};

int __cdecl _tmain(int argc, TCHAR *argv[])
{
	// Описание процесса "Производитель"
	STARTUPINFO siProducer = { sizeof(siProducer) };
	ZeroMemory(&siProducer, sizeof(siProducer));
	siProducer.cb = sizeof(siProducer);

	PROCESS_INFORMATION piProducer;
	ZeroMemory(&piProducer, sizeof(piProducer));

	// Описание процесса "Потребитель"
	STARTUPINFO siConsumer = { sizeof(siConsumer) };
	ZeroMemory(&siConsumer, sizeof(siConsumer));
	siConsumer.cb = sizeof(siConsumer);

	PROCESS_INFORMATION piConsumer;
	ZeroMemory(&piConsumer, sizeof(piConsumer));

	// Получение ID главного процесса
	int processID = GetCurrentProcessId();
	TCHAR bufPID[20];
	_itot_s(processID, bufPID, sizeof(bufPID) / sizeof(TCHAR), 10);

	// Создание проецируемого файла (разделяемая память между процессами)
	HANDLE hMapFile;

	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;

	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		&sa,
		PAGE_READWRITE,
		0,
		256,
		NULL);
	if (hMapFile == NULL)
		return 1;

	circBuf * cBuf = 0;
	cBuf = (circBuf*)MapViewOfFile(hMapFile,
		FILE_MAP_ALL_ACCESS,
		0,
		0,
		256);

	if (cBuf == NULL)
	{
		CloseHandle(hMapFile);
		return 2;
	}

	TCHAR tCBuf[10];
	_itot((int)hMapFile, tCBuf, 10);

	// Создание семафора исключающего доступа
	HANDLE hSemExc = CreateSemaphore(&sa, 1, 1, NULL);
	if (hSemExc == NULL)
	{
		UnmapViewOfFile((LPCVOID)cBuf);
		CloseHandle(hMapFile);
		return 3;
	}
	TCHAR  tSemExc[10];
	_itot((int)hSemExc, tSemExc, 10);

	// Создание семафора буфер пуст
	HANDLE hSemFree = CreateSemaphore(&sa, 10, 10, NULL);
	if (hSemFree == NULL)
	{
		UnmapViewOfFile((LPCVOID)cBuf);
		CloseHandle(hMapFile);
		CloseHandle(hSemExc);
		return 4;
	}
	TCHAR  tSemFree[10];
	_itot((int)hSemFree, tSemFree, 10);

	// Создание семафора буфер полон
	HANDLE hSemBusy = CreateSemaphore(&sa, 0, 10, NULL);
	if (hSemBusy == NULL)
	{
		UnmapViewOfFile((LPCVOID)cBuf);
		CloseHandle(hMapFile);
		CloseHandle(hSemExc);
		CloseHandle(hSemFree);
		return 5;
	}
	TCHAR  tSemBusy[10];
	_itot((int)hSemBusy, tSemBusy, 10);

	// Создание процесса "Производитель"
	TCHAR bufProducer[1000] = TEXT("Producer.exe \0");
	TCHAR szCommandLineProducer[1000];
	_tcscat(bufProducer, bufPID);
	_tcscat(bufProducer, TEXT(" "));
	_tcscat(bufProducer, tCBuf);
	_tcscat(bufProducer, TEXT(" "));
	_tcscat(bufProducer, tSemExc);
	_tcscat(bufProducer, TEXT(" "));
	_tcscat(bufProducer, tSemFree);
	_tcscat(bufProducer, TEXT(" "));
	_tcscat(bufProducer, tSemBusy);
	wcscpy(szCommandLineProducer, bufProducer);
	if (!CreateProcess(0, szCommandLineProducer, 0, 0, TRUE, CREATE_SUSPENDED, 0, 0, &siProducer, &piProducer))
		ExitProcess(1);

	// Создание процесса "Потребитель"
	TCHAR bufConsumer[1000] = TEXT("Consumer.exe \0");
	TCHAR szCommandLineConsumer[1000];
	_tcscat(bufConsumer, bufPID);
	_tcscat(bufConsumer, TEXT(" "));
	_tcscat(bufConsumer, tCBuf);
	_tcscat(bufConsumer, TEXT(" "));
	_tcscat(bufConsumer, tSemExc);
	_tcscat(bufConsumer, TEXT(" "));
	_tcscat(bufConsumer, tSemFree);
	_tcscat(bufConsumer, TEXT(" "));
	_tcscat(bufConsumer, tSemBusy);
	wcscpy(szCommandLineConsumer, bufConsumer);
	if (!CreateProcess(0, szCommandLineConsumer, 0, 0, TRUE, CREATE_NEW_CONSOLE | CREATE_NEW_CONSOLE, 0, 0, &siConsumer, &piConsumer))
		ExitProcess(1);

	// Запуск работы процессов
	ResumeThread(piProducer.hThread);
	ResumeThread(piConsumer.hThread);
	// Завершение работы программы
	CloseHandle(piProducer.hThread);
	CloseHandle(piConsumer.hThread);
	CloseHandle(hSemExc);
	CloseHandle(hSemFree);
	CloseHandle(hSemBusy);
	CloseHandle(hMapFile);
	return 0;
}
