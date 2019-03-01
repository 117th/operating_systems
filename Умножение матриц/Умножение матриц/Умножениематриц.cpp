// Умножениематриц.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <process.h>

using namespace std;

struct ThreadParams;

class Matrix {
	public:
		int **firstMatrix;
		int **secondMatrix;
		int **result; //n x k
		int n;
		int m;
		int k;

		unsigned int getSingleLine(int i){
			cout << "computing " << i << " line \n";

			for (int j = 0; j < k; j++) {
				result[i][j] = 0;
				for(int r = 0; r < m; r++) result[i][j] += firstMatrix[i][r] * secondMatrix[r][j];
			}

			return 0;
		}

		Matrix() {}
		Matrix(int n, int m, int k) {
			firstMatrix = new int*[n];
			for (int i = 0; i < n; i++) firstMatrix[i] = new int[m];
			secondMatrix = new int*[m];
			for (int i = 0; i < k; i++) secondMatrix[i] = new int[k];
			result = new int*[n];
			for (int i = 0; i < n; i++) result[i] = new int[k];
			this->k = k;
			this->n = n;
			this->m = m;
		}

};

struct ThreadParams {
	Matrix *obj;
	int line;
};

//заворачиваем нужные аргументы в структуру
ThreadParams wrapParams(Matrix *obj, int line) {
	ThreadParams params;
	params.line = line;
	params.obj = obj;

	return params;
}

Matrix naiveMult(int **firstMatrix, int **secondMatrix, int n, int m, int k) {
	Matrix result = Matrix();

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) cout << firstMatrix[i][j] << ' ';
		cout << "\n";
	}

	cout << "\n";

	for (int i = 0; i < m; i++) {
		for (int j = 0; j < k; j++) cout << secondMatrix[i][j] << ' ';
		cout << "\n";
	}

	int **resultMatrix = new int*[n];
	for (int i = 0; i < n; i++) resultMatrix[i] = new int[k];

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < k; j++) {
			resultMatrix[i][j] = 0;
			for (int r = 0; r < m; r++) resultMatrix[i][j] += firstMatrix[i][r] * secondMatrix[r][j];
		}
	}

	result.result = resultMatrix;

	return result;
}

//разворачиваем параметры из структуры и вызываем функцию в нужном экземпляре класса
unsigned int __stdcall getSingleLineWrapper(void *p) {
	ThreadParams params = *(static_cast<ThreadParams *>(p));

	params.obj->getSingleLine(params.line);

	return 0;
}

int main()
{
	//первая матрица n x m, вторая m x k

	int n = 3;
	int m = 3;
	int k = 3;

	int **firstMatrix = new int*[n];
	for (int i = 0; i < n; i++) firstMatrix[i] = new int[m];

	int **secondMatrix = new int*[m];
	for (int i = 0; i < k; i++) secondMatrix[i] = new int[k];

	for (int i = 0; i < n; i++) for (int j = 0; j < m; j++) firstMatrix[i][j] = i + j;
	for (int i = 0; i < m; i++) for (int j = 0; j < k; j++) secondMatrix[i][j] = i + 2*j;

	int **result = naiveMult(firstMatrix, secondMatrix, n, m, k).result;

	cout << "NAIVE SOLUTION:\n";
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < k; j++) cout << result[i][j] << ' ';
		cout << "\n";
	}

	Matrix *threadResult = new Matrix(n, m, k);
	threadResult->firstMatrix = firstMatrix;
	threadResult->secondMatrix = secondMatrix;

	HANDLE *threads = new HANDLE[n];

	unsigned int id;
	for (int i = 0; i < n; i++) {
		Sleep(10);
		threads[i] = (HANDLE)_beginthreadex(NULL, 0, getSingleLineWrapper, (void*)&wrapParams(threadResult, i), 0, &id);
	}

	WaitForMultipleObjects(n, threads, true, INFINITE);

	//можно было бы все аргументы сворачивать, но смысл?
	//раз уж храним результат, то будем там же хранить аргументы
	cout << "N-THREAD SOLUTION:\n";
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < k; j++) cout << threadResult->result[i][j] << ' ';
		cout << "\n";
	}

    return 0;
}

