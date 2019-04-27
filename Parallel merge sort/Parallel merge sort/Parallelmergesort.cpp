// Parallelmergesort.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
#include <thread>
#include <ctime>

#include"TaskManager.h"

using namespace std;

vector<int> uniteSortedVectors(vector<int> left, vector<int> right);
int boolCount(vector<bool> vect);

class vectorHolder;
unsigned int __stdcall uniteInThreads(void *param);

//Структура, чтобы заворачивать параметры для потоков
struct ThreadParams {
	vectorHolder *holder;
	int id;

	Task task;
};

//разделим на n примерно равных массивов и будем их сортировать в n потоках

//Класс для хранения матрицы векторов, результата и ключей слияния
class vectorHolder {
	public:
		//сюда будем кидать отсортированные подмассивы
		vector<int> result;
		vector<vector<int>> matrix;
		vector<bool> isReady;
		vector<vector<HANDLE>> events;

		void printMatrix() {
			for (int i = 0; i < matrix.size(); i++) {
				for (int j = 0; j < matrix[i].size(); j++) cout << matrix[i][j] << " ";
				cout << "\n";
			}
		}

		void setMarix(vector<int> vect, unsigned int numCores) {

			unsigned int size = vect.size() / numCores;

			//закидываем в матрицу вектора, которые в каждом потоке будем сортировать отдельно
			int j = 0;
			for (int i = 0; i < numCores - 1; i++) {
				vector<int> temp;

				while (temp.size() < size) temp.push_back(vect[j++]);

				matrix.push_back(temp);
			}

			//добросили остаток
			vector<int> temp;
			while (j < vect.size()) temp.push_back(vect[j++]);
			if (!temp.empty()) matrix.push_back(temp);
		}

		void uniteMatrix() {
			for (int j = 0; j < matrix.size(); j++) {
				result = uniteSortedVectors(result, matrix[j]);
			}
		}

		void uniteMatrixInThreads() {
			vector<Task> tasks = tasking(matrix.size(), matrix.size());

			/*for (Task task : tasks) {
				printTask(task);
			}

			printMatrix();*/

			int numberOfWorkers = getNumWorkers(tasks);

			HANDLE *threads = new HANDLE[numberOfWorkers];

			for (int i = 0; i < numberOfWorkers; i++) {
				vector<HANDLE> temp(matrix.size());
				events.push_back(temp);	
				ThreadParams *param = new ThreadParams;
				param->holder = this;
				param->id = i;
				param->task = tasks[i];
				threads[i] = (HANDLE)_beginthreadex(NULL, 0, uniteInThreads, param, 0, NULL);
			}

			WaitForMultipleObjects(numberOfWorkers, threads, true, INFINITE);

			result = matrix[0];
		}

		void printResult() {
			for (int i = 0; i < result.size(); i++) cout << result[i] << " ";
		}
};

//многопоточное слияние
unsigned int __stdcall uniteInThreads(void *param) {
	ThreadParams *params = (static_cast<ThreadParams *>(param));

	vector<Triple> steps = params->task.steps;

	int stepNumber = 0;
	for (Triple step : steps) {
		params->holder->matrix[step.left] = uniteSortedVectors(params->holder->matrix[step.left], params->holder->matrix[step.right]);
		SetEvent(params->holder->events[params->id][stepNumber++]);
		//cout << "thread " << params->id << " waiting for " << step.waitFor << " " << step.left << "x" << step.right << " in step  " << stepNumber << "\n";
		Sleep(1);
		if (step.waitFor == -1) return 0;
		WaitForSingleObject(params->holder->events[step.waitFor][stepNumber - 1], INFINITE);
	}

	return 0;
}

//объединение двух отсортированных векторов
vector<int> uniteSortedVectors(vector<int> left, vector<int> right) {

	vector<int> result;

	unsigned int i = 0, j = 0;

	while ((i < left.size()) && (j < right.size())) {
		if (left[i] < right[j]) result.push_back(left[i++]);
		else result.push_back(right[j++]);
	}

	while ((i < left.size()) && (j == right.size())) result.push_back(left[i++]);
	while ((i == left.size()) && (j < right.size())) result.push_back(right[j++]);

	return result;
}

vector<int> mergeSort(vector<int> vect, unsigned int left, unsigned int right) {
	
	if (left < right) {
		unsigned int middle = (left + right) / 2;

		//если не один элемент, то сортируем отдельно левую и правую части и сливаем
		vector<int> sortLeft = mergeSort(vect, left, middle);
		vector<int> sortRight = mergeSort(vect, middle + 1, right);

		vector<int> result = uniteSortedVectors(sortLeft, sortRight);

		return result;
	}
	else if (left == right) {
		vector<int> result;

		result.push_back(vect[left]);
		return result;
	}

}

unsigned int __stdcall mergeSortThread(void *param) {
	ThreadParams *params = (static_cast<ThreadParams *>(param));

	vector<int> copy = params->holder->matrix[params->id];
	vector<int> sortedPart = mergeSort(copy, 0, params->holder->matrix[params->id].size() - 1);

	params->holder->matrix[params->id] = sortedPart;

	return 0;
}

int boolCount(vector<bool> vect) {

	int count = 0;

	for (int i = 0; i < vect.size(); i++) if (vect[i]) count++;

	return count;
}

int main()
{
	//vector<int> input = { 11, 2, 31, 6, 7, 8, 3, 13, 100 };

	vector<int> input;

	for (int i = 0; i < 100000; i++) input.push_back(i % 20);

	unsigned int numCores = thread::hardware_concurrency();

	cout << "Number of cores: " << numCores << "\n";

	HANDLE *threads = new HANDLE[numCores];

	vectorHolder *holder = new vectorHolder();

	holder->setMarix(input, numCores);

	clock_t begin_time = clock();

	for (int i = 0; i < numCores; i++) {

		ThreadParams *param = new ThreadParams;

		param->holder = holder;
		param->id = i;

		threads[i] = (HANDLE)_beginthreadex(NULL, 0, mergeSortThread, param, 0, NULL);
	}

	WaitForMultipleObjects(numCores, threads, true, INFINITE);

	//теперь надо слить вектора

	//holder->uniteMatrix();
	holder->uniteMatrixInThreads();

	std::cout << "Time taken in threads " << float(clock() - begin_time) / CLOCKS_PER_SEC << "\n";

	//holder->printResult();
	cout << "\n";

	begin_time = clock();

	vector<int> naiveSorted = mergeSort(input, 0, input.size() - 1);

	std::cout << "Time taken in single thread " << float(clock() - begin_time) / CLOCKS_PER_SEC << "\n";

	if (holder->result != naiveSorted) cout << "DIFFERENT VECTORS\n";
	else cout << "SAME VECTORS\n";

	//for (int i = 0; i < naiveSorted.size(); i++) cout << naiveSorted[i] << " ";
	cout << "\n";

    return 0;
}

