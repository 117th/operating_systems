// Parallelmergesort.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
#include <thread>
#include <ctime>

using namespace std;

vector<int> uniteSortedVectors(vector<int> left, vector<int> right);
int boolCount(vector<bool> vect);

class vectorHolder;
unsigned int __stdcall uniteInThreads(void *param);

//Структура, чтобы заворачивать параметры для потоков
struct ThreadParams {
	vectorHolder *holder;
	int id;

	int left;
	int right;
};

//разделим на n примерно равных массивов и будем их сортировать в n потоках

//Класс для хранения матрицы векторов, результата и ключей слияния
class vectorHolder {
	public:
		//сюда будем кидать отсортированные подмассивы
		vector<int> result;
		vector<vector<int>> matrix;
		vector<bool> isReady;

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
			//ключи для слияния
			//true - вектор надо слить к результату., false - вектор уже слит и больше не нужен
			isReady.resize(matrix.size(), true);

			//количество пар и потоков
			int numThreads = thread::hardware_concurrency() / 2;

			HANDLE *threads = new HANDLE[numThreads];
			int j = 0;

			//первый прогон
			for (int i = 0; i < numThreads*2; i += 2) {

				ThreadParams *param = new ThreadParams;

				param->holder = this;
				param->left = i;
				param->right = i + 1;

				threads[j++] = (HANDLE)_beginthreadex(NULL, 0, uniteInThreads, param, 0, NULL);
			}

			WaitForMultipleObjects(numThreads, threads, true, INFINITE);

			//остаточные слияния
			while (boolCount(isReady) > 1) {
				int begin = 0, end = 1;
				j = 0;

				//j - номер потока
				//ищем пару для участия в слиянии
				for (begin = 0; begin < isReady.size(); begin++) {
					for (end = begin + 1; end < isReady.size(); end++) if (isReady[begin] && isReady[end]) {
						ThreadParams *param = new ThreadParams;

						param->holder = this;
						param->left = begin;
						param->right = end;

						begin = end + 1;

						threads[j++] = (HANDLE)_beginthreadex(NULL, 0, uniteInThreads, param, 0, NULL);

						if (j >= numThreads) j = 0;

						break;
					}
				}

				WaitForMultipleObjects(numThreads, threads, true, INFINITE);
			}

			this->result = this->matrix[0];
		}

		void printResult() {
			for (int i = 0; i < result.size(); i++) cout << result[i] << " ";
		}
};

//многопоточное слияние
unsigned int __stdcall uniteInThreads(void *param) {
	ThreadParams *params = (static_cast<ThreadParams *>(param));

	//результат сливаем в левый вектор!
	params->holder->isReady[params->right] = false;

	params->holder->matrix[params->left] = uniteSortedVectors(params->holder->matrix[params->left], params->holder->matrix[params->right]);

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
	vector<int> input = { 11, 2, 31, 6, 7, 8, 3, 13, 100 };

	//vector<int> input;

	//for (int i = 0; i < 100000; i++) input.push_back(i % 20);

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

	holder->printResult();
	cout << "\n";

	begin_time = clock();

	vector<int> naiveSorted = mergeSort(input, 0, input.size() - 1);

	std::cout << "Time taken in single thread " << float(clock() - begin_time) / CLOCKS_PER_SEC << "\n";

	//for (int i = 0; i < naiveSorted.size(); i++) cout << naiveSorted[i] << " ";
	cout << "\n";

    return 0;
}

