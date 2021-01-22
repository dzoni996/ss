/*
 * assembler.h
 *
 *  Created on: Aug 13, 2018
 *      Author: NIKOLA
 */

#ifndef ASSEMBLER_H_
#define ASSEMBLER_H_

#include "declarations.h"
#include <iostream>
#include <string>
#include <list>
#include <iterator>
#include <vector>
using namespace std;


class Assembler {

private:

	string currentSection;
	location locationCnt;
	location globalCnt;
	int number1;

	list <SimbolTable> ST;
	list <SegmentTable> LT;

	list <RelocationTable> textRel;
	list <RelocationTable> dataRel;
	list <RelocationTable> rodataRel;

	vector<int> sizes;
	bool containsSimbol(string&);
	SimbolTable* getSimbol(string&);
	bool checkCond(string);
	int getAddr(string); // 0-neposredno, 1-regdir, 2-memorisjki, 3-regindpom, 4-pcrel, -1-greska
	int getCond(string);
	bool hasGlobal;

	void printSimbols(ofstream&);
	void printSegments(ofstream&);
	void printSections(ofstream&);
	void printRelocations(ofstream&);

	vector<char> text;
	vector<char> data;
	vector<char> rodata;
	vector<char> bss;

	// For opcode
	int reg;
	int immed;
	string simbol;
	bool isSimbol;

	int resolveSimbol(string, int, string, int, string) throw (Myexc);
	// pomocna funkcija za operacije, ispituje siblo da li definisan ili ne, postavlja 
	// odg. relokacione zapise za text sekciju i vraca vrednost siblola ili 0 ako je nedefinisan
	// argumenti: simbol, lokaciju gde je potrebna promena i tip relokacije - PC_ABS ili PC_REL
	// broj linije i naziv ulaznog fajksa zbog greske!

	uint16_t generateCode(int, int, int, int); // cond, opcode, dst, src

public:
	Assembler();
	~Assembler();

	void firstPass(string input) throw (Myexc);
	void secondPass(string input) throw (Myexc);

	void process (string _input, string _output, location _start = 0) throw (Myexc);

	//location getLC () const { return this->locationCnt; }
	//void setLC (location loc) { this->locationCnt = loc; }
};




#endif /* ASSEMBLER_H_ */
