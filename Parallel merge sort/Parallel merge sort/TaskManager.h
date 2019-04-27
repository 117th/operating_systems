#pragma once
#include<vector>
#include<iostream>
#include <Windows.h>

using namespace std;

struct Triple {
	int left;
	int right;
	int waitFor;
};

struct Task {
	int workerId;
	vector<Triple> steps;
};

void printTask(Task task);
void printVector(vector<bool> vect);
int getLength(vector<bool> vect);
int getNumWorkers(vector<Task> tasks);
vector<Task> tasking(int numWorkers, int numJobs);