// Умножениепоследовательностиматриц.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
#include <thread>
#include <ctime>

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

		for (int i = 0; i < left.matrix.size(); i++){
			if (left.matrix[i] != right.matrix[i]) return false;
		}

		return true;
	}
};

class Holder {

	public:
		vector<vector<Matrix>> holder;

		Holder(vector<Matrix> input, int numOfThreads) {

			int size = input.size() / numOfThreads;

			int j = 0;
			for (int i = 0; i < numOfThreads; i++) {
				vector<Matrix> temp;

				while (temp.size() < size) temp.push_back(input[j++]);

				holder.push_back(temp);
			}

			vector<Matrix> temp;
			while (j < input.size()) temp.push_back(input[j++]);
			if (!temp.empty()) holder.push_back(temp);
		}
		
};

struct Task {
	Holder *holderObj;
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

Matrix multiply(Matrix left, Matrix right) {
	Matrix *result = new Matrix();

	//n x k * k x m
	int n = left.matrix.size(), k = right.matrix.size(), m = 0;
	if ((n == 0) || (k == 0) || (k != left.matrix[0].size())) return *result;
	if (k != 0) m = right.matrix[0].size();

	for (int i = 0; i < n; i++) {
		vector<double> line;
		for (int j = 0; j < m; j++) {
			int element = 0;
			for (int r = 0; r < k; r++) element += left.matrix[i][r] * right.matrix[r][j];
			line.push_back(element);
		}
		result->matrix.push_back(line);
	}

	return *result;
}

Matrix multiplyVector(vector<Matrix> vect) {

	Matrix result;

	int len = vect.size();

	if (len == 0) return result;

	result = vect[0];

	for (int i = 1; i < len; i++) {
		result = multiply(result, vect[i]);
	}

	return result;
}

vector<Matrix> parseHolder(Holder holderObj) {

	vector<vector<Matrix>> holder = holderObj.holder;

	vector<Matrix> result;

	for (int i = 0; i < holder.size(); i++) result.push_back(holder[i][0]);

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

	Task *params = (static_cast<Task *>(param));

	int id = params->id;

	vector<Matrix> result;

	vector<Matrix> part = params->holderObj->holder[id];

	result.push_back( multiplyVector(part) );

	params->holderObj->holder[id] = result;

	return 0;
}

int main()
{
	Matrix left;
	left.matrix = { {1, 0}, {0 , 1} };

	Matrix right;
	right.matrix = { {1, 1}, {3, 2} };

	Matrix third;
	third.matrix = { {2, 1}, {5, 2} };

	vector<Matrix> matrixVector;

	matrixVector.push_back(left);
	matrixVector.push_back(right);
	matrixVector.push_back(third);

	vector<Matrix> randomVector = getRandomSequence(9);

	cout << "Vector function: \n";
	clock_t begin_time = clock();
	Matrix singleMatrix = multiplyVector(randomVector);
	cout << "Time taken in single thread function " << float(clock() - begin_time) / CLOCKS_PER_SEC << "\n";
	singleMatrix.printMatrix();

	unsigned int numCores = thread::hardware_concurrency();

	cout << "Number of cores: " << numCores << "\n";

	if (randomVector.size() < numCores) numCores = randomVector.size();

	HANDLE *threads = new HANDLE[numCores];

	Holder *holder = new Holder(randomVector, numCores);

	begin_time = clock();
	for (int i = 0; i < numCores; i++) {

		Task *param = new Task;

		param->holderObj = holder;
		param->id = i;

		threads[i] = (HANDLE)_beginthreadex(NULL, 0, multiplyVectorThreads, param, 0, NULL);
	}

	WaitForMultipleObjects(numCores, threads, true, INFINITE);
	
	Matrix threadMatrix = multiplyVector(parseHolder(*holder));
	cout << "Time taken with all cores used: " << float(clock() - begin_time) / CLOCKS_PER_SEC << "\n";

	threadMatrix.printMatrix();

	if (singleMatrix == threadMatrix) cout << "Equal results \n";
	else cout << "Different results \n";

    return 0;
}

