#include <Windows.h>
#include <tchar.h>
#include <conio.h>

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
	DWORD PID = _ttoi(argv[1]);
	HANDLE h1 = OpenProcess(SYNCHRONIZE, 0, PID);

	// Получение глобального буфера
	HANDLE hMapFile = (HANDLE)_ttoi(argv[2]);

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

	// Получение семафора взаимного исключения
	HANDLE hSemExc = (HANDLE)_ttoi(argv[3]);
	// Получение семафора буфер пуст
	HANDLE hSemFree = (HANDLE)_ttoi(argv[4]);
	// Получение семафора буфер полон
	HANDLE hSemBusy = (HANDLE)_ttoi(argv[5]);

	// Работа производителя
	_tprintf(TEXT("#### PRODUCER ####\n"));

	for (int i = 0; i < 15; i++)
	{
		WaitForSingleObject(hSemFree, INFINITE);
		WaitForSingleObject(hSemExc, INFINITE);

		if (cBuf->iW >= BUF_SIZE)
			cBuf->iW = 0;
		cBuf->buf[cBuf->iW] = i + 1;
		_tprintf(TEXT(" # Write in buffer: %d\n Pointer: %d\n"), cBuf->buf[cBuf->iW], cBuf->iW);
		_tprintf(TEXT(" Buffer:\n | "));
		for (int j = 0; j < BUF_SIZE; j++)
			_tprintf(TEXT("%d | "), cBuf->buf[j]);
		_tprintf(TEXT("\n\n"));
		cBuf->iW++;

		ReleaseSemaphore(hSemExc, 1, NULL);
		ReleaseSemaphore(hSemBusy, 1, NULL);

		Sleep(1000);
	}
	
	_tprintf(TEXT("#### END ####\n(Press any bytton to exit)"));
	getch();
	// Закрытие приложения
	UnmapViewOfFile((LPCVOID)cBuf);
	CloseHandle(hMapFile);
	CloseHandle(hSemFree);
	CloseHandle(hSemBusy);
	CloseHandle(hMapFile);
	return 0;
}
