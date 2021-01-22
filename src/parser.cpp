/*
 * parser.cpp
 *
 *  Created on: Aug 15, 2018
 *      Author: NIKOLA
 */

#include "parser.h"
#include <string>
#include <regex>
#include <iostream>
using namespace std;

string Parser::section = "";
string Parser::name = "";
int Parser::cnt = 0;
int Parser::size = 0;
string Parser::operation = "";
string Parser::condition = "";
string Parser::dst = "";
string Parser::src = "";
int Parser::opNum = 0;
int Parser::opCode = -1;
string Parser::arguments = "";
string Parser::directive = "";
int Parser::typeSize = 0;

// OBRADA DIREKTIVA

bool Parser::isDirective(string line, location loc) {

	// Skip direktiva
	regex r1("\\s*(.skip\\s+[0-9]+)\\s*");
	if (regex_match(line, r1)){
		directive = "skip";
		regex rs("\\s*.skip\\s+([0-9]+)\\s*");
		smatch m;
		int bytes = 0;
		if (regex_search(line, m, rs) == true)
						//cout << endl <<"match[0]=" << m[0] << ", match[1]=" << m[1] << endl;
		bytes = stoi(m[1]);
		size = bytes;
		return true;
	}

	// Align direktiva
	regex r2("\\s*(.align\\s+[0-9]+)\\s*");
	if (regex_match(line, r2)){
		directive = "align";
		regex rs("\\s*.align\\s+([0-9]+)\\s*");
		smatch m;
		int arg = 0;
		if (regex_search(line, m, rs) == true)
			//cout << endl <<"match[0]=" << m[0] << ", match[1]=" << m[1] << endl;
			arg = stoi(m[1]);
		cnt = alignLoc(loc, arg);
		size = cnt - loc;
		return true;
	}

	// Char direktiva
	regex r4("\\s*(.char\\s+[\\-a-z0-9,\\s]+)\\s*");
	if (regex_match(line, r4)) {
		directive = "char";
		regex rc("\\s*.char\\s+([\\-a-z0-9,\\s]+)\\s*");
		smatch m;
		if (regex_search(line, m, rc)) {
			string op = m[1];
			size_t n = count(op.begin(), op.end(), ',');
			//cout << "broj simbola je " << n + 1 << endl;
			size = n + 1;
			// Ovde dolazi obrada za drugi prolaz
			// op - string koji stoji iza .char
			// treba odvojiti operande, sibole i brojeve (podeliti string na toekene oko , pa obrisati bele znakove) itd.
			arguments = op;
			typeSize = 1;
			//
			return true;
		}
	}

	// Word direktiva
	regex r5("\\s*(.word\\s+[\\-a-z0-9,\\s]+)\\s*");
	if (regex_match(line, r5)) {
		directive = "word";
		regex rc("\\s*.word\\s+([\\-a-z0-9,\\s]+)\\s*");
		smatch m;
		if (regex_search(line, m, rc)) {
			string op = m[1];
			size_t n = count(op.begin(), op.end(), ',');
			//cout << "broj simbola je " << n + 1 << endl;
			size = n + 1;
			size *= 2; // velicina word-a je 2B
			// Ovde dolazi obrada za drugi prolaz
			// op - string koji stoji iza .word
			// treba odvojiti operande, sibole i brojeve (podeliti string na toekene oko , pa obrisati bele znakove) itd.
			arguments = op;
			typeSize = 2;
			//
			return true;
		}
	}

	// Long direktiva
	regex r6("\\s*(.long\\s+[\\-a-z0-9,\\s]+)\\s*");
	if (regex_match(line, r6)) {
		directive = "long";
		regex rc("\\s*.long\\s+([\\-a-z0-9,\\s]+)\\s*");
		smatch m;
		if (regex_search(line, m, rc)) {
			string op = m[1];
			size_t n = count(op.begin(), op.end(), ',');
			//cout << "broj simbola je " << n + 1 << endl;
			size = n + 1;
			size *= 4;	// jer je velicina long-a 4B
			// Ovde dolazi obrada za drugi prolaz
			// op - string koji stoji iza .long
			// treba odvojiti operande, sibole i brojeve (podeliti string na toekene oko , pa obrisati bele znakove) itd.
			arguments = op;
			typeSize = 4;
			//
			return true;
		}
	}


	return false;
}

// GLOBAL DIREKTIVA

bool Parser::isGlobal(string line) {
	regex r("\\s*(.global\\s+.*)\\s*");
	if (regex_match(line, r)) {
		
		// Za II prolaz dodati funkcionalnosti!

		size = 0;
		return true;
	}
	return false;
}
// OBRADA LABELA

bool Parser::isLabel(string& line) {
	regex r("\\s*([a-zA-Z0-9_]*:)\\s*.*");
	if (regex_match(line, r)) {
		// MOZDA OVAJ DEO DA IDE U FIRST PASS???

		char* res = strtok(const_cast<char*>(line.c_str()),":");
		name = res;
		deleteSpaces(name);
		res = strtok(NULL, ":");
		if (res == NULL) line = " ";
		else line = res;
		// ------------------------------------
		return true;
	}

	return false;
}


// OBRADA INSTRUKCIJA

bool Parser::isOperation(string line) {
	
	// Add, sub, mul, div instrukcija
	if (isAdd(line)) return true;
	if (isSub(line)) return true;
	if (isMul(line)) return true;
	if (isDiv(line)) return true;

	// And, or, not
	if (isNot(line)) return true;
	if (isAnd(line)) return true;
	if (isOr(line)) return true;

	// Cmp, test
	if (isCmp(line)) return true;
	if (isTest(line)) return true;

	// Shl, shr
	if (isShl(line)) return true;
	if (isShr(line)) return true;

	// Mov
	if (isMov(line)) return true;

	// Push, pop
	if (isPush(line)) return true;
	if (isPop(line)) return true;

	// Ret
	if (isRet(line)) return true;

	// Call, jmp
	if (isCall(line)) return true;
	if (isJmp(line)) return true;

	return false;
}


// OBRADA SEKCIJA

bool Parser::isSection(string line) {

	// TEXT sekcija
	regex r1("\\s*(.text)\\s*");
	if (regex_match(line, r1)) {
		section = "TEXT";
		name = ".text";
		return true;
	}
	// DATA sekcija
	regex r2("\\s*(.data)\\s*");
	if (regex_match(line, r2)) {
		section = "DATA";
		name = ".data";
		return true;
	}
	// RODATA sekcija
	regex r3("\\s*(.rodata)\\s*");
	if (regex_match(line, r3)) {
		section = "RODATA";
		name = ".rodata";
		return true;
	}
	// BSS sekcija
	regex r4("\\s*(.bss)\\s*");
	if (regex_match(line, r4)) {
		section = "BSS";
		name = ".bss";
		return true;
	}

	return false;
}


// KRAJ PROGRAMA

bool Parser::isEnd(string line) {
	regex r("\\s*(.end)\\s*");
	if (regex_match(line, r)) {

		return true;
	}

	return false;
}


// PRAZAN RED

bool Parser::isEmpty(string line) {
	regex r("\\s*");
	if (regex_match(line, r)) {
		return true;
	}
	return false;
}

// KOMENATAR

bool Parser::isComment(string line) {
	regex r("\\s*\\#.*");
	if (regex_match(line, r)) return true;
	return false;
}

// BRISANJE BLANKO ZNAKOVA IZ STRINGA

string& Parser::deleteSpaces(string& name) {
	/*
	string::iterator end_pos = remove(name.begin(), name.end(), ' ');
	name.erase(end_pos, name.end());
	end_pos = remove(name.begin(), name.end(), '\t');
	name.erase(end_pos, name.end());
	*/
	name.erase(remove(name.begin(), name.end(), ' '), name.end());
	name.erase(remove(name.begin(), name.end(), '\t'), name.end());
	return name;
}

// INSTRIKCIJE, POMOCNE FUNKCIJE

bool Parser::isAdd(string line) {
	regex r("\\s*(add[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(add[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2]; 
			src = m[3]; 
			condition = string(m[1]).substr(3, 2);
			
			opCode = 0;
			return true;
		}
	}
	return false;
}

bool Parser::isSub(string line) {
	regex r("\\s*(sub[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(sub[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 1;
			return true;
		}
	}
	return false;
}

bool Parser::isMul(string line) {
	regex r("\\s*(mul[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(mul[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 2;
			return true;
		}
	}
	return false;
}

bool Parser::isDiv(string line) {
	regex r("\\s*(div[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(div[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 3;
			return true;
		}
	}
	return false;
}

bool Parser::isCmp(string line) {
	regex r("\\s*(cmp[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(cmp[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 4;
			return true;
		}
	}
	return false;
}

bool Parser::isAnd(string line) {
	regex r("\\s*(and[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(and[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);
			
			opCode = 5;
			return true;
		}
	}
	return false;
}

bool Parser::isOr(string line) {
	regex r("\\s*(or[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(or[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 2);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(2, 2);

			opCode = 6;
			return true;
		}
	}
	return false;
}

bool Parser::isNot(string line) {
	regex r("\\s*(not[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(not[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 7;
			return true;
		}
	}
	return false;
}

bool Parser::isTest(string line) {
	regex r("\\s*(test[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(test[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 4);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(4, 2);

			opCode = 8;
			return true;
		}
	}
	return false;
}

bool Parser::isShl(string line) {
	regex r("\\s*(shl[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(shl[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 14;
			return true;
		}
	}
	return false;
}

bool Parser::isShr(string line) {
	regex r("\\s*(shr[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(shr[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1 )) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 15;
			return true;
		}
	}
	return false;
}

bool Parser::isMov(string line) {
	regex r("\\s*(mov[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+\\s*,\\s*[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(mov[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*,\\s*([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 2;
			dst = m[2];
			src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 13;
			return true;
		}
	}
	return false;
}

bool Parser::isPush(string line) {
	regex r("\\s*(push[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(push[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 4);
			opNum = 1;
			dst = m[2];
			//src = m[3];
			condition = string(m[1]).substr(4, 2);

			opCode = 9;
			return true;
		}
	}
	return false;
}

bool Parser::isPop(string line) {
	regex r("\\s*(pop[a-z]{0,2}\\s+[\\w\\$\\*\\[\\]\\&]+)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(pop[a-z]{0,2})\\s+([\\w\\$\\*\\[\\]\\&]+)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 1;
			dst = m[2];
			//src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 10;
			return true;
		}
	}
	return false;
}

bool Parser::isRet(string line) {
	regex r("\\s*(ret[a-z]{0,2})\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(ret[a-z]{0,2})\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 0;
			//dst = m[2];
			//src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 12;
			return true;
		}
	}
	return false;
}

bool Parser::isCall(string line) {
	regex r("\\s*(call[a-z]{0,2}\\s+\\&[a-zA-Z0-9_]*)\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(call[a-z]{0,2})\\s+(\\&[a-zA-Z0-9_]*)\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 4);
			opNum = 1;
			dst = m[2];
			//src = m[3];
			condition = string(m[1]).substr(4, 2);

			opCode = 11;
			return true;
		}
	}
	return false;
}

bool Parser::isJmp(string line) {
	regex r("\\s*(jmp[a-z]{0,2})\\s+((\\&[a-zA-Z0-9_]+)|(\\$[a-zA-Z0-9_]+))\\s*");
	if (regex_match(line, r)) {
		regex r1("\\s*(jmp[a-z]{0,2})\\s+((\\&[a-zA-Z0-9_]+)|(\\$[a-zA-Z0-9_]+))\\s*");
		smatch m;
		if (regex_search(line, m, r1)) {
			operation = string(m[1]).substr(0, 3);
			opNum = 1;
			dst = m[2];
			//src = m[3];
			condition = string(m[1]).substr(3, 2);

			opCode = 16;
			return true;
		}
	}
	return false;
}
