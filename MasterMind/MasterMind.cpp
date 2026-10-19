#include <algorithm>
#include <cctype>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>
using std::cerr;
using std::cin;
using std::copy;
using std::cout;
using std::endl;
using std::find;
using std::getline;
using std::invalid_argument;
using std::istream_iterator;
using std::istringstream;
using std::mt19937;
using std::ostream;
using std::ostream_iterator;
using std::out_of_range;
using std::random_device;
using std::shuffle;
using std::string;
using std::uniform_int_distribution;
using std::vector;


static void getUnsignedInt(const string& message, size_t& result, const size_t minValue = 0, const size_t maxValue = SIZE_MAX) {
	bool inputError;
	do {
		inputError = false;
		cout << message;
		string line;
		getline(cin, line);
		if (line != "") {
			try {
				result = stoull(line);
			}
			catch (const invalid_argument& ia) {
				inputError = true;
				cerr << "Invalid argument: " << ia.what() << '\n';
			}
			catch (const out_of_range& oor) {
				inputError = true;
				cerr << "Out of range: " << oor.what() << '\n';
			}
			if (result < minValue) {
				cerr << "Sorry, the minimum acceptable value for this input is " << minValue << ".\n";
			}
			if (result > maxValue) {
				cerr << "Sorry, the maximum acceptable value for this input is " << maxValue << ".\n";
			}
		}
	} while (inputError || result > maxValue || result < minValue);
}


template<class T>
void grammaticallyPrintVectorElements(ostream& os, const vector<T>& v, const string& coordinatingLogicalOperator) { //the coordinatingLogicalOperator is not confirmed to be such, but simply takes that position in the printed list
	switch (v.size()) {
	default:
		copy(v.begin(), v.end() - 2, ostream_iterator<T>(os, ", "));
	case 2:
		os << v[v.size() - 2] << (v.size() > 2 ? ", " : " ");
		if (!coordinatingLogicalOperator.empty()) {
			os << coordinatingLogicalOperator << ' ';
		}
	case 1:
		os << v.back();
	case 0:
		break;
	}
}


template<class T>
void printAnswer(ostream& os, const vector<T>& answer, const bool presentTense = true) {
	os << "The correct answer " << (presentTense ? "is" : "was") << ' ';
	ostream_iterator<T> answerOutputIterator(os, " ");
	copy(answer.begin(), answer.end(), answerOutputIterator);
	os << endl;
}


int main() { //TODO: allow command-line arguments
	vector<string> colors = { "black", "blue", "green", "red", "white", "yellow", "blank" };
	size_t numPinColors = 6, numPinsPerGuess = 4, numGuesses = 10;
	const vector<string> cheatCodes = { "answer", "hint", "quit" }; //TODO: pair to functions?

	getUnsignedInt("How many pin colors do you want to play with (default is 6)? ", numPinColors, 1, colors.size());
	colors.resize(numPinColors);
	cout << "The color option" << (colors.size() == 1 ? " is " : "s are ");
	grammaticallyPrintVectorElements(cout, colors, "and");
	cout << ". Caps are irrelevant in your guesses.\n";
	getUnsignedInt("How many pins per guess do you want to play with (default is 4)? ", numPinsPerGuess, 1, SIZE_MAX);
	getUnsignedInt("How many guesses do you want to have (default is 10)? ", numGuesses);
	//TODO: allow specifying whether the player is the sequence producer (with an AI guesser) or guesser

	uniform_int_distribution<size_t> pick(0, colors.size() - 1);
	vector<string> answer;
	answer.reserve(numPinsPerGuess);
	random_device randomDevice;
	mt19937 generator(randomDevice());
	for (size_t i = 0; i < numPinsPerGuess; i++) {
		answer.push_back(colors[pick(generator)]);
	}

	cout << "Begin guessing:\n";
	for (size_t guessNumber = 0; guessNumber < numGuesses; ) {
		string line;
		getline(cin, line);
		for (size_t i = 0; i < line.size(); i++) { //make caps irrelevant
			line[i] = tolower(line[i]);
		}
		istringstream iss(line);
		vector<string> guess(istream_iterator<string>(iss), {});

		if (guess.size() == 2 && guess[0] == cheatCodes[1]) { //cheatCodes[1] == "hint"
			try {
				const size_t hintIndex = stoull(guess[1]);
				if (hintIndex == 0 || hintIndex > answer.size()) {
					throw out_of_range("hint index argument out of range");
				}
				cout << "Your hint: pin " << hintIndex << " is " << answer[static_cast<size_t>(hintIndex) - 1] << endl;
				continue;
			}
			catch (const invalid_argument&) {}
			catch (const out_of_range&) {}
		}
		else if (guess.size() != numPinsPerGuess) {
			if (guess.size() == 1 && find(cheatCodes.begin(), cheatCodes.end(), guess[0]) != cheatCodes.end()) {
				if (guess[0] == cheatCodes[0]) { //cheatCodes[0] == "answer"
					printAnswer(cout, answer);
					continue;
				}
				else if (guess[0] == cheatCodes[1]) { //cheatCodes[1] == "hint"
					uniform_int_distribution<size_t> distribution(0, answer.size() - 1);
					const size_t hintIndex = distribution(generator);
					cout << "Your hint: pin " << hintIndex + 1 << " is " << answer[hintIndex] << endl;
					continue;
				}
				else { //if (guess[0] == cheatCodes[2]) //cheatCodes[2] == "quit"
					cout << "Sorry, you lost. ";
					printAnswer(cout, answer, true);
					return 0;
				}
			}
			cerr << "Error: the number of guessed colors didn't match the number of pins per guess.\n";
		}
		else {
			vector<string> tempAnswerForComparing = answer;

			//determine whether there is any invalid color option
			bool colorError = false;
			for (size_t i = 0; i < tempAnswerForComparing.size(); i++) {
				if (find(colors.begin(), colors.end(), guess[i]) == colors.end()) { //if isn't a valid color
					cerr << "Error: " << guess[i] << " is not a valid color (at least in this game). The valid color option" << (colors.size() == 1 ? " is " : "s are ");
					grammaticallyPrintVectorElements(cerr, colors, "and");
					cerr << ".\n";
					colorError = true;
					continue;
				}
			}

			if (!colorError) {
				vector<char> response;

				//determine the need for any black ('b') response pins
				for (size_t i = 0; i < tempAnswerForComparing.size(); i++) {
					if (find(colors.begin(), colors.end(), guess[i]) == colors.end()) { //if isn't a valid color
						cerr << "Error: " << guess[i] << " is not a valid color (at least in this game). The valid color option" << (colors.size() == 1 ? " is " : "s are ");
						grammaticallyPrintVectorElements(cerr, colors, "and");
						cerr << ".\n";
						colorError = true;
						continue;
					}

					if (guess[i] == tempAnswerForComparing[i]) {
						response.push_back('b');
						guess.erase(guess.begin() + i);
						tempAnswerForComparing.erase(tempAnswerForComparing.begin() + i);
						i--;
					}
				}

				//determine the need for any white ('w') response pins
				for (size_t i = 0; i < tempAnswerForComparing.size(); i++) {
					for (size_t j = 0; j < guess.size(); j++) {
						if (guess[j] == tempAnswerForComparing[i]) {
							response.push_back('w');
							guess.erase(guess.begin() + j);
							tempAnswerForComparing.erase(tempAnswerForComparing.begin() + i);
							i--;
							break;
						}
					}
				}

				if (response == vector<char>(numPinsPerGuess, 'b')) {
					cout << "Congratulations! You won!\n";
					return 0;
				}
				else {
					if (response.empty()) {
						cout << "Nothing correct.";
					}
					else {
						cout << "Your guess " << guessNumber + 1 << '/' << numGuesses << ": ";
						//response pins come in alphabetical order to reveal nothing
						ostream_iterator<char> responseOutputIterator(cout);
						copy(response.begin(), response.end(), responseOutputIterator);
					}
					cout << endl;
					guessNumber++;
				}
			}
		}
	}

	cout << "Sorry, you lost. ";
	printAnswer(cout, answer, true);
	return 0;
}
