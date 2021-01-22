/*
 * parser.h
 *
 *  Created on: Aug 15, 2018
 *      Author: NIKOLA
 */
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include "declarations.h"
#include <math.h>

using namespace std;

#ifndef PARSER_H_
#define PARSER_H_

class Parser {
private:
	static location alignLoc (location adr, int arg){
		int por = pow(2,arg);
		//cout << "adr=" << adr << endl << "poravnanje na arg=" << arg << endl;
		while (adr % por != 0) {
			adr++;
		}
		return adr;
	}


	static string arguments;
	static string directive;
	static int typeSize;

	static bool isAdd(string);
	static bool isSub(string);
	static bool isMul(string);
	static bool isDiv(string);
	static bool isCmp(string);
	static bool isAnd(string);
	static bool isOr(string);
	static bool isShl(string);
	static bool isShr(string);
	static bool isNot(string);
	static bool isTest(string);
	static bool isMov(string);
	
	static bool isPush(string);
	static bool isPop(string);

	static bool isCall(string);
	static bool isJmp(string);

	static bool isRet(string);

public:
	string static section;	// naziv sekcije
	string static name;		// naziv simbola
	static int cnt;			// local counter???
	static int size; 		// velicina instrukcije

	// za instrukcije
	static string operation;
	static string condition;
	static string dst;
	static string src;
	static int opNum;
	static int opCode;

	bool static isDirective (string, location);
	bool static isGlobal(string);
	bool static isOperation(string);
	bool static isSection(string);
	bool static isEmpty(string);
	bool static isEnd(string);
	bool static isLabel(string&);
	bool static isComment(string);


	static string& deleteSpaces(string&);


	friend class Assembler;
};



#endif /* PARSER_H_ */
