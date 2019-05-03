// DiningPhilosophers.cpp: определяет точку входа для консольного приложения.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <process.h>
#include <vector>
#include <thread>
#include<mutex>

using namespace std;

class Philosopher {
	int id;
	bool isDead;

	public: 

	time_t lastMeal;
		
	void eat() {

		if (lastMeal == NULL) lastMeal = clock();
		if (clock() - lastMeal >= 10000) {
			cout << "\n\n\n" << id << " died because of starving\n\n\n";
			isDead = true;
			return;
		}

		lastMeal = clock();

		cout << id << " started eating\n";
		int wait = rand() % ((id+1)*1000) + 1;
		Sleep(wait);
		cout << id << " finished eating\n";
	}

	bool isDeadNow() {
		return isDead;
	}

	Philosopher(int id) {
		this->id = id;
		lastMeal = NULL;
		isDead = false;
	}

	Philosopher() {
		lastMeal = NULL;
		isDead = false;
	}

};

struct Table {
	vector<Philosopher> philosophers;
	vector<mutex> forks;

	vector<int> ids;
};

struct Param {
	int id;
	Table *table;
};

Table* createTable(int numberOfPhilosophers) {

	Table *table = new Table;
	for (int i = 0; i < numberOfPhilosophers; i++) table->philosophers.push_back(*(new Philosopher(i)));
	table->forks = vector<mutex>(numberOfPhilosophers);

	return table;
}

unsigned int __stdcall simulate(void *param) {

	Param *params = (static_cast<Param *>(param));

	int id = params->id;
	int leftForkIndex = id, rightForkIndex = (id + 1 < params->table->forks.size()) ? id + 1 : 0;

	while (true) {

		pass:

		time_t started = clock();
		bool lockedLeft = false, lockedRight = false;

		while (clock() - started <= (id + 1) * 300) {
			if (params->table->forks[leftForkIndex].try_lock()) {
				lockedLeft = true;
				break;
			}
		}
		started = clock();
		while (clock() - started <= (id + 1) * 300) {
			if (params->table->forks[rightForkIndex].try_lock()) {
				lockedRight = true;
				break;
			}
		}

		if (lockedLeft && lockedRight) {
			params->table->philosophers[id].eat();
			params->table->ids.push_back(id);
		}

		if(lockedLeft) params->table->forks[leftForkIndex].unlock();
		if(lockedRight) params->table->forks[rightForkIndex].unlock();

		if (params->table->philosophers[id].isDeadNow()) return 1;
	}

	return 0;
}

int main()
{
	HANDLE *threads = new HANDLE[5];

	Table *table = createTable(5);

	for (int i = 0; i < 5; i ++) {
		Param *param = new Param;
		param->id = i;
		param->table = table;

		//Sleep(10);
		threads[i] = (HANDLE)_beginthreadex(NULL, 0, simulate, param, 0, NULL);
	}

	WaitForMultipleObjects(5, threads, true, INFINITE);

    return 0;
}

