#include "ioHandle.h"
#include "savStructure.h"

// Prints a welcome message at program execution
void printWelcome () {
	clearScreen();
	cout << "\n\n\n";
	for (int i = 0; i < 80; i++)
		cout << '=';
	cout << "\n\n";
	for (int i = 0; i < 33; i++)
		cout << ' ';
	cout << "[Gen3SaveEditor]\n\n";
	for (int i = 0; i < 80; i++)
		cout << '=';
	cout << "\n\n\n";
}

// Clear the screen by printing newlines
void clearScreen () {
	for (int i = 0; i < 50; i++)
		cout << '\n';
}

// Main menu and main loop of program
void mainMenu () {

	// Declare program variables
	bool programExecution = true;
	string menuInput = "", errMessage = "";
	vector <string> commandSequence;
	SavFile gameSave;
	
	do {
		commandSequence.clear();
		cout << "\n MENU > ";
		getline (cin, menuInput);

		// Error detection for stdin
		if (parseInput (commandSequence, menuInput, ' ') < 1) {
			errMessage = "Input must be greater than zero!";
			printError (errMessage);
			continue;
		}

		if ((commandSequence [0] == "quit") || (commandSequence [0] == "exit"))
			programExecution = false;
		else if (commandSequence [0] == "load") {
			gameSave.getFiles();
			gameSave.loadGame();
			gameSave.encodePlayerName();
			gameSave.decodeSave();
			gameSave.verifyData();
			gameSave.loadRefFiles();
		}
		else if (commandSequence [0] == "edit") {
			gameSave.editData();
		}
		else if (commandSequence [0] == "save") {
			gameSave.writeToFile();
		}
		else if (commandSequence[0] == "saveas") {
			gameSave.changeSaveTo();
			gameSave.writeToFile();
		}
		else if (commandSequence [0] == "overwrite") {
			gameSave.overwriteSave();
		}
		else if (commandSequence [0] == "help") {
			printHelp();
		}
		else if (commandSequence [0] == "clear") {
			clearScreen();
		}
		else {
			errMessage = commandSequence[0]
				+ " is not a recognized command!";
			printError(errMessage);
		}

	} while (programExecution == true);

}

// Parse input on command line into a series of commands
int parseInput (vector <string> &parsed, string input, char delimiter) {
	string temp = "";
	int vSize = 0;

	parsed.clear();

	for (int i = 0; i < input.size(); i++) {
		if (input[i] == delimiter) {
			parsed.push_back(temp);
			temp = "";
		}
		else if (i == input.size()-1) {
			temp += input[i];
			parsed.push_back(temp);
			break;
		}
		else
			temp += input[i];
	}

	vSize = parsed.size();

	return vSize;
}

// Print error message to stderr (cerr)
void printError (string errorMessage) {
	cerr << "\n[Error] " << errorMessage << "\n";
}

// Print help menu
void printHelp () {
	clearScreen();
	string version = "210101";
	bool isAlpha = true, isBeta = false;

	// Print help header
	for (int i = 0; i < 80; i++)
		cout << '=';
	cout << "\n\n";

	for (int i = 0; i < 26; i++)
		cout << ' ';
	cout << "+--------------------------+\n";
	
	for (int i = 0; i < 26; i++)
		cout << ' ';
	cout << "| Generation 3 .sav Editor |\n";

	for (int i = 0; i < 26; i++)
		cout << ' ';
	cout << "+--------------------------+\n";

	// Print current version
	for (int i = 0; i < 36; i++)
		cout << ' ';
	cout << "v. " + version + "\n";

	// Print if in alpha or beta
	if (isAlpha) {
		for (int i = 0; i < 37; i++)
			cout << ' ';
		cout << "[ALPHA]\n";
	}
	else if (isBeta) {
		for (int i = 0; i < 37; i++)
			cout << ' ';
		cout << "[BETA]\n";
	}
	cout << "\n";

	// Print author list
	for (int i = 0; i < 26; i++)
		cout << ' ';
	cout << "Written by Lieutenant Debaser\n";

	cout << '\n';
	for (int i = 0; i < 80; i++)
		cout<< '=';
	cout << '\n';
	// End help header

	// Print command information
	cout << "\nCommands:\n"
	     << "\nquit:       Quit program (does not save)."
	     << "\nload:       Load game save."
	     << "\nedit:       Edit the save data."
	     << "\nsave:       Save any changes made."
	     << "\nsaveas:     Save any changes to an alternate file name."
	     << "\noverwrite:  Overwrite the original file (not recommended)."
	     << "\nhelp:       Print this menu."
	     << "\nclear:      Clear the screen.\n";
}

// Utility function to clear cin buffer
void clearBuffer () {
	cin.clear();
	cin.ignore(9999, '\n');
}

// Print the help menu for editing information
void printEditHelp () {
	clearScreen();
	
	// Print header
	for (int i = 0; i < 80; i++)
		cout << '=';
	for (int i = 0; i < 30; i++)
		cout << ' ';
	cout << "Editing Pokemon Data\n";
	for (int i = 0; i < 80; i++)
		cout << '=';

	cout << '\n';

	// Print edit information
	cout << "\nEditing the data of a Pokemon requires an initial edit type"
	     << " followed by the"
	     << "\ndata being edited. Below is a list of edit types along with"
	     << " the corresponding"
	     << "\ninformation that can be modified with them.";

	cout << "\n\n";

	// Print edit commands
	cout << "\nset - Invokes the 'set' type of edit."
	     << "\n\t   nickname - Modify the nickname."
	     << "\n\tnationality - Modify the nationality."
	     << "\n\t   markings - Change the markings."
	     << "\n\t    species - Set the Pokemon itself."
	     << "\n\t       item - Change the held item of the Pokemon.";

	cout << "\ntoggle - Invokes the 'toggle type of edit."
	     << "\n\tshiny - Toggles whether the Pokemon is shiny or not.";

	cout << '\n';

}

// Print an ordered menu of Pokemon's information
void SavFile::printOrdered (Poke in) {
	clearScreen();
	int evSum = 0;

	// Print nickname and species
	cout << ' ' << in.decodedNickname << " (" << getSpecies(in) << ")\n\n";

	// Print trainer information	
	cout << "    Trainer: " << in.decodedOT << " ["
	     << getTrainerGender(in) << "]\n"
	     << " Trainer ID: " << getOTID(in) << "     "
	     << " Secret  ID: " << getOTSec(in) << '\n';

	// Print current friendship value with trainer
	cout << " Friendship: " << getHappiness(in) << " / 255\n";

	// Print help item (if any)
	cout << "  Held Item: " << getHeldItem(in) << '\n';

	// Print shiny status
	cout << "   Is Shiny: ";
	if (getIfShiny(in) == true)
		cout << "Yes\n";
	else
		cout << "No\n";

	// Print moves and available PP
	for (int i = 0; i < 4; i++) {
		printMove(in, i);
	}
	cout << "\n\n";

	// Print current stats (if applicable), EVs, and IVs
	cout << "          HP: ";
       	evSum += printStat(in, 0);
	cout << '\n' << "      Attack: ";
       	evSum += printStat(in, 1);
	cout << '\n' << "     Defense: ";
	evSum += printStat(in, 2);
	cout << '\n' <<  "       Speed: ";
	evSum += printStat(in, 3);
	cout << '\n' << " Sp.  Attack: ";
	evSum += printStat(in, 4);
	cout << '\n' << " Sp. Defense: ";
	evSum += printStat(in, 5);
	cout << '\n' << " Total EVs: " << evSum << " / 510";
	cout << "\n\n";
}
