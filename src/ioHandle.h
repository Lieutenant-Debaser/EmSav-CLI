#ifndef IOHANDLE_H
#define IOHANDLE_H

#include <iostream>
#include <string>
#include <vector>

using namespace std;

void printWelcome ();
void clearScreen ();
void mainMenu ();
int parseInput (vector <string> &parsed, string input, char delimiter);
void printError (string errorMessage);
void printHelp ();
void clearBuffer();
void printEditHelp();

#endif
