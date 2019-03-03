// Гистограммы.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <Windows.h>
#include <process.h>

// Эта структура описывает задание на построение гистограммы для потока
struct HistogrammingTask
{
	char *data;					// Исходные данные, по которым нужно считать гистограмму
	unsigned int from;			// Начальный индекс, с которого поток должен начать вычисления (включается в вычисления)
	unsigned int to;			// Конечный индекс, перед которым поток должен остановится (не включается в вычисления)
	long *histogramm;			// Результирующая гистограмма
};


// Функция потока для вычисления гистограммы
unsigned int __stdcall histogramming_thread(void *param)
{
	HistogrammingTask *task = static_cast<HistogrammingTask*>(param);

	// Основной цикл вычисления гистограммы.
	for (unsigned int i = task->from; i < task->to; i++)
		InterlockedIncrement(&task->histogramm[task->data[i]]);
	
	delete task;		// За удаление структуры-задания отвечает поток.
	return 0;
}

unsigned int __stdcall histogramming_thread_local(void *param)
{
	HistogrammingTask *task = static_cast<HistogrammingTask*>(param);

	long local_histogramm[20] = {};

	// Вычисляем локальную гистограмму.
	for (unsigned int i = task->from; i < task->to; i++) 
		local_histogramm[task->data[i]]++;

	// Добавляем к общей локальную с помощью InterlockedAdd
	for (int i = 0; i < 20; i++) 
		InterlockedAdd(&task->histogramm[i], local_histogramm[i]);

	delete task;		// За удаление структуры-задания отвечает поток.
	return 0;
}


int _tmain(int argc, _TCHAR* argv[])
{
	const unsigned int data_size = 1024 * 1024 * 50;		// Размер и массив исходных данных
	char *data = new char[data_size];

	const int num_bins = 20;			// Размер гистограммы (и одновременно максимальное значение данных)
	long histogramm[num_bins] = {};		// Гистограмма, = {} инициализирует массив нулями.


										// Заполнение исходных данных
	for (unsigned int i = 0; i < data_size; i++)
		data[i] = (char)(i % num_bins);

	// Число потоков
	unsigned int num_threads;

	printf_s("Enter number of threads: ");
	scanf_s("%u", &num_threads);

	HANDLE *threads = new HANDLE[num_threads];

	// Запуск потоков
	for (unsigned int thread = 0; thread < num_threads; thread++)
	{
		HistogrammingTask *task = new HistogrammingTask;

		task->data = data;
		task->histogramm = histogramm;
		task->from = data_size / num_threads * thread;
		if (thread != num_threads - 1)
			task->to = data_size / num_threads * (thread + 1);
		else
			task->to = data_size;

		threads[thread] = (HANDLE)_beginthreadex(NULL, 0, histogramming_thread_local, task, 0, NULL);
	}

	// Ожидание завершения вычислений
	WaitForMultipleObjects(num_threads, threads, true, INFINITE);

	// Печать результата
	for (int i = 0; i < num_bins; i++)
	{
		printf("%ld ", histogramm[i]);
	}

	// Освобождение памяти
	delete[] threads;
	delete[] data;

	return 0;
}

