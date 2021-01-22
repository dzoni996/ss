/*
 * assembler.cpp
 *
 *  Created on: Aug 13, 2018
 *      Author: NIKOLA
 */
#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include "assembler.h"
#include "declarations.h"
#include "parser.h"
#include <iostream>
#include <string>
#include <fstream>
#include <iomanip>
#include <regex>
using namespace std;

Assembler::Assembler() {
	this->hasGlobal = false;
	this->locationCnt = 0;
	this->globalCnt = 0;
	this->currentSection = "-1";
	this->number1 = -1;

}

Assembler::~Assembler() {
	this->ST.clear();
	this->LT.clear();
	this->sizes.clear();

	this->textRel.clear();
	this->dataRel.clear();
	this->rodataRel.clear();

	this->data.clear();
	this->data.clear();
	this->rodata.clear();
}

void Assembler::process(string input, string output, location start) throw (Myexc) {
	int s = start;
	this->ST.clear();
	this->LT.clear();
	this->sizes.clear();

	this->textRel.clear();
	this->dataRel.clear();
	this->rodataRel.clear();

	this->data.clear();
	this->data.clear();
	this->rodata.clear();

	this->hasGlobal = false;
	this->locationCnt = 0;
	this->globalCnt = 0;
	this->currentSection = "-1";
	this->number1 = 0;

	// PRVI PROLAZ ASEMBLERA
	cout << "First pass..." << endl;
	this->firstPass(input);

	// LT
	int j=0;
	for (list<SegmentTable>::iterator i = this->LT.begin(); i != this->LT.end(); i++) {
		i->loc += start;
		i->size = sizes[++j];

	}
	// ISPIS POSLE I PROLAZA ----------------------------------------------------	
	//this->printSimbols();
	cout << "First pass done." << endl;
	//------------------------------------------------------

	// DRUGI PROLAZ ASEMBLERA
	cout << "Second pass..." << endl;
	this->secondPass(input);
	cout << "Second pass done." << endl;
	// ISPIS POSLE II PROLAZA ----------------------------------------------------
	ofstream outfile(output);

	this->printSimbols(outfile);
	this->printSegments(outfile);
	this->printRelocations(outfile);
	this->printSections(outfile);
	
	outfile.close();

	return;
}

void Assembler::firstPass(string input) throw (Myexc) {
	int lineNum = 1;
	string line;
	ifstream infile(input);
	if (infile.is_open() == true) {
		while (getline(infile, line)) {
			string test = line;

			// Da li je pocetak sekcije?
			if (Parser::isSection(line)){
				if (this->containsSimbol(Parser::name)) throw Myexc("Double definition of section '"+ Parser::name + "' in line "+ to_string(lineNum)+" in "+input);
				/*if (!LT.empty()) {
					LT.end()->size = locationCnt;
				}*/
				sizes.push_back(locationCnt);
				this->currentSection = Parser::section;
				this->locationCnt = 0; // reset local counter

				SimbolTable temp = SimbolTable(Parser::name, this->currentSection, 0, "local", this->number1++);
				temp.setDefined();
				this->ST.push_back(temp);
				SegmentTable stemp = SegmentTable(Parser::name, this->globalCnt, this->globalCnt);				
				this->LT.push_back(stemp);
				lineNum++;
				continue;
			}


			// Da li sadrzi labelu?
			if (Parser::isLabel(line)){
				if (this->containsSimbol(Parser::name)) throw Myexc("Double definition of '" + Parser::name + "' in line " + to_string(lineNum) + " in " + input);
				SimbolTable temp = SimbolTable(Parser::name, this->currentSection, this->locationCnt, "local", number1++);
				temp.setDefined();
				this->ST.push_back(temp);
			}
			test = line;

			// Da li je prazan red?
			if (Parser::isEmpty(line)) {
				lineNum++;
				continue;
			}

			// Da li je instrukcija?
			if (Parser::isOperation(line)) {
				// Da li je instrukcija u text sekciji
				if (this->currentSection.compare("TEXT") != 0) throw Myexc("Instruction in line "+to_string(lineNum)+" must be in .text section.");
				// Da li je dobar uslov
				if (this->checkCond(Parser::condition) == false) throw Myexc("Unkown condition for instruction '"+ Parser::condition+ "' in line " + to_string(lineNum) + " in " + input);
				// Da li su ispravni operandi
				if (Parser::opNum >= 1) {
					int addr = this->getAddr(Parser::dst);
					if (addr == -1)
						throw Myexc(
								"Unkown addressing mode '" + Parser::src
										+ "' in line " + to_string(lineNum)
										+ " in " + input);
					if (Parser::opNum == 2) {
						addr = this->getAddr(Parser::src);
						if (addr == -1)
							throw Myexc(
									"Unkown addressing mode '" + Parser::dst
											+ "' in line " + to_string(lineNum)
											+ " in " + input);
					}
				}
				// Adresiranja
				if (Parser::opNum == 2) {
					if (this->getAddr(Parser::src) == 0)
						if (this->getAddr(Parser::dst) == 0)
							throw Myexc(
									"Both operands can't be immediate in line "
											+ to_string(lineNum) + " in "
											+ input);
					if (this->getAddr(Parser::src) != 1	&& this->getAddr(Parser::dst) != 1)
						throw Myexc("One operand must be register in line "
										+ to_string(lineNum) + " in " + input);
					if (this->getAddr(Parser::dst) == 0)
						throw Myexc("Destination can't be immediate in line "
										+ to_string(lineNum) + " in " + input);


					// jos provera, za svaku instr proveriti!!!
				}
				if (Parser::opNum == 1) {
					if (Parser::isPop(line))
						if (this->getAddr(Parser::dst) == 0)
							throw Myexc("Invalid addressing mode in line "+ to_string(lineNum) + " in " + input);

				}

				// Velicina strukcije
				if (Parser::opNum == 2 && this->getAddr(Parser::src) == 1 && this->getAddr(Parser::dst) == 1) {
					this->locationCnt += 2;
					this->globalCnt += 2;
				} else if(Parser::opNum == 1 && this->getAddr(Parser::dst) == 1) {
					this->locationCnt += 2;
					this->globalCnt += 2;
				} else if (Parser::isRet(line)) {
					this->locationCnt += 2;
					this->globalCnt += 2;
				}
				else {
					this->locationCnt += 4;
					this->globalCnt += 4;
				}

				lineNum++;
				continue;
			}

			// Da li je direktiva?
			if (Parser::isDirective(line, this->locationCnt)) {
				if (this->currentSection.compare("TEXT") == 0)
					if (Parser::directive.compare("align") != 0)
						throw Myexc("Section .text can't contain directives in line " + to_string(lineNum) + " in " + input);
				int size = Parser::size;
				this->locationCnt += size;
				this->globalCnt += size;
				lineNum++;
				continue;
			}	

			// Da li je .global direktiva?
			if (Parser::isGlobal(line)) {
				if (this->hasGlobal == false)
					this->hasGlobal = true;
				else throw Myexc("Program can have only one .global directive!");
				if (this->currentSection.compare("-1") != 0) throw Myexc("Directive .global can't be in section!");
				lineNum++;
				continue;
			}

			// Da li je komentar?
			if (Parser::isComment(line)) {
				lineNum++;
				continue;
			}

			// Da li je kraj?
			if (Parser::isEnd(line)) {
				/*if (!LT.empty()) {
					LT.end()->size = locationCnt;
				}*/
				sizes.push_back(locationCnt);
				break;
			}
			
			// Ukoliko nije ni u jednoj od predhodnih slucajeva -> GRESKA!!!
			throw Myexc("Invalid operation in line " + to_string(lineNum) + " in " + input);
			
			}
		infile.close();
	}
	else {
		cout << "Unable to open files!" << endl;
	}

}

void Assembler::secondPass(string input) throw (Myexc) {
		int lineNum = 1;
		string line;
		ifstream infile(input);
		if (infile.is_open() == true) {
			while (getline(infile, line)) {
				string test = line;

				// Da li je pocetak sekcije?
				if (Parser::isSection(line)){
					this->currentSection = Parser::section;
					this->locationCnt = 0; // reset local counter
					lineNum++;
					continue;
				}

				// Da li sadrzi labelu?
				if (Parser::isLabel(line)){	}
				test = line;

				// Da li je prazan red?
				if (Parser::isEmpty(line)) {
					lineNum++;
					continue;
				}

				// Da li je instrukcija?
				if (Parser::isOperation(line)) {
					// DODATI OBRADU!!!

					// Velicina strukcije - OSTAJE!
					int size = 0;
					if (Parser::opNum == 2 && this->getAddr(Parser::src) == 1 && this->getAddr(Parser::dst) == 1) {
						size = 2;
					}
					else if (Parser::opNum == 1 && this->getAddr(Parser::dst) == 1) {
						size = 2;
					}
					else if (Parser::isRet(line)) {
						size = 2;
					}
					else {
						size = 4;
					}

					// ODREDJIVANJE OPCODE

					int cond = this->getCond(Parser::condition);
					int opcode = Parser::opCode;
					int dst = 0, src = 0, pom = 0;

					if (Parser::opNum == 2 && this->getAddr(Parser::src) == 1 && this->getAddr(Parser::dst) == 1) { // oba oper, su registri
						//size = 2;
						//int cond1 = this->getCond(Parser::condition);
						dst = dst | ((this->getAddr(Parser::dst) & 0x3) << 3) | (this->reg & 0x7);
						src = src | ((this->getAddr(Parser::src) & 0x3) << 3) | (this->reg & 0x7);
					}
					else if (Parser::opNum == 1 && this->getAddr(Parser::dst) == 1) { // jednosadr sa registrom
						//size = 2;

						dst = dst | ((this->getAddr(Parser::dst) & 0x3) << 3) | (this->reg & 0x7);
						src = 0;
					}
					else if (Parser::isRet(line)) { // pop pc(r7)
						//size = 2;
						opcode = 10; // pop
						dst = dst | (1 << 3) | 7; // r7
						src = 0;
					}
					else if (Parser::isJmp(line)) {
						if (this->getAddr(Parser::dst) == 0) { // mov r7, &lab
							opcode = 13; // mov
							dst = dst | (0x01 << 3) | 7;
							src = 0;
							pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_ABS", lineNum, input);

						}
						if (this->getAddr(Parser::dst) == 4) { // add r7, lab
							opcode = 0; // add
							dst = dst | (0x01 << 3) | 7;
							src = 0 | (2 << 3); // memorijsko adr!
							pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_REL", lineNum, input);
						}
					}
					else {
						//size = 4;
						// OBRADA DST
							// 0 neposredno
						if ((getAddr(Parser::dst) == 0)) {
							if (isSimbol == false) { // broj
								dst = 0;
								pom = immed;
							}
							else { // simbol
								dst = 0;
								pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_ABS", lineNum, input);
							}
						}
						// 1 registarsko
						if ((getAddr(Parser::dst) == 1)) {
							dst = dst | (0x01 << 3) | this->reg;
						}
						// 2 memorijsko
						if ((getAddr(Parser::dst) == 2)) {
							if (isSimbol == false) { // broj
								dst = dst | (0x2 << 3) | 0;
								pom = immed;
							}
							else { // simbol
								dst = dst | (0x2 << 3) | 0;
								pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_ABS", lineNum, input);
							}
						}
						// 3 memorijsko ind sa pomerajem
						if ((getAddr(Parser::dst) == 3)) {
							dst = dst | (0x3 << 3) | this->reg;
							if (isSimbol == false) { // broj									
								pom = immed;
							}
							else { // simbol
								pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_ABS", lineNum, input);
							}
						}
						if (Parser::opNum > 1) {
						// OBRADA SRC
						// 0 neposredno
						if ((getAddr(Parser::src) == 0)) {
							src = 0;
							if (isSimbol == false) { // broj
								pom = immed;
							}
							else { // simbol
								pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_ABS", lineNum, input);
							}
						}
						// 1 registarsko
						if ((getAddr(Parser::src) == 1)) {
							src = src | (0x01 << 3) | this->reg;
						}
						// 2 memorijsko
						if ((getAddr(Parser::src) == 2)) {
							src = src | (0x2 << 3) | 0;
							if (isSimbol == false) { // broj
								pom = immed;
							}
							else { // simbol								
								pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_ABS", lineNum, input);
							}
						}
						// 3 memorijsko ind sa pomerajem
						if ((getAddr(Parser::src) == 3)) {
							int r = this->reg;
							src = src | (0x3 << 3) | this->reg;
							if (isSimbol == false) { // broj									
								pom = immed;
							}
							else { // simbol
								pom = this->resolveSimbol(this->simbol, this->locationCnt + 2, "PC_ABS", lineNum, input);
							}
						}
					}
					}
					
					uint16_t Opcode = this->generateCode(cond, opcode, dst, src);
					char16_t first = (Opcode & 0xFF00) >> 8;
					char16_t second = (Opcode & 0x00FF);
					this->text.push_back(first);
					this->text.push_back(second);
					
					if (size == 4) {
						char16_t first = (pom & 0x00FF) ;
						char16_t second = (pom & 0xFF00) >> 8;
						this->text.push_back(first);
						this->text.push_back(second);
					}
					

					this->locationCnt += size;
					this->globalCnt += size;

					lineNum++;
					continue;
				}

				// Da li je direktiva?
				if (Parser::isDirective(line, this->locationCnt)) {

					int size = Parser::size;
					this->locationCnt += size;
					this->globalCnt += size;

					vector<char> *out = &this->bss;
					list <RelocationTable> *rel = &this->textRel;
					if (currentSection.compare("TEXT") == 0) {
						rel = &this->textRel;
						out = &this->text;
					}
					else if (currentSection.compare("DATA") == 0) {
						rel = &this->dataRel;
						out = &this->data;
					}
					else if (currentSection.compare("RODATA") == 0) {
						rel = &this->rodataRel;
						out = &this->rodata;
					}
					else if (currentSection.compare("BSS") == 0) {
						for (int i = 0; i < size; i++)
							this->bss.push_back(0);
						lineNum++;
						continue;
					}


					if (Parser::directive.compare("align") == 0) {
						for (int i = 0; i < size; i++)
							out->push_back(0);
						lineNum++;
						continue;
					}
					else if (Parser::directive.compare("skip") == 0) {
						for (int i = 0; i < size; i++)
							out->push_back(0);
						lineNum++;
						continue;
						
					}
					else { // char, word ili long
						int k = 0;

						string arguments = Parser::arguments;
						char* res = strtok(const_cast<char*>(arguments.c_str()), ",");
						while (res != NULL) {
							string arg = string(res);
							Parser::deleteSpaces(arg);
							
							regex rn("\\-?[0-9]+");
							if (regex_match(arg, rn)) { // broj
								int n = stoi(arg);
								if (Parser::directive.compare("char") == 0) out->push_back(n);
								else if (Parser::directive.compare("word") == 0) {
									out->push_back(n & 0x00FF);
									out->push_back((n & 0xFF00) >> 8);
								}
								else { // long
									char test = n & 0x0FF;
									for (int i = 0; i < 4; i++) {
										out->push_back(test);
										n = n >> 8;
										test = n & 0xFF;
										
									}
									//out->push_back(n & 0xFF);
									//out->push_back((n & 0xFF00) >> 8);
									//out->push_back((n & 0xFF0000) >> 16);
									//out->push_back((n & 0xFF000000) >> 24);
								}
							}
							else { // simbol
								bool flag = false;
								list<SimbolTable>::iterator iterator;
								//SimbolTable* elem = this->getSimbol(arg);
								for (iterator = this->ST.begin(); iterator != this->ST.end(); ++iterator) {
									if (iterator->name.compare(arg) == 0) {
										flag = true;
										// OVDE ISPITATI SVE SLUCAJEVE!!!
										if (iterator->visibility.compare("global") == 0 && iterator->defined == false) { // globalni
											// globalni, nedefinisani -> rel na simbol + vrednost 0
											
											RelocationTable temp = RelocationTable(this->locationCnt - size + k*Parser::typeSize, "PC_ABS", iterator->num);
											rel->push_back(temp);
											for (int i = 0; i < Parser::typeSize; i++)	out->push_back(0);
											
										}
										else { // lokalni
											// REL ZAPIS itd, da li je iz iste sekcije itd
											if (iterator->section.compare(this->currentSection) == 0) { // ista sekcija
												// ista sekcija, nema relokacije, upisuje se vrednost
												int n = iterator->value;
												char test = n & 0x0FF;
												for (int i = 0; i < Parser::typeSize; i++) {
													out->push_back(test);
													n = n >> 8;
													test = n & 0xFF;
												}
											}
											else {
												// razlicita sekcija -> rel na sekciju iz ko+ vrednost iz tebele simbola
												list<SimbolTable>::iterator iterator1;
												for (iterator1 = this->ST.begin(); iterator1 != this->ST.end(); ++iterator1) {
													if (iterator1->section.compare(iterator->section) == 0) {
														RelocationTable temp = RelocationTable(this->locationCnt - size + k * Parser::typeSize, "PC_ABS", iterator1->num);
														rel->push_back(temp);
														int n = iterator->value;
														char test = n & 0x0FF;
														for (int i = 0; i < Parser::typeSize; i++) {
															out->push_back(test);
															n = n >> 8;
															test = n & 0xFF;
														}
														break;
													}

												}
											}

										}
										// ----------------
										break;
									}
								}
								if (flag == false) throw Myexc("Undefined symbol in line " + to_string(lineNum) + " in " + input);
							} 
							k++;
							res = strtok(NULL, ",");

						}
					}
					lineNum++;
					continue;
				}

				// Da li je .global direktiva?
				if (Parser::isGlobal(line)) {
					// DODATI OBRADU!!!
					regex r("\\s*.global\\s+([a-zA-Z0-9\\_\\,\\s\\.]+)\\s*");
					smatch m;
					if (regex_search(line, m, r)){
						string por = m[1];
						char* res = strtok(const_cast<char*>(por.c_str()),",");
						while (res != NULL) {
							
							string arg = string(res);
							if (Parser::isSection(arg)) throw Myexc("Section can't be global!");
							Parser::deleteSpaces(arg);
							bool flag = false;
							list<SimbolTable>::iterator iterator;
							for (iterator = this->ST.begin(); iterator != this->ST.end(); ++iterator)
								if (iterator->name.compare(arg) == 0) {
									iterator->setGlobal();
									flag = true;
								}
							if (flag == false) {
								SimbolTable temp = SimbolTable(arg, "UND", 0, "global", this->number1++);
								this->ST.push_back(temp);
							}
							res = strtok(NULL, ",");
						}

					}

					// ---------------
					lineNum++;
					continue;
				}

				// Da li je komentar?
				if (Parser::isComment(line)) {
					lineNum++;
					continue;
				}

				// Da li je kraj?
				if (Parser::isEnd(line)) {
					break;
				}

				// Ukoliko nije ni u jednoj od predhodnih slucajeva -> GRESKA!!!
				throw Myexc("Unknown operation in line " + to_string(lineNum) + " in " + input);

				}
			infile.close();
		}
		else {
			cout << "Unable to open files!" << endl;
		}


 }


 bool Assembler::containsSimbol(string& sim) {
	 list<SimbolTable>::const_iterator iterator;
	 for (iterator = this->ST.begin(); iterator != this->ST.end(); ++iterator)
		 if (iterator->name.compare(sim) == 0) return true;
	 return false;
 }

 SimbolTable* Assembler::getSimbol(string& sim) {
	 string test = sim;
	 SimbolTable* ret = NULL;
	 list<SimbolTable>::const_iterator iterator;
	 for (iterator = this->ST.begin(); iterator != this->ST.end(); ++iterator)
		 //std::cout << *iterator;
		 if (iterator->name.compare(sim) == 0) {
			 ret = (SimbolTable*)&iterator;
			 break;
		 }
	 return ret;
 }

 bool Assembler::checkCond(string cond) {
	 if (cond.compare("gt") != 0 && cond.compare("eq") != 0 &&
		 cond.compare("ne") != 0 && cond.compare("al") != 0 && cond.compare("") != 0)
		 return false;
	 return true;
 }


 int Assembler::getAddr(string op) {
	 // 0-neposredno, 1-regdir, 2-memorijsko, 3-regindpom, 4-pcrel
	 regex r1("(\\-?[0-9]+)|(\\&[[a-zA-Z0-9_]+)");
	 if (regex_match(op, r1)) {
		 //cout << "IMMED" << endl;
		 regex rp("\\-?[0-9]+");
		 if (regex_match(op, rp)) { //  broj
			 immed = stoi(op);
			 isSimbol = false;
		 }
		 else { // simbol
			 simbol = op.substr(1, op.length()-1);
			 isSimbol = true;
		 }
		 return 0;		 
	 }
	 regex r2("r([0-7])");
	 if (regex_match(op, r2)) {
		 //cout << "REGDIR" << endl;
		 smatch m;
		 if (regex_search(op, m, r2)) {
			 reg = stoi(m[1]);
		 }
		 else reg = -1;

		 return 1;
	 }
	 // ako je podrazano *labela, dodati u regex (\\*[a-zA-Z0-9_]+)
	 regex r3("([a-zA-Z0-9_]+)|(\\*[0-9]+)");
	 if (regex_match(op, r3)) {
		 //cout << "MEM" << endl;
		 regex rp("\\*[0-9]+");
		 if (regex_match(op, rp)) { // memind
			 immed = stoi(op.substr(1, op.length()-1));
			 isSimbol = false;
		 }
		 else { // memdir
			 simbol = op;
			 isSimbol = true;
		 }
		 return 2;
	 }
	 regex r4("r([0-7])\\[([0-9a-zA-Z]+)\\]");
	 if (regex_match(op, r4)) {
		 //cout << "REGINDPOM" << endl;
		 smatch m;
		 if (regex_search(op, m, r4)) {
			 reg = stoi(m[1]);
			 regex rp("\\-?[0-9]+");
			 string pom = m[2];
			 if (regex_match(pom, rp)) { //  broj
				 immed = stoi(pom);
				 isSimbol = false;
			 }
			 else { // simbol
				 simbol = pom;
				 isSimbol = true;
			 }

		 }
		 return 3;		
	 }
	 regex r5("\\$[a-zA-Z0-9_]+");
	 if (regex_match(op, r5)) {
		 //cout << "PCREL" << endl;
		 simbol = op.substr(1, op.length());

		 return 4;
	 }
	 return -1;
 }

void Assembler::printSimbols(ofstream& cout) {
	cout << "# Simbols' table \n";
	cout << setw(12) << "Name" << setw(10) << "Section" << setw(8) << "Value" << setw(8) << "Vis" << setw(8) << "Num" << endl;
	for (list<SimbolTable>::iterator i = this->ST.begin(); i != this->ST.end();	i++)
		cout << setw(12) << right << i->name << setw(10) << i->section << setw(8) << i->value 
		<< setw(8) << i->visibility  << setw(6) << i->num << setw(6) << i->defined << endl;
}

void Assembler::printSegments(ofstream& cout) {
	cout << endl << "# Segments' table \n";
	cout << setw(8) << "Name" << '\t' << "Base" << '\t' << "Size" << endl;
	for (list<SegmentTable>::iterator i = this->LT.begin(); i != this->LT.end(); i++) {
		cout << setw(8) << right << i->section << '\t' << setw(4) << i->loc << '\t' << setw(4) << i->size << endl;
	}
}

void Assembler::printSections(ofstream& cout) {
	if (text.size() > 0) {
		cout << endl << "# .text section" << endl;
		int ii = 0;
		for (int i = 0; i < text.size(); i++) {
			//printf("%02X ", (unsigned char)text[i]);
			cout << setfill('0') << setw(2) << hex << +(unsigned char)text[i] << " ";
			++ii;
			if (ii % 8 == 0) cout << endl;
		}
		cout << endl;
	}
	if (data.size() > 0) {
		cout << endl << "# .data section" << endl;
		int ii = 0;
		for (int i = 0; i < data.size(); i++) {
			//printf("%02X ", (unsigned char)data[i]);
			cout << setfill('0') << setw(2) << hex << +(unsigned char)data[i] << " "; // radi i ovo!
			++ii;
			if (ii % 8 == 0) cout << endl;
		}
		cout << endl;
	}
	if (rodata.size() > 0) {
		cout << endl << "# .rodata section" << endl;
		int ii = 0;
		for (int i = 0; i < rodata.size(); i++) {
			//printf("%02X ", (unsigned char)rodata[i]);
			cout << setfill('0') << setw(2) << hex << +(unsigned char)rodata[i] << " ";
			++ii;
			if (ii % 8 == 0) cout << endl;
		}
		cout << endl;
	}
	if (bss.size() > 0) {
		cout << endl << "# .bss section" << endl;
		int ii = 0;
		for (int i = 0; i < bss.size(); i++) {
			//printf("%02X ", (unsigned char)bss[i]);
			cout << setfill('0') << setw(2) << hex << +(unsigned char)bss[i] << " ";
			++ii;
			if (ii % 8 == 0) cout << endl;
		}
		cout << endl;
	}

}

void Assembler::printRelocations(ofstream& cout1) {

	// Relocations text
	int i = 0;
	if (textRel.size() > 0) {
		cout1 << endl << "# .rel .text" << endl;
		cout1 << setw(8) << "Offset" << setw(8) << "Type" << setw(6) << "Ref" << endl;
		for (list <RelocationTable>::iterator i = this->textRel.begin(); i != this->textRel.end(); i++) {
			cout1  << setw(8) << hex << i->offset << setw(8) << i->type << setw(6) << dec << i->ref << endl;
		}
		cout1 << endl;
	}

	// Relocations data
	if (dataRel.size() > 0) {
		cout1 << endl << "# .rel .data" << endl;
		cout1 << setw(8) << "Offset" << setw(8) << "Type" << setw(6) << "Ref" << endl;
		for (list <RelocationTable>::iterator i = this->dataRel.begin(); i != this->dataRel.end(); i++) {
			cout1 << setw(8) << hex << i->offset << setw(8) << i->type << setw(6) << dec << i->ref << endl;
		}
		cout1 << endl;
	}

	// Relocations rodata
	if (rodataRel.size() > 0) {
		cout1 << endl << "# .rel .rodata" << endl;
		cout1 << setw(12) << "Offset" << setw(8) << "Type" << setw(6) << "Ref" << endl;
		for (list <RelocationTable>::iterator i = this->rodataRel.begin(); i != this->rodataRel.end(); i++) {
			cout1  << setw(8) << hex << i->offset << setw(8) << i->type << setw(6) << dec << i->ref << endl;
		}
		cout1 << endl;
	}

}

uint16_t Assembler::generateCode(int cond, int opcode, int dst, int src) {
	char16_t op = 0;
	op |= (cond & 0x03) << 14;
	op |= (opcode & 0x0F) << 10;
	//char16_t dst = 0 | (1 << 3) | (1);
	//char16_t src = 0 | (1 << 3) | (2);
	op |= (dst & 0x1F) << 5;
	op |= (src & 0x1F);
	char16_t test = op;
	//cout << "0x" << setfill('0') << setw(4) << hex << op << endl;
	//char16_t first = (op & 0xFF00) >> 8;
	//char16_t second = (op & 0x00FF);
	//cout << setfill('0') << setw(2) << hex << first << " " << second;

	return op;
}

int Assembler::getCond(string cond) {
	if (cond.compare("eq") == 0) return 0;
	if (cond.compare("ne") == 0) return 1;
	if (cond.compare("gt") == 0) return 2;
	if (cond.compare("al") == 0 || cond.compare("") == 0) return 3;
	return - 1;
}

int Assembler::resolveSimbol(string arg, int loc, string reltype, int lineNum, string input) throw (Myexc) { // reltype = "PC_ABS" / "PC_REL"
		vector<char> *out = &this->text;
		list <RelocationTable> *rel = &this->textRel;

		bool flag = false;
		list<SimbolTable>::iterator iterator;
		for (iterator = this->ST.begin(); iterator != this->ST.end(); ++iterator) {
			if (iterator->name.compare(arg) == 0) {
				flag = true;
				if (iterator->visibility.compare("global") == 0 && iterator->defined == false) { 
					// globalni, nedefinisani -> rel na simbol + vrednost 0
					RelocationTable temp = RelocationTable(loc, reltype, iterator->num);
					rel->push_back(temp);
					return -2;
				}
				else { // lokalni
					if (iterator->section.compare(this->currentSection) == 0) { 
						// ista sekcija, nema relokacije, upisuje se vrednost
						int n = iterator->value;
						return n;

					}
					else {
						// razlicita sekcija -> rel na sekciju + vrednost iz tebele simbola
						list<SimbolTable>::iterator iterator1;
						for (iterator1 = this->ST.begin(); iterator1 != this->ST.end(); ++iterator1) {
							if (iterator1->section.compare(iterator->section) == 0) {
								RelocationTable temp = RelocationTable(loc, reltype, iterator1->num);
								rel->push_back(temp);
								int n = iterator->value;
								return n;

								break;
							}

						}
					}

				}
				break;
			}
		}
		if (flag == false) throw Myexc("Undefined symbol in line " + to_string(lineNum) + " in " + input);
	}