#include "savStructure.h"
#include "ioHandle.h"

// Get the save file along with the player name and gender
void SavFile::getFiles () {

	bool entry = true, getName = true, getGender = true, getConfirmation = true;
	string errMessage = "";
	char confirmation = ' ';
	int tempGender = 0;

	do {
		cout << "\nEnter the name of the save file to be loaded.\n";
		cout << "\n LOAD > ";
		getline(cin, loadFromFile);

		// Get player's name for later
		getName = true;
		cout << "\nEnter player name on save file.\n";

		do {
			cout << "\n LOAD > ";
			getline (cin, playerName);
			if (playerName.size() > 7) {
				errMessage = "Player name cannot be longer than seven characters!";
				printError(errMessage);
			}
			else
				getName = false;			 
		} while (getName == true);

		// Get player's gender for later

		getGender = true;
		cout << "\nSelect player's gender on save file."
		     << "\n[0] Male"
		     << "\n[1] Female\n";

		do {
			cout << "\n LOAD > ";
			if (!(cin >> tempGender)) {
				cin.clear();
				cin >> errMessage;
				errMessage += " is not an integer.";
				printError (errMessage);
			}
			else if ((tempGender > 1) || (tempGender < 0)) {
				errMessage = "Must enter 0 (M) or 1 (F).";
				printError (errMessage);
			}
			else
				getGender = false;
		} while (getGender == true);

		playerGender = tempGender;

		// Confirm if information is correct

		cout << "\nIs this information correct? (y/n)\n"
		     << "\nSave File:     " << loadFromFile
		     << "\nPlayer Name:   " << playerName
		     << "\nPlayer Gender: ";
		if (playerGender == 0x00)
			cout << "Male\n";
		else
			cout << "Female\n";

		do {
			cout << "\n LOAD > ";
			if (!(cin >> confirmation)) {
				cin.clear();
				cin >> errMessage;
				errMessage += " is not 'y' or 'n'.";
				printError (errMessage);
			}
			else if (confirmation == 'y') {
				getConfirmation = false;
				entry = false;
			}
			else if (confirmation == 'n')
				getConfirmation = false;
			else {
				errMessage = "Must enter 'y' or 'n' only.";
				printError (errMessage);
			}
			cin.ignore (255, '\n');
		} while (getConfirmation == true);
	} while (entry == true);
}

// Actually load the save from the input file
void SavFile::loadGame () {

	string errMessage = "";
	isLoaded = false;
	isDecoded = false;
	isModified = false;

	// Stream to read in data from file
	ifstream readSav;
	readSav.open(loadFromFile.c_str(), ios::binary);

	// Unable to open given save file
	if (!(readSav.is_open())) {
		errMessage = "Could not open ";
		errMessage += loadFromFile;
		errMessage += " as it does not exist.";
		printError(errMessage);
	}

	// Open given save file, read in data, and close stream
	else {

		for (int i = 0; i < MAX_SIZE; i++)
			readSav >> hex >> noskipws >> savArray[i];

		readSav.close();

		saveToFile = "edited_" + loadFromFile;
		isLoaded = true;
	}
}

// Load in the reference files used in the program
void SavFile::loadRefFiles () {
	// Declare file information
	string pokeFile = "data/pokeList.dat", locFile = "data/locList.dat",
	       itemFile = "data/itemList.dat", moveFile = "data/moveList.dat";
	const int INFILEQUANT = 4;

	ifstream readRefFile;
	string inFile = "data/nonexistentfile.dat";
	bool readingFile = true;

	vector <GenericInfo> tempList;
	GenericInfo tempStruct;
	string errMessage = "", fLine = "";

	cout << "\nReading reference files... ";

	// Reset stored information for new reading
	pokeList.clear();
	locList.clear();
	itemList.clear();
	moveList.clear();

	for (int i = 0; (i < INFILEQUANT) && (isLoaded == true); i++) {
		// Determine which file is to be read in
		switch(i) {
			case 0:
				inFile = pokeFile;
				break;
			case 1:
				inFile = locFile;
				break;
			case 2:
				inFile = itemFile;
				break;
			case 3:
				inFile = moveFile;
				break;
			default:
				errMessage = "More requests for ref files than expected.";
				printError(errMessage);
				i = 1000;
		}

		if (i > INFILEQUANT)
			break;
		else {
			readRefFile.open(inFile.c_str());
			if (!(readRefFile.is_open())) {
				errMessage = "Could not open " + inFile + "!";
				printError(errMessage);
				readRefFile.clear();
				readRefFile.seekg(0, ios::beg);
				isLoaded = false;
				continue;
			}
			else {
				readingFile = true;
			}
		}

		string IDasString = "";
		stringstream IDtoInt;
		int tempInt = 0;
		
		while(readingFile) {
			getline(readRefFile, fLine);

			if (readRefFile.eof()) {
				readingFile = false;
				break;
			}
			for (int j = 0; j < fLine.size(); j++) {
				if (fLine[j] == ' ') {
					fLine.erase(fLine.begin(), fLine.begin()+j+1);
					break;
				}
				else {
					IDasString += fLine[j];
				}
			}

			IDtoInt << IDasString;
			IDasString = "";
			IDtoInt >> tempInt;
			stringstream().swap(IDtoInt);

			tempStruct.ID = tempInt;
			tempStruct.name = fLine;

			tempList.push_back(tempStruct);
		}
		
		switch(i) {
			case 0:
				pokeList = tempList;
				break;
			case 1:
				locList = tempList;
				break;
			case 2:
				itemList = tempList;
				break;
			case 3:
				moveList = tempList;
				break;
		}

		tempList.clear();

		readRefFile.clear();
		readRefFile.seekg(0, readRefFile.beg);
		readRefFile.close();
	}

	cout << "DONE!\n";
}

// Determine starting location of oldest save (game will default to most recent
// save if the modified save can't be loaded, preventing loss of data)
bool SavFile::decodeSave() {

	string errMessage = "";

	// Test to see if the save has already been decoded
	if (isDecoded == true) {
		errMessage = "Save has already been decoded!";
		printError(errMessage);
		return false;
	}

	vector <int32_t> trainerStarts;

	cout << "\nDecoding save... ";

	// Basic search for trainer location
	for (int i = 0; i < MAX_SIZE; i++) {
		if (savArray[i] == encodedName[0]) {
			for (int j = 1; j < 8; j++) {
				if (j+i >= MAX_SIZE)
					break;
				else if (savArray[j+i] != encodedName[j]) {
					break;
				}
				else if (j == 7) {
					j++;
					if (savArray[j+i] == playerGender)
						trainerStarts.push_back(i);
				}
			}
		}
	}

	// Do advanced search if too many possible trainer locations
	if (trainerStarts.size() > 2) {
		cout << "Performing a deeper search... ";
		for (int i = 0; i < trainerStarts.size(); i++) {
			for (int j = trainerStarts[i]+9; j < trainerStarts[i]+13; j++) {
				if (savArray[j] != 0x00)
					break;
				else if (j == trainerStarts[i]+12) {
					trainerStarts.erase(trainerStarts.begin()+i);
				}
			}
		}

		// Advanced search failed
		if (trainerStarts.size() != 2) {
			errMessage = "Deeper search has failed.";
			printError(errMessage);
			isDecoded = false;
		}

		// Advanced search was successful
		else {
			cout << "DONE!\n";
			isDecoded = true;
		}
	}

	// Unable to find the two requisite saves
	else if (trainerStarts.size() < 1) {
		errMessage = "Could not resolve player information.";
		printError(errMessage);
		isDecoded = false;
	}

	// Decoding save was successful
	else {
		cout << "DONE!\n";
		isDecoded = true;
	}

	// If decoding was successful: Get save structures
	if (isDecoded == true) {

		// Find the value of the oldest save index
		string saveAIndex = "", saveBIndex = "";
		int tempLocInt = 0;
		for (int i = 0; i < 4; i++) {
			tempLocInt = (trainerStarts[0] + 4096) - i;
			saveAIndex += savArray[tempLocInt];

			tempLocInt = (trainerStarts[1] + 4096) -i;
			saveBIndex += savArray[tempLocInt];
		}

		int32_t refLocation = 0x00;

		// Put the location of the oldest save in the structure
		if (saveAIndex < saveBIndex) {
			refLocation = trainerStarts[0];
		}
		else {
			refLocation = trainerStarts[1];
		}

		// Find the structures with the same save index
		int arrayCount = 0;

		for (int i = 0; i < MAX_SIZE; i += 4096) {
			if (savArray[i+4095] == savArray[refLocation+4095]) {
				for (int j = 4095; j > 4091; j--) {

					if (savArray[i+j] != savArray[refLocation+j])
						break;
					else if (j == 4092) {
						sectionLocations[arrayCount] = i;
						arrayCount++;
					}

				}
			}
		}


		// Each save must have 14 save structures with the same index
		if (arrayCount != 14) {
			errMessage = "Could not parse save data structures.";
			printError(errMessage);
			isDecoded = false;
		}

		// Sort save structures based on index ID
		int16_t locA = 0x0000, locB = 0x0000;
		for (int i = 0; i < 13; i++) {
			locA = savArray[sectionLocations[i]+4085];
			locA = locA<<8;
			locA += savArray[sectionLocations[i]+4084];

			locB = savArray[sectionLocations[i+1]+4085];
			locB = locB<<8;
			locB += savArray[sectionLocations[i+1]+4084];

			if (locA > locB) {
				swap (sectionLocations[i], sectionLocations[i+1]);
				i -= 2;
				if (i < -1)
					i = -1;
			}
		}

		// Get game code and security key (if applicable)
		int8_t gameCodeByte = 0x00;
		for (int i = 0; i < 4; i++) {
			gameCode = gameCode<<8;
			gameCodeByte = savArray[sectionLocations[0]+172+i];
			gameCode += gameCodeByte;
		}
		
		// Determine the game the save belongs to and verify key
		int32_t verifyKey = 0x00;

		// Sapphire and Ruby
		if (gameCode == 0x00) {
			securityKey = 0x00;
			cout << "\nGame identified as Ruby or Sapphire.\n";
		}

		// FireRed and LeafGreen
		else if (gameCode == 0x01) {
			for (int i = 0; i < 4; i++) {
				securityKey = securityKey<<8;
				gameCodeByte = savArray[sectionLocations[0]+2808+i];
				securityKey += gameCodeByte;
			}
			for (int i = 3872; i < 3876; i++) {
				verifyKey = verifyKey<<8;
				gameCodeByte = savArray[sectionLocations[0]+i];
				verifyKey += gameCodeByte;
			}
			if (securityKey == verifyKey) {
				cout << "\nGame has been identified as FireRed or LeafGreen.\n";
			}
			else {
				securityKey = 0xffffffff;
				gameCode = 0x00000000;
				errMessage = "Game identity could not be verified.";
				printError(errMessage);
				isDecoded = false;
			}
		}

		// Emerald (which is a fall-back)
		else {
                        securityKey = gameCode;
                        for (int i = 500; i < 504; i++) {
                                verifyKey = verifyKey<<8;
                                gameCodeByte = savArray[sectionLocations[0]+i];
                                verifyKey += gameCodeByte;
                        }
                        if (securityKey == verifyKey) {
                                cout << "\nGame has been identified as Emerald.\n";
                        }
                        else {
                                securityKey = 0xffffffff;
                                gameCode = 0x00000000;
                                errMessage = "Game identity could not be verified";
                                printError(errMessage);
                                isDecoded = false;
                        }
                }

		// Stop decoding if the save wasn't able to be determined
		if (isDecoded == false)
			return false;
		
		// Increment save index by 2 for the older save
		int32_t saveIndex = 0x00000000;
		for (int i = 4; i > 0; i--) {
			saveIndex = saveIndex<<8;
			saveIndex += savArray[sectionLocations[0]+4091+i];
		}
		saveIndex += 2;

		int8_t saveIndexByte = 0x00;

		for (int i = 0; i < 4; i++) {
			saveIndexByte = saveIndex;
			saveIndex = saveIndex>>8;
			for (int j = 0; j < 14; j++) {
				savArray[sectionLocations[j]+4092+i] = saveIndexByte;
			}
		}

	}
}

// Convert the player name to the character encoding used by the game
void SavFile::encodePlayerName () {
	for (int i = 0; i < playerName.size(); i++) {

		// Upper-Case Letters
		if ((playerName[i] <= 90) && (playerName[i] >= 65))
			encodedName[i] = playerName[i] + 122;

		// Lower-Case Letters
		else if ((playerName[i] <= 122) && (playerName[i] >= 97))
			encodedName[i] = playerName[i] + 116;

		// Numbers
		else if ((playerName[i] <= 57) && (playerName[i] >= 48))
			encodedName[i] = playerName[i] + 113;

		// Special Characters
		// Does not currently cover all possible special characters
		// due to the limitations of ASCII compared to the game's
		// encoding scheme
		else {
			switch (playerName[i]) {
				case 32:	// Space
					encodedName[i] = 0x00;
					break;
				case 33:	// !
					encodedName[i] = 0xAB;
					break;
				case 63:	// ?
					encodedName[i] = 0xAC;
					break;
				case 46:	// .
					encodedName[i] = 0xAD;
					break;
				case 45:	// -
					encodedName[i] = 0xAE;
					break;
				case 22:	// "
					encodedName[i] = 0xB1;
					break;
				case 39:	// '
					encodedName[i] = 0xB3;
					break;
				case 44:	// ,
					encodedName[i] = 0xB8;
					break;
				case 47:	// /
					encodedName[i] = 0xBA;
					break;
				

				default:
					encodedName[i] = 0xFF;
			}
		}
	}

	for (int i = playerName.size(); i < 8; i++)
		encodedName[i] = 0xFF;
}

// Convert player name from propreitary encoding to ASCII
void SavFile:: decodePlayerName() {

	playerName = "";

	for (int i = 8; i < 8; i++) {
		cout << dec << encodedName[i];
	}

	for (int i = 0; i < 7; i++) {
		if ((encodedName[i] >= 187) && (encodedName[i] <= 212))
			playerName += encodedName[i] - 122;
		else if ((encodedName[i] >= 213) && (encodedName[i] <= 238))
			playerName += encodedName[i] - 116;
		else if ((encodedName[i] >= 161) && (encodedName[i] <= 170))
			playerName += encodedName[i] - 113;
		else {
			switch (encodedName[i]) {
				case (0x00): // Space
					playerName += 32;
					break;
				case (0xAB): // !
					playerName += 33;
					break;
				case (0xAC): // ?
					playerName += 63;
					break;
				case (0xAD): // .
					playerName += 46;
					break;
				case (0xAE): // -
					playerName += 45;
					break;
				case (0xB1): // open "
				case (0xB2): // close "
					playerName += 22;
					break;
				case (0xB3): // open '
				case (0xB4): // close '
					playerName += 39;
					break;
				case (0xB5): // MALE SYMBOL
				case (0xB6): // FEMALE SYMBOL
					playerName += '~';
					break;
				case (0xB8): // ,
					playerName += 44;
					break;
				case (0xBA): // /
					playerName += 47;
					break;
				case (0xF1): // *A*
				case (0xF2): // *O*
				case (0xF3): // *U*
				case (0xF4): // *a*
				case (0xF5): // *o*
				case (0xF6): // *u*
					playerName += '&';
					break;
				
				default: // Assumes 0xFF if unable to decode
					i = 100;
					break;
			}
		}
	}
}

// Begin subroutine to edit save data
void SavFile::editData () {

	string errMessage = "";

	// If the save hasn't been decoded yet
	if (isDecoded == false) {
		errMessage = "Game data has not been decoded yet!";
		printError(errMessage);
	}

	// If a save hasn't been loaded yet
	else if (isLoaded == false) {
		errMessage = "Game save has not been loaded yet!";
		printError(errMessage);
	}

	// If the save has been decoded
	else {
		cout << "\nSelect what to edit:\n\n"
		     << "[1] Trainer Data\n"
		     << "[2] Team Data\n"
		     << "[3] PC Box Data\n\n";

		bool choosing = true;
		char choice = ' ';

		// Edit menu to choose what to edit
		do {
			cout << "\n EDIT > ";
			if (!(cin >> choice)) {
				errMessage = "Selection must be an integer.";
				printError(errMessage);
				cin.clear();
				cin.ignore(255, '\n');
			}
			else {
				switch (choice) {
					case '1':
						isModified = editTrainer();
						choosing = false;
						break;
					case '2':
						isModified = editTeam();
						choosing = false;
						break;
					case '3':
						isModified = editPC();
						choosing = false;
						break;
					default:
						errMessage = "Selection must be between 1 and 3";
						printError(errMessage);
				}
			}
			cin.ignore(255, '\n');
		} while (choosing == true);

		// If changes have been made, recalculate checksums
		if (isModified == true) {
			clearScreen();
			verifyData();
		}
	}
}

// Edit trainer data
bool SavFile::editTrainer() {
	bool changesMade = false, editing = true;
	string longChoice = "";
	int shortChoice = 0;

	string errMessage = "";

	printPlayer();

	// Edit portion
	do {
		cout << "\n[1] name"
		     << "\n[2] gender"
		     << "\n[3] trainerid"
		     << "\n[4] secretid"
		     << "\n[5] inventory\n";
		cout << "\n EDIT > ";

		// If what was entered was not an integer
		if (!(cin >> shortChoice)) {
			cin.clear();
			cin >> longChoice;
			if (longChoice == "name") {
				setPlayerName();
				changesMade = true;
			}
			else if (longChoice == "gender") {
				toggleGender();
				changesMade = true;
			}
			else if (longChoice == "trainerid") {
				changesMade = true;
			}
			else if (longChoice == "secretid") {
				changesMade = true;
			}
			else if (longChoice == "inventory") {
				changesMade = true;
			}
			else if (longChoice == "exit") {
				editing = false;
			}
			else {
				errMessage = longChoice + " is not a command.";
				printError(errMessage);
			}
		}

		// If what was entered was an integer
		else {
			switch (shortChoice) {
				case 1:
					setPlayerName();
					changesMade = true;
					break;
				case 2:
					toggleGender();
					changesMade = true;
					break;
				case 3:
					changesMade = true;
					break;
				case 4:
					changesMade = true;
					break;
				case 5:
					changesMade = true;
					break;
				default:
					errMessage = shortChoice + " is not a valid choice.";
					printError(errMessage);
			}
		}
	} while (editing == true);

	return changesMade;
}

// Print the current player information
void SavFile::printPlayer () {

	// Player name
	decodePlayerName();
	cout << "\nTrainer Name: " << playerName
	     << "\nGender:       ";
	// Player gender
	if (playerGender == 0x00)
		cout << "Male";
	else
		cout << "Female";

	uint8_t byteGet = 0x00;
	uint16_t wordGet = 0x0000;

	// Trainer ID and secret ID
	for (int i = 11; i > 9; i--) {
		wordGet = wordGet<<8;
		byteGet = savArray[sectionLocations[0] + i];
		wordGet += byteGet;
	}

	cout << "\nTrainer ID:   " << wordGet;
	wordGet = 0x0000;

	for (int i = 13; i > 11; i--) {
		wordGet = wordGet<<8;
		byteGet = savArray[sectionLocations[0] + i];
		wordGet += byteGet;
	}

	cout << "\nSecret ID:    " << wordGet
	     << "\nPlay Time:";
	wordGet = 0x0000;

	// Play time
	for (int i = 15; i > 13; i--) {
		wordGet = wordGet<<8;
		byteGet = savArray[sectionLocations[0] + i];
		wordGet += byteGet;
	}
	cout << "\n\tHours:   " << wordGet;
	wordGet = 0x0000;

	wordGet = savArray[sectionLocations[0]+16];
	cout << "\n\tMinutes: " << wordGet;

	wordGet = savArray[sectionLocations[0]+17];
	cout << "\n\tSeconds: " << wordGet;

	wordGet = savArray[sectionLocations[0]+18];
	cout << "\n\tFrames:  " << wordGet;

	cout << "\n\n";

}

// Get a new name for the player
void SavFile::setPlayerName() {
	bool gotName = false, confirm = false;
	string newName = "", errMessage = "";
	char answer = ' ';

	cout << "\nEnter player's new name (maximum of 7 characters).\n";

	// Loop to get new name
	do {
		cout << "\n NAME > ";
		cin >> newName;

		// Entered name is too long ( > 7)
		if (newName.size() > 7) {
			errMessage = "Name must be shorter than 8 characters!";
			printError(errMessage);
		}

		// Entered name was interpreted as valid
		else {
			// Confirmation for name
			cout << "\nIs " << newName << " okay? (y/n)\n";
			do {
				cout << "\n EDIT > ";
				if(!(cin >> answer)) {
					cin.clear();
					cin >> errMessage;
					errMessage += " is not a character!";
					printError(errMessage);
				}
				else {
					switch (answer) {
						case 'y':
							confirm = true;
							gotName = true;
							break;
						case 'n':
							confirm = true;
							break;
						default:
							errMessage = answer + " is not a valid choice!";
							printError(errMessage);
							cin.clear();
							cin.ignore(255, '\n');
					}
				}
			} while (confirm == false);
		}
	} while (gotName == false);

	// Store data and encode to game's format
	playerName = newName;
	encodePlayerName();

	// Write encoded data to stored save
	for (int i = 0; i < playerName.size(); i++)
		savArray[sectionLocations[0]+i] = encodedName[i];
	for (int i = 7-playerName.size(); i >= playerName.size(); i--)
		savArray[sectionLocations[0]+i] = 0xff;
}

// Toggle the gender of the player
void SavFile::toggleGender() {
	// Toggle gender in program data
	if (playerGender == 0x00) {
		cout << "\nGender set to Female!\n";
		playerGender = 0x01;
	}
	else {
		cout << "\nGender set to Male!\n";
		playerGender = 0x00;
	}

	// Write new gender to stored save data
	savArray[sectionLocations[0]+8] = playerGender;
}

// Change the trainer's visible ID
void SavFile::setTrainerID () {

}

// Change the trainer's secret ID
void SavFile::setSecretID () {

}

// Change the time played
void SavFile::setPlayTime () {

}

// Edit team data
bool SavFile::editTeam() {
	bool changesMade = false, editing = true;
	uint8_t byteBuffer = 0x00;
	int teamStart = 0, shortChoice = 0;
	string teamNickname [6] = {""};
	string longChoice = "", errMessage = "";

	// Team data location is dependent on the game
	if (gameCode == 0x00000001) {	// FireRed and LeafGreen
		teamStart = 52;
	}
	else {				// R-S-E
		teamStart = 564;
	}

	// Get the size of the team
	teamSize = 0x00;
	for (int i = teamStart+3; i >= teamStart; i--) {
		byteBuffer = savArray[sectionLocations[1]+i];
		teamSize += byteBuffer;
		if (i > teamStart)
			teamSize = teamSize<<8;
	}

	// Actual team data is 4 bytes after the team size
	teamStart += 4;

	printTeam(teamStart, teamNickname);

	do {
		// Get input for team choice
		cout << "\n TEAM > ";
		if (!(cin >> shortChoice)) {
			cin.clear();
			cin >> longChoice;

			// Stop editing team
			if (longChoice == "exit") {
				editing = false;
				shortChoice = -1;
			}

			// Clear the screen
			else if (longChoice == "clear") {
				clearScreen();
				shortChoice = -1;
			}

			// Print team again
			else if (longChoice == "print") {
				printTeam(teamStart, teamNickname);
				shortChoice = -1;
			}

			// Search team for entered string
			else {
				for (int i = 0; i < teamSize; i++) {
					for (int j = 0; j < teamNickname[i].size(); j++) {
						if (longChoice[j] != teamNickname[i][j]) {
							shortChoice = 0;
							break;
						}
						else if (j == teamNickname[i].size()-1) {
							shortChoice = i+1;
							i = 100;
							break;
						}
					}
				}
			}
		}

		// Do operation based on value in shortChoice
		if ((shortChoice >= 1) && (shortChoice <= teamSize)) {
			cin.clear();
			cin.ignore(255,'\n');
			editPokemon(sectionLocations[1]+teamStart+((shortChoice-1)*100), true);
			changesMade = true;
			printTeam(teamStart, teamNickname);
		}
		else if (shortChoice == -1)
			continue;
		else {
			errMessage = "That is not a valid choice.";
			printError(errMessage);
		}
	} while (editing == true);
	return changesMade;
}

// Print the player's current team
void SavFile::printTeam (int teamStart, string teamNicknames []) {
	uint8_t encodedNickname [10] = {0x00};
	string nickname = "";

	for (int i = 0; i < teamSize; i++) {
		for (int j = 0; j < 10; j++)
			encodedNickname[j] = savArray[sectionLocations[1]+(i*100)+j+teamStart+8];
		nickname = decodeArray(encodedNickname, 10);
		cout << "\n[" << i+1 << "] " << nickname;
		teamNicknames[i] = nickname;
	}
	cout << '\n';
}

void SavFile::editPokemon(uint32_t startAddr, bool isInTeam) {
	Poke temp;
	temp.isInParty = isInTeam;
	temp.startAddr = startAddr;
	string errMessage = "";

	// Get personality value and trainer ID
	for (int i = 3; i >= 0; i--) {
		temp.personality += savArray[temp.startAddr+i];
		temp.trainerID += savArray[temp.startAddr+i+4];
		if (i != 0) {
			temp.personality = temp.personality<<8;
			temp.trainerID = temp.trainerID<<8;
		}
	}

	// Get encoded name and decode
	for (int i = 0; i < 10; i++) {
		temp.encodedNickname[i] = savArray[temp.startAddr+8+i];
	}
	temp.decodedNickname = decodeArray(temp.encodedNickname, 10);

	// Get language code
	for (int i = 1; i >= 0; i--) {
		temp.languageCode = savArray[temp.startAddr+18+i];
		if (i == 1)
			temp.languageCode = temp.languageCode<<8;
	}

	// Get encoded original trainer name and decode
	for (int i = 0; i < 7; i++) {
		temp.encodedOT[i] = savArray[temp.startAddr+20+i];
	}
	temp.decodedOT = decodeArray(temp.encodedOT, 7);

	// Get markings
	temp.markings = savArray[temp.startAddr+27];

	// Get current checksum
	for (int i = 1; i >= 0; i--) {
		temp.checksum += savArray[temp.startAddr+28+i];
		if (i == 1)
			temp.checksum = temp.checksum<<8;
	}

	// Get padding
	for (int i = 1; i >= 0; i--) {
		temp.padding = savArray[temp.startAddr+30+i];
		if (i == 1)
			temp.padding = temp.padding<<8;
	}

	// Get encrypted data section
	for (int i = 0; i < 48; i++)
		temp.encodedData[i] = savArray[temp.startAddr+32+i];
	decodePokeData(temp);

	// If the pokemon is in the party, get extended information
	if (temp.isInParty == true) {
		// Get status condition
		for (int i = 3; i >= 0; i--) {
			temp.status = savArray[temp.startAddr+80+i];
			if (i != 0)
				temp.status = temp.status<<8;
		}

		// Get current level
		temp.level = savArray[temp.startAddr+84];

		// Get remaining pokerus time
		temp.pokerusTimer = savArray[temp.startAddr+85];

		// Get current stats
		for (int i = 1; i >= 0; i--) {
			temp.curHP = savArray[temp.startAddr+86+i];
			temp.totHP = savArray[temp.startAddr+88+i];
			temp.atk = savArray[temp.startAddr+90+i];
			temp.def = savArray[temp.startAddr+92+i];
			temp.spd = savArray[temp.startAddr+94+i];
			temp.spAtk = savArray[temp.startAddr+96+i];
			temp.spDef = savArray[temp.startAddr+98+i];
			if (i == 1) {
				temp.curHP = temp.curHP<<8;
				temp.totHP = temp.totHP<<8;
				temp.atk = temp.atk<<8;
				temp.def = temp.def<<8;
				temp.spd = temp.spd<<8;
				temp.spAtk = temp.spAtk<<8;
				temp.spDef = temp.spDef<<8;
			}
		}
	}

	// Declare variables for next portion of program
	bool editing = true;
	string cmdIssued = "";
	vector <string> cmdParsed;
	stringstream strToInt;

	// Alias values for exiting edit section
	string exitAlias [] = {"exit", "stop", "done", "quit"};
	const int ALIAS_SIZE = 4;

	// Clear screen and print pokemon info
	clearScreen();
	printOrdered(temp);

	// Use CLI to modify pokemon
	do {
		cmdParsed.clear();
		cout << "\n EDIT > ";
		getline(cin, cmdIssued);

		// Check if input size is zero (can occur in some situations)
		if (parseInput(cmdParsed, cmdIssued, ' ') < 1) {
			errMessage = "Input must be greater than zero!";
			printError(errMessage);
			continue;
		}

		// Print help menu
		if (cmdParsed[0] == "help")
			printEditHelp();
		
		// "set" commands
		else if (cmdParsed[0] == "set") {
			if (cmdParsed.size() < 2) {
				errMessage = "Missing argument for 'set'";
				printError(errMessage);
			}
			else if (cmdParsed[1] == "ev")
				editEfforts(temp);
			else if (cmdParsed[1] == "item")
				editItem(temp);
			else if (cmdParsed[1] == "markings")
				editMarkings(temp);
			else if (cmdParsed[1] == "move") {

				if (cmdParsed.size() < 3) {
					errMessage = "Move number needed!";
					printError(errMessage);
				}
				else {
					int tempInt = 0;
					strToInt << cmdParsed[2];
					if (!(strToInt >> tempInt)) {
						strToInt.clear();
						strToInt >> errMessage;
						errMessage += " is not a number!";
						printError(errMessage);
					}
					else if ((tempInt > 3) || (tempInt < 0)) {
						errMessage = "Move number is not valid!";
						printError(errMessage);
					}
					else
						editMove(temp, tempInt);
					strToInt.str("");
					strToInt.clear();
				}
			}
			else if (cmdParsed[1] == "nationality")
				editNationality(temp);
			else if (cmdParsed[1] == "nickname")
				editNickname(temp);
			else if (cmdParsed[1] == "species")
				editSpecies(temp);
			else {
				errMessage = "Could not resolve " + cmdParsed[1];
				printError(errMessage);
			}
		}

		// "toggle" commands
		else if (cmdParsed[0] == "toggle") {
			if (cmdParsed.size() < 2) {
				errMessage = "Missing argument for 'toggle'";
				printError(errMessage);
			}
			else if (cmdParsed[1] == "shiny")
				toggleShiny(temp);
			else {
				errMessage = "Could not resolve " + cmdParsed[1];
				printError(errMessage);
			}
		}

		// Re-print pokemon information
		else if (cmdParsed[0] == "print") {
			clearScreen();
			printOrdered(temp);
		}

		// Test for a variation of "exit", print error if not found
		else {
			for (int i = 0; i < ALIAS_SIZE; i++) {
				if (cmdParsed[0] == exitAlias[i]) {
					editing = false;
					break;
				}
				else if (i == ALIAS_SIZE-1) {
					errMessage = cmdParsed[0] + " is not a valid command.";
					printError(errMessage);
				}
			}
		}
	} while (editing == true);

	encodePokeData(temp);
	writePokemon(temp);
}

// Decode the encrypted pokemon data section
void SavFile::decodePokeData(Poke &inputPoke) {
	uint32_t tempEncrypted = 0x00, litEndPerson = 0x00, litEndTrnID = 0x00;
	uint8_t narrower = 0x00;
	uint8_t keyByte0 = 0x00, keyByte1 = 0x00, keyByte2 = 0x00, keyByte3 = 0x00;

	inputPoke.decryptionKey = inputPoke.personality^inputPoke.trainerID;

	// Break up full decryption key into 4-byte chunks
	keyByte0 = inputPoke.decryptionKey;
	keyByte1 = inputPoke.decryptionKey>>8;
	keyByte2 = inputPoke.decryptionKey>>16;
	keyByte3 = inputPoke.decryptionKey>>24;	

	// Decrypt data section of pokemon 4 bytes at a time
	for (int i = 0; i < 48; i += 4) {
		inputPoke.decodedData[i]   = inputPoke.encodedData[i]^keyByte0;
		inputPoke.decodedData[i+1] = inputPoke.encodedData[i+1]^keyByte1;
		inputPoke.decodedData[i+2] = inputPoke.encodedData[i+2]^keyByte2;
		inputPoke.decodedData[i+3] = inputPoke.encodedData[i+3]^keyByte3;
	}
	// ^ The whole decoding sequence is a bit jank; I'm sure there's a
	//   better way of doing it, but this works without too much trouble.

	getDataSectionOrder(inputPoke);	
}

// Get the order of the segments in the data section
void SavFile::getDataSectionOrder(Poke &inputPoke) {
	inputPoke.mod24Val = inputPoke.personality%24;
	// 0 = Species, XP, and Friendship
        // 1 = Move Information
        // 2 = EVs and Contest Stats
        // 3 = Origin and Characteristics
        // Determine the order of the information present in the data section
        // using the mod24 value of the personality value
        if (inputPoke.mod24Val <= 5) {
                inputPoke.datSectOrder[0] = 0;
                if (inputPoke.mod24Val <= 1) {
                        inputPoke.datSectOrder[1] = 1;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 2;
                                inputPoke.datSectOrder[3] = 3;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 3;
                                inputPoke.datSectOrder[3] = 2;
                        }
                }
                else if ((inputPoke.mod24Val == 2) || (inputPoke.mod24Val == 3)) {
                        inputPoke.datSectOrder[1] = 2;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 1;
                                inputPoke.datSectOrder[3] = 3;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 3;
                                inputPoke.datSectOrder[3] = 1;
                        }
                }
                else {
                        inputPoke.datSectOrder[1] = 3;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 1;
                                inputPoke.datSectOrder[3] = 2;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 2;
                                inputPoke.datSectOrder[3] = 1;
                        }
                }
        }
        else if ((inputPoke.mod24Val >= 6) && (inputPoke.mod24Val <= 11)) {
                inputPoke.datSectOrder[0] = 1;
                if (inputPoke.mod24Val <= 7) {
                        inputPoke.datSectOrder[1] = 0;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 2;
                                inputPoke.datSectOrder[3] = 3;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 3;
                                inputPoke.datSectOrder[3] = 2;
                        }
                }
                else if ((inputPoke.mod24Val == 8) || (inputPoke.mod24Val == 9)) {
                        inputPoke.datSectOrder[1] = 2;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 0;
                                inputPoke.datSectOrder[3] = 3;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 3;
                                inputPoke.datSectOrder[3] = 0;
                        }
                }
                else {
                        inputPoke.datSectOrder[1] = 3;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 0;
                                inputPoke.datSectOrder[3] = 2;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 2;
                                inputPoke.datSectOrder[3] = 0;
                        }
                }
        }
	else if ((inputPoke.mod24Val >= 12) && (inputPoke.mod24Val <= 17)) {
                inputPoke.datSectOrder[0] = 2;
                if (inputPoke.mod24Val <= 13) {
                        inputPoke.datSectOrder[1] = 0;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 1;
                                inputPoke.datSectOrder[3] = 3;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 3;
                                inputPoke.datSectOrder[3] = 1;
                        }
                }
                else if ((inputPoke.mod24Val == 14) || (inputPoke.mod24Val == 15)) {
                        inputPoke.datSectOrder[1] = 1;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 0;
                                inputPoke.datSectOrder[3] = 3;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 3;
                                inputPoke.datSectOrder[3] = 0;
                        }
                }
                else {
                        inputPoke.datSectOrder[1] = 3;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 0;
                                inputPoke.datSectOrder[3] = 1;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 1;
                                inputPoke.datSectOrder[3] = 0;
                        }
                }
        }
	else if ((inputPoke.mod24Val >= 18) && (inputPoke.mod24Val <= 23)) {
                inputPoke.datSectOrder[0] = 3;
                if (inputPoke.mod24Val <= 19) {
                        inputPoke.datSectOrder[1] = 0;
                        if (inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 1;
                                inputPoke.datSectOrder[3] = 2;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 2;
                                inputPoke.datSectOrder[3] = 1;
                        }
                }
                else if ((inputPoke.mod24Val == 20) || (inputPoke.mod24Val == 21)) {
                        inputPoke.datSectOrder[1] = 1;
                        if(inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 0;
                                inputPoke.datSectOrder[3] = 2;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 2;
                                inputPoke.datSectOrder[3] = 0;
                        }
                }
                else {
                        inputPoke.datSectOrder[1] = 2;
                        if(inputPoke.mod24Val % 2 == 0) {
                                inputPoke.datSectOrder[2] = 0;
                                inputPoke.datSectOrder[3] = 1;
                        }
                        else {
                                inputPoke.datSectOrder[2] = 1;
                                inputPoke.datSectOrder[3] = 0;
                        }
                }
        }
        else {
                inputPoke.datSectOrder [0] = 0;
                inputPoke.datSectOrder [1] = 1;
                inputPoke.datSectOrder [2] = 2;
                inputPoke.datSectOrder [3] = 3;
        }
        // ^ Again, definitely a better way to do this, but skill level is
        //   not there yet
}

// Encrypt the pokemon data section
void SavFile::encodePokeData(Poke &inputPoke) {
        uint32_t calcChecksum = 0x00;
	uint16_t sumSection = 0x00, finalChecksum = 0x00;
        uint8_t narrower = 0x00;
        uint8_t keyByte0 = 0x00, keyByte1 = 0x00, keyByte2 = 0x00, keyByte3 = 0x00;
        // Break up full decryption key into 4-byte chunks
        keyByte0 = inputPoke.decryptionKey;
        keyByte1 = inputPoke.decryptionKey>>8;
        keyByte2 = inputPoke.decryptionKey>>16;
        keyByte3 = inputPoke.decryptionKey>>24;

        // Reencrypt data section using decryption key
        for (int i = 0; i < 48; i += 4) {
                inputPoke.encodedData[i]   = inputPoke.decodedData[i]^keyByte0;
                inputPoke.encodedData[i+1] = inputPoke.decodedData[i+1]^keyByte1;
                inputPoke.encodedData[i+2] = inputPoke.decodedData[i+2]^keyByte2;
                inputPoke.encodedData[i+3] = inputPoke.decodedData[i+3]^keyByte3;
        }

	// Calculate checksum of new data section
	for (int i = 0; i < 24; i++) {
		sumSection = 0x0000;
		for (int j = (i*2)+1; j >= i*2; j--) {
			sumSection += inputPoke.decodedData[j];
			if (j != i*2)
				sumSection = sumSection<<8;
		}
		calcChecksum += sumSection;
	}
	finalChecksum = calcChecksum&0xFFFF;
	inputPoke.checksum = finalChecksum;
}

// Write pokemon data to save file
void SavFile::writePokemon(Poke output) {

	// Write personality value and trainerID to save
	for (int i = 3; i >= 0; i--) {
		savArray[output.startAddr+i] = output.personality>>(8*i);
		savArray[output.startAddr+i+4] = output.trainerID>>(8*i);
	}

	// Write nickname in proprietary encoding to save
	for (int i = 0; i < 10; i++)
		savArray[output.startAddr+i+8] = output.encodedNickname[i];

	// Write the language code to the save
	savArray[output.startAddr+19] = output.languageCode>>8;
	savArray[output.startAddr+18] = output.languageCode;

	// Write the trainer's name in proprietary encoding to save
	for (int i = 0; i < 7; i++)
		savArray[output.startAddr+i+20] = output.encodedOT[i];

	// Write checksum to save
	savArray[output.startAddr+29] = output.checksum>>8;
	savArray[output.startAddr+28] = output.checksum;

	// Write encoded data section to save
	for (int i = 0; i < 48; i++)
		savArray[output.startAddr+32+i] = output.encodedData[i];

	// If pokemon is in party, write current state to save
	if (output.isInParty == true) {
		for (int i = 3; i >= 0; i--)
			savArray[output.startAddr+80+i] = output.status>>(8*i);
		savArray[output.startAddr+84] = output.level;
		savArray[output.startAddr+85] = output.pokerusTimer;
		for (int i = 1; i >= 0; i--) {
			savArray[output.startAddr+86+i] = output.curHP>>(8*i);
			savArray[output.startAddr+88+i] = output.totHP>>(8*i);
			savArray[output.startAddr+90+i] = output.atk>>(8*i);
			savArray[output.startAddr+92+i] = output.def>>(8*i);
			savArray[output.startAddr+94+i] = output.spd>>(8*i);
			savArray[output.startAddr+96+i] = output.spAtk>>(8*i);
			savArray[output.startAddr+98+i] = output.spDef>>(8*i);
		}
	}
}

// Edit the pokemon nickname
void SavFile::editNickname(Poke &inputPoke) {
	string unEncodedNewName = "", errMessage = "";
	uint8_t newNameArray [10] = {0xFF};

	// Continually get the new nickname until it's size is less than 11
	do {
		cout << "\n\nThe current nickname is " 
	     	<< inputPoke.decodedNickname << ".\n" << "Enter the new nickname: ";
		getline(cin, unEncodedNewName);
		if (unEncodedNewName.size() > 10) {
			errMessage = "Nicknames can be no larger than 10 characters!";
			printError(errMessage);
		}
	} while (unEncodedNewName.size() > 10);

	// Put characters from string into array for encoding
	for (int i = 0; i < unEncodedNewName.size(); i++)
		newNameArray[i] = unEncodedNewName[i];

	// Encode new nickname into game's character encoding
	encodeArray(newNameArray, inputPoke.encodedNickname, 10);

	// Reload nickname into pokemon structure
	inputPoke.decodedNickname = decodeArray(inputPoke.encodedNickname, 10);
}

// Edit the nation of origin
void SavFile::editNationality(Poke &inputPoke) {
	bool entry = true;
	string searchTerm = "";
	uint8_t natId = 0x00;
	string errMessage = "";

	cout << "\nChoose the new nationality: "
	     << "\n japanese   " << "english"
	     << "\n french     " << "italian"
	     << "\n german     " << "korean"
	     << "\n spanish    " << '\n';
	do {
		cout << "\n NAT. > ";
		cin >> searchTerm;
		if (searchTerm == "japanese") {
			natId = 0x01;
			entry = false;
		}
		else if (searchTerm == "english") {
			natId = 0x02;
			entry = false;
		}
		else if (searchTerm == "french") {
			natId = 0x03;
			entry = false;
		}
		else if (searchTerm == "italian") {
			natId = 0x04;
			entry = false;
		}
		else if (searchTerm == "german") {
			natId = 0x05;
			entry = false;
		}
		else if (searchTerm == "korean") {
			natId = 0x06;
			entry = false;
		}
		else if (searchTerm == "spanish") {
			natId = 0x07;
			entry = false;
		}
		else {
			errMessage = searchTerm + " is not a valid nationality.";
			printError(errMessage);
		}
	} while (entry == true);

	inputPoke.languageCode = 0x02;
	inputPoke.languageCode = inputPoke.languageCode<<8;
	inputPoke.languageCode += natId;

	cin.ignore(255, '\n');
}

// Edit OT name
void SavFile::editTrainerName(Poke &inputPoke) {

}

// Edit PC box markings
void SavFile::editMarkings(Poke &inputPoke) {

}

// Toggle if pokemon is shiny
void SavFile::toggleShiny(Poke &inputPoke) {
	uint16_t trnID = 0x00, secID = 0x00, perUp = 0x00, perLw = 0x00;
	unsigned int hexToDec = 0;
	trnID = inputPoke.trainerID>>16;
	secID = inputPoke.trainerID;
	perUp = inputPoke.personality>>16;
	perLw = inputPoke.personality;

	uint16_t shinyVal = 0x0000;
	shinyVal = (trnID^secID^perUp^perLw);
	hexToDec = shinyVal;

	// Toggle to non-shiny if shiny
	if (shinyVal < 8) {
		inputPoke.trainerID += 0x80000;
		cout << "\nPokemon is no longer shiny!";
	}

	// Toggle to shiny if non-shiny
	else {
		inputPoke.trainerID = inputPoke.trainerID^shinyVal;
		cout << "\nPokemon is now shiny!";
	}

	cout << '\n';

	inputPoke.decryptionKey = inputPoke.personality^inputPoke.trainerID;
	getDataSectionOrder(inputPoke);
}

// Edit the pokemon species
void SavFile::editSpecies(Poke &inputPoke) {
	bool entry = true;
	string searchTerm = "", errMessage = "";
	uint16_t pokeId = 0x0000;

	cout << "\n\nEnter the new Pokemon.";
	do {
		cout << "\n SPECIES > ";
		getline(cin, searchTerm);
		if (searchTerm == "exit") {
			entry = false;
			break;
		}
		else if (searchTerm == "help") {
			cout << "\nType in the name of the Pokemon you would like to change this one to.";
		}
		else {

			// Auto-capitalize search term
			if ((searchTerm[0] >= 97) && (searchTerm[0] <= 122))
				searchTerm[0] -= 32;

			// Search for entered value
			for (int i = 0; i < pokeList.size(); i++) {

				// Corresponding entry has been found
				if (searchTerm == pokeList[i].name) {
					cout << "\nPokemon is now " << searchTerm << "!\n";
					pokeId = pokeList[i].ID;
					i = pokeList.size()+1;
					entry = false;

					// Set actual data in structure
					for (int j = 0; j < 4; j++) {
						if (inputPoke.datSectOrder[j] == 0) {
							inputPoke.decodedData[(j*12)+1] = pokeId>>8;
							inputPoke.decodedData[(j*12)+0] = pokeId&0xff;
							break;
						}
					}
				
					// If new Pokemon is Mew or Deoxys, set the required obedience value
					if ((pokeId == 410) || (pokeId == 151)) {
						for (int j = 0; j < 4; j++) {
							if (inputPoke.datSectOrder[j] == 3) {
								if ((inputPoke.decodedData[(j*12)+11]&0x80) == 0)
									inputPoke.decodedData[(j*12)+11] += 0x80;
								break;
							}
						}
					}
	
					pokeId = i;
				}
			}
			// pokeList.size()+1 is marker for search being successful
			if (pokeId != pokeList.size()+1) {
				errMessage = "Could not find " + searchTerm;
				printError(errMessage);
			}
		}
	} while (entry == true);
}

// Edit the held item
void SavFile::editItem(Poke &inputPoke) {
	bool entry = true;
	string searchTerm = "", errMessage = "";
	uint16_t itemId = 0x0000;

	cout << "\n\nEnter the new item.";
	do {
		cout << "\n ITEM > ";
		getline(cin, searchTerm);
		if (searchTerm == "exit") {
			entry = false;
			break;
		}
		else if (searchTerm == "help") {
			cout << "\nEnter the name of the item you would like the Pokemon to hold.";
		}
		else {
			// Auto-capitalize item search term
			for (int i = 0; i < searchTerm.size(); i++) {
				if ((i == 0) || (searchTerm[i-1] == ' ')) {
					if ((searchTerm[i] >= 97) && (searchTerm[i] <= 122))
						searchTerm[i] -= 32;
				}
			}

			// Search for item in itemList
			for (int i = 0; i < itemList.size(); i++) {

				// Set data in structure if a match
				if (searchTerm == itemList[i].name) {
					cout << "\nItem set to " << searchTerm << '\n';
					itemId = itemList[i].ID;
					i = itemList.size()+1;
					entry = false;
					for (int j = 0; j < 4; j++) {
						if (inputPoke.datSectOrder[j] == 0) {
							inputPoke.decodedData[(j*12)+3] = itemId>>8;
							inputPoke.decodedData[(j*12)+2] = itemId&0xff;
							break;
						}
					}

					itemId = i;
				}
			}

			// itemList.size()+1 is marker for search being successful
			if (itemId != itemList.size()+1) {
				errMessage = "Could not find item " + searchTerm;
				errMessage += '.';
				printError(errMessage);
			}
		}
	} while (entry == true);
}

// Edit single move
void SavFile::editMove(Poke &inputPoke, int moveNum) {
	int startPos = 0;
	uint16_t moveId = 0x00;
	string moveName = "", errMessage = "";

	// Find location in data section of moves
	for (int i = 0; i < 4; i++) {
		if (inputPoke.datSectOrder[i] == 1) {
			startPos = i*12;
			break;
		}
	}

	// Find the move ID of the move being edited
	moveId = inputPoke.decodedData[startPos+(moveNum*2)+1];
	moveId = moveId<<8;
	moveId += inputPoke.decodedData[startPos+(moveNum*2)];

	// Search the move list for the current move
	for (int i = 0; i < moveList.size(); i++) {

		// Move has been found
		if (moveId == moveList[i].ID) {
			moveName = moveList[i].name;
			i = moveList.size()+1;
			moveId = i;
		}
	}

	// If moveId is not moveList.size()+1 search has failed
	// (Shouldn't happen, but it might)
	if (moveId != moveList.size()+1) {
		errMessage = "Cannot find move of Pokemon.";
		printError(errMessage);
	}
	else {
		bool entry = true;

		cout << "Current move is " << moveName
		     << "\nEnter the name of a new move.";

		do {
			cout << "\n MOVE > ";
			getline(cin, moveName);

			if (moveName == "exit") {
				entry = false;
				moveId = moveList.size()+1;
				continue;
			}
			
			// Search for entered name
			for (int i = 0; i < moveList.size(); i++) {

				// Entered move name has been found
				if (moveName == moveList[i].name) {
					moveId = moveList[i].ID;
					inputPoke.decodedData[startPos+(moveNum*2)+1] = moveId>>8;
					inputPoke.decodedData[startPos+(moveNum*2)] = moveId;
					cout << "\nMove has been set!\n";
					i = moveList.size()+1;
					moveId = i;
					entry = false;
				}
			}

			// Move could not be found
			if (moveId != moveList.size()+1) {
				errMessage = "The move " + moveName;
				errMessage += " could not be found.";
				printError(errMessage);
			}
		} while (entry == true);
	}
}

// Edit the EVs of a Pokemon
void SavFile::editEfforts(Poke &inputPoke) {
	unsigned int totalEffort = 0, startPos = 0, temp = 0;
	uint16_t hp = 0x00, atk = 0x00, def = 0x00, spd = 0x00, spA = 0x00, spD = 0x00;
	uint16_t evTotal = 0x00;
	bool entry = true, reallocation = true;
	string cmdInput = "", errMessage = "";
	vector <string> cmdParsed;
	stringstream strToInt;

	// Possible entries that will cause an exit
	string exitAlias [] = {"exit", "stop", "quit", "done"};
	const int EXIT_SIZE = 4;

	// Alternate names for stats
	string healthAlias [] = {"hp", "health"};
	string attackAlias [] = {"atk", "attack"};
	string defenseAlias [] = {"def", "defense"};
	string speedAlias [] = {"spd", "speed"};
	string spAtkAlias [] = {"spa", "specialattack"};
	string spDefAlias [] = {"spd", "specialdefense"};
	const int STAT_SIZE = 2;

	// Get the starting location of EVs
	for (int i = 0; i < 4; i++) {
		if (inputPoke.datSectOrder[i] == 2) {
			startPos = (i*12);
			break;
		}
	}

	// Main edit loop for Effort Values
	do {
		// Reallocation occurs either at the first time run or if changes happen
		if (reallocation == true) {
		        // Get the EVs themselves from the data section 
			evTotal = 0x00;
        		for (int i = 0; i < 6; i++) {
                		temp = getStatEV(inputPoke, i);
				evTotal += temp;
                		switch(i) {
                       			 case 0:
                                		hp = temp;
                                		break;
                        		case 1:
                                		atk = temp;
                                		break;
                       			 case 2:
                                		def = temp;
                                		break;
                        		case 3:
                                		spd = temp;
                                		break;
                        		case 4:
                                		spA = temp;
                                		break;
                        		case 5:
                                		spD = temp;
                                		break;
                		}
			}
			
			reallocation = false;
		}

		// Print main header for editing
		cout << "\n\n Effort Values (EVs) \n";
		for (int i = 0; i < 80; i++)
			cout << '=';
		cout << "\n\n"
		     << "\t     Health: " << hp << '\n'
		     << "\t     Attack: " << atk << '\n'
		     << "\t    Defense: " << def << '\n'
		     << "\t      Speed: " << spd << '\n'
		     << "\tSp.  Attack: " << spA << '\n'
		     << "\tSp. Defense: " << spD << "\n\n";
		for (int i = 0; i < 80; i++)
			cout << '=';
		cout << "\n\tTotal EVs: " << evTotal;
		if (evTotal > 510)
			cout << '*';
		else if (evTotal == 510)
			cout << " [MAX]";
		cout << '\n';
	
		cout << "\n EVs > ";

		// Get input and parse it
		getline(cin, cmdInput);
		parseInput(cmdParsed, cmdInput, ' ');
		clearScreen();

		// Set all characters in first issued command to lower-case
		for (int i = 0; i < cmdParsed[0].size(); i++) {
			if ((cmdParsed[0][i] >= 65) && (cmdParsed[0][i] <= 90))
				cmdParsed[0][i] += 32;
		}

		// If a second argument hasn't been given (exceed, exit, bad EV input)
		if (cmdParsed.size() < 2) {
			// Check if an 'exit' has been issued
			for (int i = 0; i < EXIT_SIZE; i++) {
				// If exit has been issued with vaild EVs, stop entry
				if (cmdParsed[0] == exitAlias[i]) {
					if (evTotal > 510) {
						errMessage = "Total EVs exceed standard 510!";
                                        	errMessage += " Use 'exceed' to override limit.";
                                        	printError(errMessage);
						reallocation = true; // Using reallocation
						// to test whether this error has been
						// printed because I don't want to make
						// another bool just for this error
                                        break;

					}
					else {
						entry = false;
						break;
					}
				}
			}

			// Check if a manual override of vanilla maximum is issued
                        if (cmdParsed[0] == "exceed")
                       		 entry = false;
			else if ((reallocation == false) && (entry == true)) {
				errMessage = "Missing second argument to set EV.";
				printError(errMessage);
			}

			// Check status of entry
			continue;
		}

		// Put second argument in string stream to convert to integer
		else
			strToInt << cmdParsed[1];
		
		// If conversion to integer has failed
		if (!(strToInt >> temp)) {
			strToInt.clear();
			strToInt >> errMessage;
			errMessage += " is not an integer.";
			printError(errMessage);
		}
		else {
			// EVs must be between 0 and 255
			if ((temp > 255) || (temp < 0)) {
				errMessage = "EVs must be between 0 and 255.";
				printError(errMessage);
			}
			else {
				// Branch based on input
				for (int i = 0; i < STAT_SIZE; i++) {
					if (cmdParsed[0] == healthAlias[i]) {
						reallocation = true;
						inputPoke.decodedData[startPos] = temp;
					}
					else if (cmdParsed[0] == attackAlias[i]) {
						reallocation = true;
						inputPoke.decodedData[startPos+1] = temp;
					}
					else if (cmdParsed[0] == defenseAlias[i]) {
						reallocation = true;
						inputPoke.decodedData[startPos+2] = temp;
					}
					else if (cmdParsed[0] == speedAlias[i]) {
						reallocation = true;
						inputPoke.decodedData[startPos+3] = temp;
					}
					else if (cmdParsed[0] == spAtkAlias[i]) {
						reallocation = true;
						inputPoke.decodedData[startPos+4] = temp;
					}
					else if (cmdParsed[0] == spDefAlias[i]) {
						reallocation = true;
						inputPoke.decodedData[startPos+5] = temp;
					}
				}
			}

			// If stat set failed, print error
			if (reallocation == false) {
				errMessage = cmdParsed[0] + " is not a valid command.";
				printError(errMessage);
			}
		}

		strToInt.str("");
		strToInt.clear();
	} while (entry == true);

	printOrdered(inputPoke);
}

// Edit PC data
bool SavFile::editPC() {
	bool changesMade = false;

	return changesMade;
}

// Write current game save to file
void SavFile::writeToFile () {
	string errMessage = "";

	// If a save has not been loaded
	if (isLoaded == false) {
		errMessage = "Unable to save as no file has been loaded!";
		printError(errMessage);
	}

	// Write to file
	else {
		ofstream writeFile;
		writeFile.open(saveToFile.c_str(), ios::binary);

		// If unable to open file for saving changes
		if (!(writeFile.is_open())) {
			errMessage = "Unable to save to file.";
			printError(errMessage);
		}

		// Write current save information to file
		else {
			cout << "\nSaving file... ";
			for (int i = 0; i < MAX_SIZE; i++)
				writeFile << hex << noskipws << savArray[i];
			writeFile.close();
			cout << "DONE!\n";
		}
	}
}

// Change the name of the save file to be written to
string SavFile::changeSaveTo() {
	bool nameEntry = true;
	char choice = ' ';
	string saveName = "", errMessage = "";

	// Get the new output save file
	cout << "\nEnter a new file name to save to.\n";
	do {
		cout << "\n SAVEAS > ";
		cin >> saveName;
		cout << "\nIs it okay to now save to " << saveName << "? (y/n)\n";
		do {
			cout << "\n SAVEAS > ";
			if (!(cin >> choice)) {
				errMessage = "Please enter only characters.";
				printError(errMessage);
				cin.clear();
				cin.ignore(255, '\n');
			}
			else {
				switch (choice) {
					case 'y':
						saveToFile = saveName;
						nameEntry = false;
						break;
					case 'n':
						break;
					default:
						errMessage = "Please enter only 'y' or 'n'.";
						printError(errMessage);
				}
				cin.ignore(255, '\n');
			}
		} while ((choice != 'y') && (choice != 'n'));
	} while (nameEntry == true);

	return saveToFile;
}

// Overwrite the original file with changes made to the current one
void SavFile::overwriteSave() {
	string errMessage = "";

	// If a save file has not been loaded
	if (isLoaded == false) {
		errMessage = "Unable to overwrite as no file has been loaded.";
		printError(errMessage);
	}

	// Overwrite original file
	else {
		ofstream writeTo;
		writeTo.open(loadFromFile.c_str(), ios::binary);

		// Could not open original file for writing
		if (!(writeTo.is_open())) {
			errMessage = "Could not write to original file.";
			printError(errMessage);
		}

		// Write to original save
		else {
			cout << "\nOverwriting save... ";
			for (int i = 0; i < MAX_SIZE; i++)
				writeTo << hex << noskipws << savArray[i];
			writeTo.close();
			cout << "DONE!\n";
		}
	}
}

// Calculate checksum of save file
void SavFile::verifyData() {
	uint64_t calculated = 0x00;
	uint32_t temp = 0x00, halfSums = 0x00;
	uint16_t retSum = 0x00, upperBits = 0x00, lowerBits = 0x00;
	uint8_t byteGrabber = 0x00;
	unsigned int sumSize = 0;

	cout << "\nCalculating Checksums... ";

	for (int i = 0; i < 14; i++) {
		// Reset values for next calculation
		calculated = 0x00;
		lowerBits = 0x00;
		upperBits = 0x00;
		halfSums = 0x00;

		// Number of bytes contributing to checksum for section
		switch (i) {
			case 0:
				sumSize = 3884;
				break;
			case 1:
			case 2:
			case 3:
			case 5:
			case 6:
			case 7:
			case 8:
			case 9:
			case 10:
			case 11:
			case 12:
				sumSize = 3968;
				break;
			case 4:
				sumSize = 3848;
				break;
			case 13:
				sumSize = 2000;
				break;
			default:
				sumSize = 0;
		}

		// Get checksum for each section
		for (int j = sectionLocations[i]; j < (sectionLocations[i]+sumSize); j += 4) {
			temp = 0x00;
			for (int offset = 3; offset >= 0; offset--) {
				byteGrabber = savArray[j+offset];
				temp += byteGrabber;
				if (offset != 0)
					temp = temp<<8;
			}
			calculated += temp;
		}

		// Break calculated value down into upper and lower bits
		lowerBits = (calculated&0xffff);
		calculated = calculated>>16;
		upperBits = (calculated&0xffff);

		// Store the actual checksum value
		halfSums = upperBits+lowerBits;
		retSum = halfSums&0xffff;

		// Replace checksum value within save
		savArray[sectionLocations[i]+4086] = retSum&0xff;
		savArray[sectionLocations[i]+4087] = (retSum>>8)&0xff;
	}

	cout << "DONE!\n";
}

// Convert encoded array to ASCII string
string SavFile::decodeArray (uint8_t encoded [], int size) {
	string decodedString = "";

        for (int i = 0; i < size; i++) {
		// Capital letters
                if ((encoded[i] >= 187) && (encoded[i] <= 212))
                        decodedString += encoded[i] - 122;
		// Lower-case letters
                else if ((encoded[i] >= 213) && (encoded[i] <= 238))
                        decodedString += encoded[i] - 116;
		// Numbers
                else if ((encoded[i] >= 161) && (encoded[i] <= 170))
                        decodedString += encoded[i] - 113;
		// Other characters
                else {
                        switch (encoded[i]) {
                                case (0x00): // Space
                                        decodedString += 32;
                                        break;
                                case (0xAB): // !
                                        decodedString += 33;
                                        break;
                                case (0xAC): // ?
                                        decodedString += 63;
                                        break;
                                case (0xAD): // .
                                        decodedString += 46;
                                        break;
                                case (0xAE): // -
                                        decodedString += 45;
                                        break;
				case (0xB0): // ...
					decodedString += 46;
					break;
                                case (0xB1): // open "
                                case (0xB2): // close "
                                        decodedString += 22;
                                        break;
                                case (0xB3): // open '
                                case (0xB4): // close '
                                        decodedString += 39;
                                        break;
                                case (0xB5): // MALE SYMBOL
                                case (0xB6): // FEMALE SYMBOL
                                        decodedString += '~';
                                        break;
                                case (0xB8): // ,
                                        decodedString += 44;
                                        break;
                                case (0xBA): // /
                                        decodedString += 47;
                                        break;
                                case (0xF1): // *A*
                                case (0xF2): // *O*
                                case (0xF3): // *U*
                                case (0xF4): // *a*
                                case (0xF5): // *o*
                                case (0xF6): // *u*
                                        decodedString += '&';
                                        break;

                                default: // Assumes 0xFF if unable to decode
                                        i = 100;
                                        break;
                        }
                }
        }

	return decodedString;
}

string SavFile::encodeArray(uint8_t decoded [], uint8_t encoded [], int size) {
	// String variable for return
	string retStr = "";

	// Handling for double quotes
	unsigned int sQuoteCount = 0, dQuoteCount = 0;

	// Blank out destination array and set return string
	for (int i = 0; i < size; i++) {
		if (decoded[i] == 0xFF)
			break;
		retStr += decoded[i];
	}

	for (int i = 0; i < size; i++)
		encoded[i] = 0xFF;

	// Encoded characters from ASCII to game encoding
	for (int i = 0; i < size; i++) {
		// Capital letters
		if ((decoded[i] >= 64) && (decoded[i] <= 90))
			encoded[i] = decoded[i] + 122;
		// Lower-case letters
		else if ((decoded[i] >= 97) && (decoded[i] <= 122))
			encoded[i] = decoded[i] + 116;
		// Numbers
		else if ((decoded[i] >= 48) && (decoded[i] <= 57))
			encoded[i] = decoded[i] + 113;
		// Other characters
		else {
			switch(decoded[i]) {
				case 32:	// space
					encoded[i] = 0x00;
					break;
				case 33:	// !
					encoded[i] = 0xAB;
					break;
				case 63:	// ?
					encoded[i] = 0xAC;
					break;
				case 46:	// .
					encoded[i] = 0xAD;
					break;
				case 45:	// -
					encoded[i] = 0xAE;
					break;
				case 34:	// "
					if (dQuoteCount % 2 == 0)
						encoded[i] = 0xB1;
					else
						encoded[i] = 0xB2;
					dQuoteCount++;
				case 39:	// '
					if (sQuoteCount % 2 == 0)
						encoded[i] = 0xB3;
					else
						encoded[i] = 0xB4;
					sQuoteCount++;
				case 44:	// ,
					encoded[i] = 0xB8;
					break;
				case 47:	// /
					encoded[i] = 0xBA;
					break;
				case 38:	// *A* -> *u*
					encoded[i] = 0xF1;
				default:
					encoded[i] = 0xFF;
					break;
			}
		}
	}
	
	return retStr;
}

// Return what the species of the pokemon is
string SavFile::getSpecies (Poke in) {
	uint16_t species = 0x00;
	unsigned int startPos = 0;
	string speciesName = "";

	// Determine which section has the relevant data
	for (int i = 0; i < 4; i++) {
		if (in.datSectOrder[i] == 0)
			startPos = i*12;
	}

	// Convert the species ID from little endian to big endian
	species += in.decodedData[startPos+1];
        species = species<<8;
        species += in.decodedData[startPos];

	// Search the list for a matching species ID
        for (int i = 0; i < pokeList.size(); i++) {
                if (species == pokeList[i].ID) {
			speciesName = pokeList[i].name;
			break;
		}
		else if (i == pokeList.size()-1)
			speciesName = "[Undefined]";
        }

	return speciesName;
}

// Return whether the pokemon is shiny or not
bool SavFile::getIfShiny(Poke in) {
        uint16_t trnID = 0x00, secID = 0x00, perUp = 0x00, perLw = 0x00;
        unsigned int hexToDec = 0;
        trnID = in.trainerID>>16;
        secID = in.trainerID;
        perUp = in.personality>>16;
        perLw = in.personality;

        uint16_t shinyVal = 0x0000;
        shinyVal = (trnID^secID^perUp^perLw);
        hexToDec = shinyVal;
	
	if (shinyVal < 8)
		return true;
	else
		return false;	
}

// Return the trainer gender as a character
char SavFile::getTrainerGender(Poke in) {
	uint16_t captureCond = 0x0000;
	char result = 0x00;
	int startPos = 0;

	// Determine which section has the relevant data
	for (int i = 0; i < 4; i++) {
		if (in.datSectOrder[i] == 3) {
			startPos = i*12;
			break;
		}
	}

	// Capture conditon data starts at offset of 2, and is size of 2
	captureCond += in.decodedData[startPos+2+1];
	captureCond = captureCond<<8;
	captureCond += in.decodedData[startPos+2];

	// Get just the OT gender bit
	captureCond = captureCond&0x8000;
	captureCond = captureCond>>15;

	// Determine gender char based on value
	if (captureCond == 0)
		result = 'M';
	else if (captureCond == 1)
		result = 'F';
	else
		result = '?';

	return result;
}

// Return the OT viewable ID
unsigned int SavFile::getOTID (Poke in) {
	uint16_t tID = 0x0000;

	tID = in.trainerID;

	return tID;
}

// Return the OT secret ID
unsigned int SavFile::getOTSec(Poke in) {
	uint16_t sID = 0x0000;

	sID = in.trainerID>>16;

	return sID;
}

// Return the happiness value
unsigned int SavFile::getHappiness(Poke in) {
	uint8_t happiness = 0x00;
	int startPos = 0;

	// Determine which section has the relevant data
	for (int i = 0; i < 4; i++) {
		if (in.datSectOrder[i] == 0) {
			startPos = i*12;
			break;
		}
	}

	happiness = in.decodedData[startPos+9];

	return happiness;
}

// Return the held item
string SavFile::getHeldItem(Poke in) {
        uint16_t item = 0x00;
        unsigned int startPos = 0;
        string itemName = "";

        // Determine which section has the relevant data
        for (int i = 0; i < 4; i++) {
                if (in.datSectOrder[i] == 0)
                        startPos = i*12;
        }

        // Convert the species ID from little endian to big endian
        item += in.decodedData[startPos+3];
        item = item<<8;
        item += in.decodedData[startPos+2];

        // Search the list for a matching item ID
        for (int i = 0; i < itemList.size(); i++) {
                if (item == itemList[i].ID) {
                        itemName = itemList[i].name;
			break;
		}
                else if (i == itemList.size()-1)
                        itemName = "[None]";
        }

	return itemName;	
}

// Print a move with a given offset
void SavFile::printMove(Poke in, int offset) {
	unsigned int startPos = 0, ppUpLoc = 0;
	int movePP = 0;

	// Determine which section has the move data
	for (int i = 0; i < 4; i++) {
		if (in.datSectOrder[i] == 1)
			startPos = i*12;
		else if (in.datSectOrder[i] == 0)
			ppUpLoc = i*12;
	}

	movePP = getMovePP(in, startPos, offset);

	// Print info about move to console
	cout << "\n Move " << offset << ": "
	     << getMoveName(in, startPos, offset) << "\tPP: ";
	if (movePP < 10)
		cout << ' ';	
	cout  << movePP << " (+ " << getPPUP(in, ppUpLoc, offset) << ')';
}

// Return move name
string SavFile::getMoveName(Poke in, unsigned int startPos, int offset) {
	int trueOffset = offset*2;
	uint16_t mID = 0x00;
	string moveName = "";

	// Get the move ID from little endian to big endian
	mID += in.decodedData[startPos+trueOffset+1];
	mID = mID<<8;
	mID += in.decodedData[startPos+trueOffset];

	// Search the list for matching move ID
	for (int i = 0; i < moveList.size(); i++) {
		if (mID == moveList[i].ID) {
			moveName = moveList[i].name;
			break;
		}
		else if(i == itemList.size()-1)
			moveName = "[Undefined]";
	}

	// Add trailing spaces to move for better formatting
	if (moveName.size() < 8) {
		for (int i = moveName.size(); i < 8; i++)
			moveName += ' ';
	}

	return moveName;
}

// Returns number of ppUps used on a move
unsigned int SavFile::getPPUP(Poke in, unsigned int ppUpLoc, int moveNum) {
	uint8_t ppUpDat = 0x00, twoBitAND = 0x3;

	// Get full byte of ppUp data
	ppUpDat = in.decodedData[ppUpLoc + 8];

	// Set the bits in twoBitAND to the appropriate positon
	twoBitAND = twoBitAND<<(2*moveNum);

	// AND ppUpDat with the 2 bits in twoBitAND
	ppUpDat = ppUpDat&twoBitAND;

	// Shuffle bits over in ppUpDat to print correct value
	ppUpDat = ppUpDat>>(2*moveNum);

	return ppUpDat;
}

// Return move PP
unsigned int SavFile::getMovePP(Poke in, unsigned int startPos, int offset) {
	int trueOffset = offset+8;
	uint8_t mPP;

	mPP = in.decodedData[startPos+trueOffset];

	return mPP;
}

// Print stat information
unsigned int SavFile::printStat(Poke in, int offset) {
	unsigned int ev = 0, iv = 0;

	if (in.isInParty) {
		switch(offset) {
			case 0:
				cout << in.curHP << " / " << in.totHP;
				break;
			case 1:
				cout << in.atk;
				break;
			case 2:
				cout << in.def;
				break;
			case 3:
				cout << in.spd;
				break;
			case 4:
				cout << in.spAtk;
				break;
			case 5:
				cout << in.spDef;
				break;
		}
	}

	ev = getStatEV(in, offset);
	iv = getStatIV(in, offset);
	
	// Print text formatted based on size of number
	cout << "\tEV: ";
	if ((ev < 100) && (ev > 9))
		cout << ' ';
	else if (ev < 10)
		cout << "  ";
	cout << ev << " / 255\tIV: ";
	if(iv < 10)
		cout << ' ';
	cout << iv << " / 31";

	return ev;
}

// Return stat EV
unsigned int SavFile::getStatEV(Poke in, int offset) {
	unsigned int startPos = 0, ev = 0;

	for (int i = 0; i < 4; i++) {
		if (in.datSectOrder[i] == 2) {
			startPos = i*12;
			break;
		}
	}

	ev = in.decodedData[startPos+offset];

	return ev;
}

// Return stat IV
unsigned int SavFile::getStatIV(Poke in, int offset) {
	unsigned int startPos = 0;
	uint32_t dwordIV = 0x00;
	uint8_t narrower = 0x00, ivVal = 0x00;

	for (int i = 0; i < 4; i++) {
		if (in.datSectOrder[i] == 3) {
			startPos = i*12;
			break;
		}
	}

	for (int i = 7; i >= 4; i--) {
		narrower = in.decodedData[startPos+i];
		dwordIV += narrower;
		if (i != 4)
			dwordIV = dwordIV<<8;
	}

	dwordIV = dwordIV>>(5*offset);

	ivVal = dwordIV&0x1F;

	return ivVal;
}




















