#include <iostream>
#include <random>
#include "ThreadPool.h"
#include <conio.h>
#include <windows.h>

#define BUFFER_SIZE 10
#define INPUT_RECORD_BUFFER 128

using namespace std;

void simulate_hard_computation();
void fillBuffer(int& num, int& currentFillIndex, int* buffer);
void readBuffer(int* buffer, int& currentFillIndex);
void printBuffer(int* buffer);

void handleConsole();
void KeyEventProc(KEY_EVENT_RECORD ker);
bool setConsoleParams();

bool isExitFlagSet = false;
HANDLE hStdin;
HANDLE hStdout;
DWORD fdwSaveOldMode;
DWORD fdwMode;
INPUT_RECORD irInBuf[INPUT_RECORD_BUFFER];
CONSOLE_SCREEN_BUFFER_INFO start_attribute;

ThreadPool fillThreads(1);
ThreadPool readThreads(1);
ThreadPool keyboardThread(1);
ThreadPool consoleLog(1);

int fillIndex = 0;
int readIndex = 0;

enum keyButtons {
	KEY_W = 87,
	KEY_S = 83,
	ARROW_UP = 38,
	ARROW_DOWN = 40,
	ESC = 27,
	KEY_I = 73
};

int main()
{
	fillThreads.init();
	readThreads.init();
	keyboardThread.init();
	//consoleLog.init();

	srand(time(NULL));

	int num = 0;
	int currentFillIndex = 0;
	int buffer[BUFFER_SIZE];

	for (int i = 0; i < BUFFER_SIZE; i++) {
		buffer[i] = 0;
	}

	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	isExitFlagSet = false;
	fdwMode = 0;

	setConsoleParams();
	keyboardThread.submit(handleConsole);

	for (int i = 0; i < 512; i++) {
		num = i;
		if (currentFillIndex < BUFFER_SIZE) {
			fillThreads.submit(fillBuffer, num, currentFillIndex, buffer);
			readThreads.submit(readBuffer, buffer, currentFillIndex);
			currentFillIndex++;
		} else {
			currentFillIndex = 0;
		}
	}

	//consoleLog.submit(printBuffer, buffer);
	printBuffer(buffer);
	fillThreads.shutdown();
	readThreads.shutdown();
	//consoleLog.shutdown();
	keyboardThread.shutdown();
	return 0;
}

void printBuffer(int* buffer) {
	while (!isExitFlagSet) {
		this_thread::sleep_for(chrono::milliseconds(500));
		for (int i = 0; i < BUFFER_SIZE; i++) {
			cout << buffer[i] << "\t";
		}
		cout << "\n";
	}
}

void simulate_hard_computation() {
	this_thread::sleep_for(chrono::milliseconds(2000));
}

void fillBuffer(int& num, int& currentFillIndex, int* buffer) {
	simulate_hard_computation();
	buffer[currentFillIndex] = num;
}

void readBuffer(int* buffer, int& currentReadIndex) {
	simulate_hard_computation();
	buffer[currentReadIndex] = 0;
	for (int i = 0; i < BUFFER_SIZE; i++) {
		cout << buffer[i] << "\t";
	}
	cout << "\n";
}

void handleConsole()
{
	DWORD cNumRead = 0;
	while (!isExitFlagSet) {
		if (!ReadConsoleInput(hStdin, irInBuf, 128, &cNumRead)) {
			cout << "Error: ReadConsoleInput\n";
		}

		for (int i = 0; i < cNumRead; i++)
		{
			switch (irInBuf[i].EventType)
			{
			case KEY_EVENT: // keyboard input
				KeyEventProc(irInBuf[i].Event.KeyEvent);
				break;
			default:
				break;
			}
		}
		cNumRead = 0;
	}
	cout << "Keyboard handler has closed!" << endl;
}

void KeyEventProc(KEY_EVENT_RECORD ker)
{
	if (ker.bKeyDown) {
		switch (ker.wVirtualKeyCode)
		{
		case KEY_W:
			readThreads.addThread();
			break;
		case KEY_S:
			//fillThreads.removeThread();
			readThreads.setDeleteFlag(true);
			break;
		case ARROW_UP:
			fillThreads.addThread();
			break;
		case ARROW_DOWN:
			//fillThreads.removeThread();
			fillThreads.setDeleteFlag(true);
			break;
		case KEY_I: {
			cout << "\n===========FILL_THREAD_POOL_STATUS============\n";
			cout << "Threads are working: " << fillThreads.getThreadsCount() << "\n";
			cout << "Tasks: " << fillThreads.getTasksCount() << "\n";
			cout << fillIndex << "\n";
			cout << "===========READ_THREAD_POOL_STATUS============\n";
			cout << "Threads are working: " << readThreads.getThreadsCount() << "\n";
			cout << "Tasks: " << readThreads.getTasksCount() << "\n";
			cout << readIndex << "\n";
			if (fillThreads.getCurrentDeleteThreadId() >= 0) {
				cout << "Current delete thread id: " << fillThreads.getCurrentDeleteThreadId() << "\n";
			}
			cout << "==============================================\n\n";
		}
			break;
		case ESC:
			isExitFlagSet = true;
			break;
		default:
			break;
		}
	}
}

bool setConsoleParams()
{
	bool errorFlag = false;

	if (hStdin == INVALID_HANDLE_VALUE) {
		cout << "Error: GetStdHandle\n";
		errorFlag = true;
	}

	if (!GetConsoleMode(hStdin, &fdwSaveOldMode)) {
		cout << "Error: GetConsoleMode\n";
		errorFlag = true;
	}

	fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS | ENABLE_PROCESSED_INPUT;
	if (!SetConsoleMode(hStdin, fdwMode)) {
		cout << "Error: SetConsoleMode\n";
		errorFlag = true;
	
	}
	return errorFlag;
}
