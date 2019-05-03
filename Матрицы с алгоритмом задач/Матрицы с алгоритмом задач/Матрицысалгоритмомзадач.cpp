// Матрицысалгоритмомзадач.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
#include <thread>
#include <ctime>

#include "TaskManager.h"

using namespace std;

class Matrix {

public:
	vector<vector<double>> matrix;

	void printMatrix();

	Matrix(int n, int k) {

		srand(time(0));

		for (int i = 0; i < n; i++) {
			vector<double> temp;
			for (int j = 0; j < k; j++) temp.push_back(rand() % 10);
			matrix.push_back(temp);
		}
	}

	Matrix() {

	}

	friend const bool operator==(const Matrix &left, const Matrix &right) {
		if (left.matrix.empty() && right.matrix.empty()) return true;
		if (left.matrix.size() != right.matrix.size()) return false;

		for (int i = 0; i < left.matrix.size(); i++) {
			if (left.matrix[i] != right.matrix[i]) return false;
		}

		return true;
	}

	friend const Matrix operator*(const Matrix &left, const Matrix &right) {
		Matrix result;

		//n x k * k x m
		int n = left.matrix.size(), k = right.matrix.size(), m = 0;
		if ((n == 0) || (k == 0) || (k != left.matrix[0].size())) throw invalid_argument("not matching size");
		if (k != 0) m = right.matrix[0].size();

		for (int i = 0; i < n; i++) {
			vector<double> line;
			for (int j = 0; j < m; j++) {
				int element = 0;
				for (int r = 0; r < k; r++) element += left.matrix[i][r] * right.matrix[r][j];
				line.push_back(element);
			}
			result.matrix.push_back(line);
		}

		return result;
	}
};

class Holder {

public:
	vector<Matrix> holder;
	vector<vector<HANDLE>> events;

	Holder(vector<Matrix> vect) {
		holder = vect;

		int length = vect.size();

		for (int i = 0; i < length + 1; i++) {
			vector<HANDLE> temp(length);
			events.push_back(temp);
		}
	}
};

struct ThreadParam {
	Holder *holderObj;
	Task task;
	int id;
};

void Matrix::printMatrix() {

	int size = matrix.size();

	for (int i = 0; i < size; i++) {
		int length = matrix[i].size();
		for (int j = 0; j < length; j++) cout << matrix[i][j] << " ";
		cout << "\n";
	}
}

Matrix multiplyMatrixVector(vector<Matrix> vect) {

	Matrix result;

	int len = vect.size();

	if (len == 0) return result;

	result = vect[0];

	for (int i = 1; i < len; i++) {
		result = result*vect[i];
	}

	return result;
}

vector<Matrix> getRandomSequence(int size) {

	vector<Matrix> sequence;

	srand(time(0));
	int prev = rand() % 10 + 1, next = rand() % 10 + 1;

	//prev x next
	for (int i = 0; i < size; i++) {
		Matrix *temp = new Matrix(prev, next);
		sequence.push_back(*temp);

		prev = next;
		next = rand() % 10 + 1;
	}

	return sequence;
}

unsigned int __stdcall multiplyVectorThreads(void *param) {

	ThreadParam *params = (static_cast<ThreadParam *>(param));

	vector<Triple> steps = params->task.steps;

	int length = steps.size();

	for (int i = 0; i < length; i++) {
		params->holderObj->holder[steps[i].left] = params->holderObj->holder[steps[i].left] * params->holderObj->holder[steps[i].right];
		SetEvent(params->holderObj->events[params->id][i]);
		if (steps[i].waitFor == -1) return 0;
		cout << "thread " << params->id << " step " << i << " waiting for " << steps[i].waitFor << "\n";
		//Sleep(1);
		WaitForSingleObject(params->holderObj->events[steps[i].waitFor][i], INFINITE);
	}

	//когда поток заканчивает работу, забиваем ВСЕ его события
	for (int i = 0; i < params->holderObj->events[params->id].size(); i++) SetEvent(params->holderObj->events[params->id][i]);

	return 0;
}

int main()
{
	Matrix left;
	left.matrix = { { 1, 0 },{ 0 , 1 } };

	Matrix right;
	right.matrix = { { 1, 1 },{ 3, 2 } };

	Matrix third;
	third.matrix = { { 2, 1 },{ 5, 2 } };

	Matrix four;
	four.matrix = { { 1, 1 },{ 1, 2 } };

	vector<Matrix> matrixVector;

	matrixVector.push_back(left);
	matrixVector.push_back(right);
	matrixVector.push_back(third);
	matrixVector.push_back(four);

	int amountOfMatrix = 8;

	vector<Matrix> randomVector = getRandomSequence(amountOfMatrix);

	matrixVector = randomVector;

	clock_t begin_time = clock();
	Matrix singleMatrix = multiplyMatrixVector(matrixVector);
	cout << "Time taken in single thread function " << float(clock() - begin_time) / CLOCKS_PER_SEC << "\n\n";
	singleMatrix.printMatrix();

	unsigned int numCores = thread::hardware_concurrency();

	cout << "Number of cores: " << numCores << "\n";

	if (randomVector.size() < numCores) numCores = randomVector.size();

	HANDLE *threads = new HANDLE[numCores];

	Holder *holder = new Holder(matrixVector);

	vector<Task> tasks = tasking(numCores, amountOfMatrix);

	for (Task task : tasks) printTask(task);

	begin_time = clock();

	for (int i = 0; i < numCores; i++) {

		ThreadParam *param = new ThreadParam;
		param->holderObj = holder;
		param->id = i;
		param->task = tasks[i];

		threads[i] = (HANDLE)_beginthreadex(NULL, 0, multiplyVectorThreads, param, 0, NULL);
	}

	WaitForMultipleObjects(numCores, threads, true, INFINITE);

	Matrix threadMatrix = holder->holder[0];
	cout << "Time taken with all cores used: " << float(clock() - begin_time) / CLOCKS_PER_SEC << "\n";

	threadMatrix.printMatrix();

	if (singleMatrix == threadMatrix) cout << "Equal results \n";
	else cout << "Different results \n";

	return 0;
}