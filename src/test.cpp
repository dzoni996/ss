/*
 * test.cpp
 *
 *  Created on: Aug 13, 2018
 *      Author: NIKOLA
 */
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <iomanip>
#include "declarations.h"
#include "assembler.h"
using namespace std;

int main(int argc, char *argv[]) {
	
	if (argc != 4) {
		cout << "No enough parameters!";
		return -1;
	}
	char *input = argv[1];
	char *output = argv[2];
	location start = stoi(argv[3]);

	Assembler *assembler = new Assembler();
	try {
		assembler->process(input, output, start);
	}
	catch (Myexc e) {
		cerr << "ERROR: " << e.getMsg() << endl;
		cout << endl << "Press any key to exit..." << endl;
		getchar();
		return -1;
	}
	catch (exception e) {
		cout << e.what();
		cout << endl << "Press any key to exit..." << endl;
		getchar();
		return -1;
	}

	delete assembler;
	
	cout << endl << "Press any key to exit..." << endl;
	getchar();
	return 0;
}

