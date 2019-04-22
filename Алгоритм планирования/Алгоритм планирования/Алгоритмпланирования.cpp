// Алгоритмпланирования.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include<vector>
#include<iostream>

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

void printTask(Task task){
	
	cout << "THREAD ID: " << task.workerId << "\n\n";

	for (int i = 0; i < task.steps.size(); i++) {
		cout << "STEP " << i << ":\n";
		cout << "left index = " << task.steps[i].left << "\n";
		cout << "right index = " << task.steps[i].right << "\n";
		if(task.steps[i].waitFor != -1) cout << "wait for " << task.steps[i].waitFor << " thread" << "\n\n";
		else {
			cout << "stop\n\n";
			cout << "\n=========================\n";
			return;
		}
	}

	cout << "\n=========================\n";
}

void printVector(vector<bool> vect) {
	for (bool element : vect) cout << element << " ";
}

int getLength(vector<bool> vect) {
	int sum = 0;
	for (bool element : vect) if (element) sum++;
	return sum;
}

vector<Task> tasking(int numWorkers, int numJobs) {
	
	vector<Task> tasks(numWorkers);
	vector<bool> jobs(numJobs, true);

	while (getLength(jobs) > 1) {
		int workerIndex = 0;
		for (int i = 0; i < numJobs; i += 1) {
			if ((i + 1 < numJobs) && jobs[i]) {
				Triple step;
				step.left = i;
				int j = -1;
				for (j = i + 1; j < numJobs; j++) {
					if (jobs[j]) break;
				}
				step.right = j;
				if (j != -1) jobs[j] = false;
				if ((i < numJobs / 2) && (((i + 2) / 2) % numWorkers > workerIndex)) step.waitFor = ((i + 2) / 2) % numWorkers;
				else step.waitFor = -1;
				tasks[workerIndex].workerId = workerIndex;
				tasks[workerIndex].steps.push_back(step);
				workerIndex++;
				if (workerIndex >= numWorkers) workerIndex = 0;
			}
		}
	}

	printVector(jobs);

	cout << "left " << getLength(jobs) << "\n";

	return tasks;
}


int main()
{
	vector<Task> tasks = tasking(4, 8);

	for (Task task : tasks) {
		printTask(task);
	}

    return 0;
}

