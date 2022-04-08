/* Simple program guessing passwords, written by Sourka.
 * It requests for a password and hints, helping (or not) to guess the password.
 * Set of characters/words being checked is created based on the password.
 * The program does not support passphrases yet. */

#include <iostream>
#include <stdexcept>
#include <functional>
#include <limits>
#include <cmath>

struct Args;
typedef std::function<bool(std::string&)> pswdChecker;

// IO, and other
template<typename T> bool input(T&);
template<typename T> bool loopedInput(T&);
template<typename T> bool loopedInput(T&, std::function<bool(T&)>&);
std::string getPassword();
std::string getPossibleChars(std::string&);
bool getLenHint(Args&, uint32_t);
double getMs(double);

// UE–related
bool welcome();
bool logAction(uint32_t, uint32_t);
bool showResult(bool found, double& t);

// The „guessor”
bool guess(Args&, const pswdChecker&);
bool match(std::string&, uint32_t, std::string&, uint32_t, const pswdChecker&);

// Main function
bool turnErrCatcher();
bool turn(double&);
bool programLoop();


struct Args {
	uint32_t minLength, maxLength;
	std::string possibleChars;
};

// ················· //

bool rescueCin() {
	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return true;
}

template<typename T> bool input(T& data) {
	std::cin >> data;
	if(std::cin.fail()) {
		rescueCin();
	} else if(!std::cin.good()) { // bad or eof
		throw std::runtime_error("Some error on console input.");
	}
	return true;
}

template<typename T> bool loopedInput(T& data) {
	for(uint32_t i = 0; i < 4; ++i) {
		// input goes to referenced variable
		if(!input(data)) {
			std::cout << "Uncorrect data, please try again." << std::endl;
		} else return true;
	}
	throw std::runtime_error("Input request dismissed.");
}

template<typename T> bool loopedInput(T& data, std::function<bool(T&)>& validator) {
	for(uint32_t i = 0; i < 4; ++i) {
		// input goes to referenced variable
		if(!input(data) || !validator(data)) {
			std::cout << "Uncorrect data, please try again." << std::endl;			
		} else return true;
	}
	throw std::runtime_error("Input request dismissed.");
}

std::string getPassword() {
	std::cout << "Provide the password:" << std::endl;
	std::string pswd;
	loopedInput(pswd);
	if(pswd.size() > 40) {
		std::runtime_error e("Password too long.");
		throw e;
	}
	return pswd;
}

// Returns set of characters with only one occurence of each.
std::string getPossibleChars(std::string& txt) {
	std::string filtered;
	for(auto ch: txt) {
		int32_t pos = filtered.find(ch);
		if(pos == -1)
			filtered += ch;
		else continue;
	}
	return filtered;
}

// Gets min and max length hits from user and manages those inputs.
bool getLenHint(Args& args, uint32_t pswdLength) {
	std::function<bool(uint32_t&)> validator = [pswdLength, &args](uint32_t& n) -> bool {
		return n > 0 &&
			n < pswdLength + 30 &&
			args.minLength <= args.maxLength;
	};
	args.minLength = args.maxLength = UINT32_MAX;
	std::cout << "Provide a hint of minimal password length:" << std::endl;
	loopedInput(args.minLength, validator);
	std::cout << "Provide a hint of maximal password length:" << std::endl;
	loopedInput(args.maxLength, validator);
	if(!args.minLength)
		args.minLength = 1;
	return true;
}

// Converts time returned by clock() to miliseconds. It may be system–dependent.
double getMs(double raw) {
	return raw / (CLOCKS_PER_SEC / 1000);
}

// ················· //

bool welcome() {
	std::cout << "This is a simple password guesser.\n" << std::endl;
	return true;
}

bool logAction(uint32_t possibleChars, uint32_t length) {
	std::cout << "Testing for " << length << " length: "
		<< pow(possibleChars, length) << " possible combinations." << std::endl;
	return true;
}

bool showResult(bool found, double& ms) {
	std::cout << (found ? "Guessed!" : "Not guessed.") << std::endl;
	std::cout << "Operation took " << ms << " miliseconds." << std::endl;
	return true;
}

// ················· //

// There's no need to return the password, as user will know it.
bool guess(Args& args, const pswdChecker& verifier) {
	for(uint32_t i = args.minLength; i <= args.maxLength; ++i) {
		std::string s; // it can be returned.
		logAction(args.possibleChars.size(), i);
		bool result = match(args.possibleChars, i, s, 0, verifier);
		if(result) return true;
	}
	return false;
}

// Real result can be accessed by reference.
bool match(std::string& possibleChars, uint32_t length, std::string& tested, uint32_t pswdPos, const pswdChecker& verify) {
	if(!tested.size()) { // not initialized yet
		tested = std::string(length, possibleChars[0]);
	}
	for(uint32_t i = 0; i < possibleChars.size(); ++i) {
		tested[pswdPos] = possibleChars[i];
		bool result;
		if(pswdPos + 1 >= length) {
			result = verify(tested);
		} else {
			result = match(possibleChars, length, tested, pswdPos + 1, verify);
		}
		if(result) return result;
	}
	return false;
}

// ················· //

// It works, but it is not good enough at the user side.
bool turnErrCatcher() {
	try {
		double took;
		bool result = turn(took);
		showResult(result, took);
	} catch(std::runtime_error& e) {
		std::cout << "\nAn error occured:\n" << e.what() << std::endl;
		std::cout << "Type \"exit\", if you want to close the program.\n";
		std::string answer;
		// It can throw the same error, as being handled – so rethrow it.
		try {
			loopedInput(answer);
		} catch(std::runtime_error& f) {
			throw std::runtime_error("Error occured during error handling.");
		}
		if(answer == "exit") return false;
	}
	return true;
}

bool turn(double& ntime) {
	Args hints;
	std::string pswd = getPassword();
	getLenHint(hints, pswd.size());
	hints.possibleChars = getPossibleChars(pswd);
 	// matches(pswd); // init
 	auto verifier = [pswd](std::string& proposition) -> bool {
		return pswd == proposition;
	};
	clock_t before = clock();
	bool result = guess(hints, verifier);
	clock_t after = clock();
	ntime = getMs(after - before);
 	return result;
}

bool programLoop() {
	while(true) {
		if(!turnErrCatcher())
			break; // decided to quit
		std::cout << std::endl;
	}
	return true;
}


int main() {
	welcome();
	programLoop();
	return 0;
}
