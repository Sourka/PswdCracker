/* Simple program guessing passwords, written by Sourka.
 * It requests for a password and hints, helping (or not) to guess the password.
 * Set of characters/words being checked is created based on the password. */
 
#ifndef MAIN_H
	#define MAIN_H

#include <iostream>
#include <stdexcept>
#include <functional>
#include <limits>
#include <cmath>
#include <vector>
#include <algorithm>


template<typename Chain> struct Args;
typedef std::vector<std::string> strarr;

// IO, and other
bool rescueCin(); // uses <limits>
template<typename T> bool typeDependentInput(T&, char);
// bool typeDependentInput(std::string&, char); // not used
// bool typeDependentInput(std::vector<std::string>&, char); // not used
template<typename T> bool input(T&, std::string&);
template<typename T> bool loopedInput(T&);
template<typename T> bool loopedInput(T&, std::function<bool(T&)>&);
strarr getPassword();
template<typename Chain> Chain getPossibleWords(Chain&);
template<typename Chain> bool getLenHint(Args<Chain>&, uint32_t);
double getMs(double);

// UE–related
void welcome();
void logActionStart(uint32_t, uint32_t); // uses <cmath>
void logActionEnd(double);
void showResult(bool found, double& t);

// The „guessor”
template<typename Chain>
	bool guess(Args<Chain>&, Chain&, const std::function<bool(Chain&)>&);
	
template<typename Chain>
	bool match(Chain&, uint32_t, Chain&, uint32_t, const std::function<bool(Chain&)>&);


// Main functions
template<typename Chain> bool runGuesswork(Chain&, double&);
bool turn(double&);
bool startTurn();
bool programLoop();


template<typename Chain> 
struct Args {
	uint32_t minLength, maxLength;
	Chain possibleItems;
};


#endif
