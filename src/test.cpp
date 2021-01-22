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
	//cout << hex << assembler->generateCode(2, 1, 9, 10);
	delete assembler;
	
	cout << endl << "Press any key to exit..." << endl;
	getchar();
	return 0;
}

// --------------------------------------------------------

int main1() {
	cerr << "Hello!";

	class Myexc : public exception {
		string msg;
	public:
		Myexc(string m) : msg(m) {}
		string getMsg() const { return msg; }
	};

	try {
		for (int i = 0; i < 10; i++) {
			if (i == 9) throw Myexc("greska");
			else cout << i << endl;
		}
			
	}
	catch (Myexc e) {
		cout << e.getMsg();
	}
	cout << endl << "--------------" << endl;

	string str1 = "nesto";
	string str2 = "nesto";
	cout << str1.compare(str2);

	cout << endl << "--------------" << endl;

	regex novi1("\\s*(.char\\s+[a-z0-9,\\s]+)\\s*");
	string line1 = " .char 2, 5 ";
	if (regex_match(line1, novi1)) {
		regex novi2("\\s*.char\\s+([a-z0-9,\\s]+)\\s*");
		smatch m;
		if (regex_search(line1, m, novi2)) {
			

			for (auto x : m) cout << x << "\n";
			cout << "\nUspesno\n";
		}
		cout << "Radi\n";
	}

	cout << endl << "--------------" << endl;
	string line2 = " addgt &r1, $r1 ";
	regex rrr("\\s*(add[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line2, rrr)) {
		cout << "ADD";

	}
	else {
		cout << "nece add";
	}
	cout << endl << "--------------" << endl;
	
	string glob = " .global a, b,  c";
	regex gr("\\s*.global\\s+([a-zA-Z0-9\\_\\,\\s]+)\\s*");
	smatch mm;
	if (regex_search(glob, mm, gr)){
		cout << "GLOBAL" << endl;
		cout <<"m[1] = " << mm[1] <<endl;
		string por = mm[1];
		char* res = strtok(const_cast<char*>(por.c_str()),",");
		while (res != NULL) {
			cout << res << endl;
			res = strtok(NULL, ",");
		}

	}

	// Citanje fajla -------------------------------
	/*
	string line;
	ifstream infile(input);
	ofstream outfile(output);
	if (infile.is_open() && outfile.is_open()) {
	while (getline(infile, line)) {
	cout << line << '\n';
	outfile << line << '\n';
	}
	infile.close();
	outfile.close();
	}

	else
	cout << "Unable to open files";
	*/
	// ----------------------------------------------

	//cout << "press any key to continue...";
	//getchar();
	return 0;
}
