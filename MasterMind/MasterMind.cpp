#include <algorithm>
#include <cctype>
#include <climits>
#include <ios>
#include <iostream>
#include <iterator>
#include <limits>
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
using std::numeric_limits;
using std::ostream;
using std::ostream_iterator;
using std::out_of_range;
using std::random_device;
using std::shuffle;
using std::streamsize;
using std::string;
using std::uniform_int_distribution;
using std::vector;


static void getUnsignedInt(const string& message, unsigned int& result, const unsigned int minValue = 0, const unsigned int maxValue = UINT_MAX) {
	bool inputError;
	do {
		inputError = false;
		cout << message;
		string line;
		getline(cin, line);
		if (line != "") {
			try {
				result = stoul(line);
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


int main() { //TODO: allow for command-line arguments
	vector<string> colors = { "black", "blue", "green", "red", "white", "yellow", "blank" };
	unsigned int numPinColors = 6, numPinsPerGuess = 4, numGuesses = 10;
	const vector<string> cheatCodes = { "answer", "hint", "quit" }; //TODO: pair to functions?

	getUnsignedInt("How many pin colors do you want to play with (default is 6)? ", numPinColors, 1, colors.size());
	colors.resize(numPinColors); //throw out the extra colors
	cout << "The color option" << (colors.size() == 1 ? " is " : "s are ");
	grammaticallyPrintVectorElements(cout, colors, "and");
	cout << ". Caps are irrelevant in your guesses.\n";
	getUnsignedInt("How many pins per guess do you want to play with (default is 4)? ", numPinsPerGuess, 1, UINT_MAX);
	getUnsignedInt("How many guesses do you want to have (default is 10)? ", numGuesses);
	//TODO: allow for specifying whether the player is the sequence producer (with an AI guesser) or guesser

	//rather inefficiently generate the random answer by multiplying the colors vector by the number of pins in the answer/guess, then randomly reordering and extracting the first few
	vector<string> answer;
	for (unsigned int i = 0; i < numPinsPerGuess; i++) {
		answer.insert(answer.end(), colors.begin(), colors.end());
	}
	random_device randomDevice; //to seed the constructor of the random engine to create different sequences of shuffles each time the program is run
	mt19937 generator(randomDevice()); //to reuse throughout multiple shuffles to generate different permutations every time
	shuffle(answer.begin(), answer.end(), generator);
	answer.resize(numPinsPerGuess);

	cout << "Begin guessing:\n";
	for (unsigned int guessNumber = 0; guessNumber < numGuesses; ) {
		string line;
		getline(cin, line);
		for (unsigned int i = 0; i < line.size(); i++) { //make caps irrelevant
			line[i] = tolower(line[i]);
		}
		istringstream iss(line);
		vector<string> guess(istream_iterator<string>(iss), {});

		if (guess.size() == 2 && guess[0] == cheatCodes[1]) { //cheatCodes[1] == "hint"
			try {
				const unsigned int hintIndex = stoul(guess[1]);
				if (hintIndex == 0 || hintIndex >= answer.size()) {
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
					uniform_int_distribution<unsigned int> distribution(0, answer.size() - 1);
					const unsigned int hintIndex = distribution(generator);
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
			for (unsigned int i = 0; i < tempAnswerForComparing.size(); i++) {
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
				for (unsigned int i = 0; i < tempAnswerForComparing.size(); i++) {
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
				for (unsigned int i = 0; i < tempAnswerForComparing.size(); i++) {
					for (unsigned int j = 0; j < guess.size(); j++) {
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
						//response pins come in alphabetical order (revealing nothing)
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
