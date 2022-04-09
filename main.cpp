#include "main.h"

/* Setting here „false” will disable logging to console during time–consuming operations.
 * Doing it by program parameters is not available now, and would be slower. */
#define SUBLOGS true


/* Arguments, that control how string is interpreted, are chars/strings,
 * not template types. */

bool rescueCin() {
	std::cin.clear();
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return true;
}

template<typename T> bool typeDependentInput(T& data, char) {
	std::cin >> data;
	return true;
}

// The second parameter is hold for compatibility
template<> bool typeDependentInput(std::string& str, char) {
	rescueCin();
	std::getline(std::cin, str);
	return true;
}

// Extracts words to array, breaks if delim character is found.
template<> bool typeDependentInput(strarr& arr, char delim) {
	std::string word;
	for(uint32_t i = 0; i < 1000; ++i) {
	// for(auto& item: arr) {
		if(std::cin.peek() == delim && i > 0) break;
		std::cin >> word;
		arr.push_back(word);
	}
	return true;
}

template<typename T> bool input(T& data) {
	// rescueCin();
	typeDependentInput(data, '\n');
	if(std::cin.fail()) {
		rescueCin();
		return false;
	} else if(!std::cin.good()) { // bad or eof
		throw std::runtime_error("Some error on console input.");
	}
	return true;
}

// Gives additional attemps to user.
template<typename T> bool loopedInput(T& data) {
	for(uint32_t i = 0; i < 3; ++i) {
		// input goes to referenced variable
		if(!input(data)) {
			std::cout << "Uncorrect data, please try again." << std::endl;
		} else return true;
	}
	throw std::runtime_error("Input request dismissed.");
}

// Variant with anonymous function as validator.
template<typename T> bool loopedInput(T& data, std::function<bool(T&)>& validator) {
	for(uint32_t i = 0; i < 3; ++i) {
		// input goes to referenced variable
		if(!input(data) || !validator(data)) {
			std::cout << "Uncorrect data, please try again." << std::endl;
		} else return true;
	}
	throw std::runtime_error("Input request dismissed.");
}

strarr getPassword() {
	std::cout << "Provide the password (no longer than 40 characters or words):" << std::endl;
	strarr pswd;
	loopedInput(pswd);
	// This check will work good both for strings and arrays.
	if(!pswd.size()) {
		throw std::runtime_error("Password is empty.");
	} else if(pswd.size() > 40 || pswd.at(0).size() > 40) {
		throw std::runtime_error("Password too long.");
	}
	return pswd;
}

// template: Only strings and arrays of strings supported
template<> strarr getPossibleWords(strarr& arr) {
	auto copy = arr;
	// Sorting shuffles characters, and allows to remove all duplicates by std::unique
	// NOTE: only characters and strings, if the chain contains other data, it may not work.
	std::sort(copy.begin(), copy.end());
	std::unique(copy.begin(), copy.end());
	return copy;
}

// template: Only strings and arrays of strings supported
// NOTE: it looks like std::unique does not work for strings.
template<> std::string getPossibleWords(std::string& str) {
	std::string filtered;
	for(auto word: str) {
		auto pos = std::find(filtered.begin(), filtered.end(), word);
		if(pos == filtered.end())
			filtered += word;
		else continue;
	}
	return filtered;
}

// Gets min and max length hits from user and manages those inputs.
template<typename Chain> bool getLenHint(Args<Chain>& args, uint32_t pswdLength) {
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
	return true;
}

// Converts time returned by clock() to miliseconds. It may be system–dependent.
double getMs(double raw) {
	return raw / (CLOCKS_PER_SEC / 1000);
}

// ················· //

void welcome() {
	std::cout << "This is a simple password guesser.\n" << std::endl;
}

void logActionStart(uint32_t possibleItems, uint32_t length) {
	std::cout << "Testing for " << length << " length: "
		<< pow(possibleItems, length) << " possible combinations... " << std::endl;
}

void logActionEnd(double ms) {
	std::cout << " ...took " << ms << " ms." << std::endl;
}

void showResult(bool found, double& ms) {
	std::cout << (found ? "Guessed!" : "Not guessed.") << std::endl;
	std::cout << "Operation took " << ms << " miliseconds." << std::endl;
}

// ················· //

// This function calls match() for the subsequent values of password length.
// Note: There's no need to return the password, as user will know it.
template<typename Chain>
bool guess(Args<Chain>& args, const std::function<bool(Chain&)>& verifier) {
	for(uint32_t i = args.minLength; i <= args.maxLength; ++i) {
		Chain c; // it can be returned.
		// It can be turned off, to improve overall speed
		#if SUBLOGS
			logActionStart(args.possibleItems.size(), i);
			clock_t start = clock();
		#endif
		
		bool result = match(args.possibleItems, i, c, 0, verifier);
		
		#if SUBLOGS
			clock_t end = clock();
			logActionEnd(getMs(end - start));
		#endif
		
		if(result) return true;
	}
	return false;
}

// Checks all combinations for given password length.
// Real result can be accessed by reference.
template<typename Chain>
bool match(Chain& possibleItems, uint32_t length, Chain& tested, uint32_t pswdPos, const std::function<bool(Chain&)>& verify) {
	if(!tested.size()) { // not initialized yet
		tested = Chain(length, possibleItems[0]);
	}
	// For each available item, insert it at current position, and...
	for(uint32_t i = 0; i < possibleItems.size(); ++i) {
		tested[pswdPos] = possibleItems[i];
		bool result;
		if(pswdPos + 1 >= length) { // it's the last position, verify
			result = verify(tested);
		} else { // it is not the last position, dig deeper
			result = match(possibleItems, length, tested, pswdPos + 1, verify);
		}
		if(result) return result;
		// else continue
	}
	return false;
}

// ················· //

template<typename Chain> bool getHints(Args<Chain>& hints, Chain& pswd) {
	getLenHint(hints, pswd.size());
	hints.possibleItems = getPossibleWords<Chain>(pswd);
	return true;
}

template<typename Chain> bool runGuesswork(Chain& pswd, double& ntime) {
	Args<Chain> hints;
	getHints(hints, pswd);
	// anon function
	std::function<bool(Chain&)> verifier = [pswd](Chain& proposition) -> bool {
		if(pswd.size() != proposition.size()) return false; // faster
		return std::equal(pswd.begin(), pswd.end(), proposition.begin());
	};
	// start timer and go
	clock_t before = clock();
	bool result = guess(hints, verifier);
	clock_t after = clock();
	ntime = getMs(after - before);
 	return result;
}

bool turn(double& ntime) {
	strarr arr = getPassword();
	if(arr.size() > 1) {
		return runGuesswork<strarr>(arr, ntime);
	} else if(arr.size() == 1) {
		return runGuesswork<std::string>(arr[0], ntime);
	} else throw std::logic_error("[bool turn()]: Unexpected contition!");
}

bool startTurn() {
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

bool programLoop() {
	while(true) {
		if(!startTurn())
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
