/*
 * declarations.h
 *
 *  Created on: Aug 13, 2018
 *      Author: NIKOLA
 */

#include <string>
using namespace std;

#ifndef DECLARATIONS_H_
#define DECLARATIONS_H_

// SECTIONS 		-------------------

typedef int section;
const section TEXT = 0;
const section DATA = 1;
const section RODATA = 2;
const section BSS = 3;


// LOCATON 			-------------------

typedef int location;


// TABELA SIMBOLA 	-------------------

struct simbolsTable {
public:
	string name;
	string section;
	int value;
	string visibility;
	int num;
	bool defined;

	simbolsTable(string n, string s, int val, string vis, int nu):
		name(n),section(s),value(val),visibility(vis), num(nu) {
		defined = false;
	}

	void setVis(string s) { visibility = s; }
	void setGlobal() { visibility = "global"; }

	void setDefined() {
		defined = true;
	}
};

typedef simbolsTable SimbolTable;


// TABELA SEGMENATA	-------------------

struct segmentTable {
public:
	string section;
	location loc;
	int size;


	segmentTable(string sec, location l, int s):loc(l),size(s),section(sec) {
		
	}

};

typedef segmentTable SegmentTable;

// TABELA REALOKACIJA	-------------------

struct relocationTable {
public:
	int offset;
	string type;
	int ref;

	relocationTable(int v, string t, int r) :offset(v), type(t), ref(r) {}
};

typedef relocationTable RelocationTable;

// GRESEKE			-------------------
class Myexc {
	string msg;
public:
	Myexc(string s) : msg(s) {}
	string getMsg() const { return msg; }
};

#endif /* DECLARATIONS_H_ */
