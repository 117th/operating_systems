// Инкремент.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <Windows.h>
#include <stdio.h>
#include <process.h>
#include <iostream>

using namespace std;

long p = 0;

unsigned int __stdcall unsafeIncr(void *param) {
	
	for (int i = 0; i < 5000; i++) p++;

	return 0;
}

unsigned int __stdcall safeIncr(void *param) {

	for (int i = 0; i < 5000; i++) InterlockedIncrement(&p);

	return 0;
}

int main()
{

	unsigned int id;

	HANDLE threads[5];

	for (int i = 0; i < 5; i++) 
		threads[i] = (HANDLE)_beginthreadex(NULL, 0, unsafeIncr, (void*)i, 0, &id);
	
	WaitForMultipleObjects(5, threads, true, INFINITE);

	cout << "Res: " << p << "\n";

    return 0;
}

