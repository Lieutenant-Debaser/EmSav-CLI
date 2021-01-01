#ifndef SAVSTRUCTURE_H
#define SAVSTRUCTURE_H

#include <iostream>
#include <string>
#include <cstdint>
#include <fstream>
#include <vector>
#include <sstream>

using namespace std;

// Generic structure to hold all information
struct GenericInfo {
	unsigned int ID = 0;
	string name = "";
};

// Structure for a single pokemon's information
struct Poke {
        bool isInParty = false;
        uint32_t startAddr = 0x00;

        uint32_t personality = 0x00, trainerID = 0x00;
        uint8_t encodedNickname [10] = {0x00};
        string decodedNickname = "";
        uint16_t languageCode = 0x00;
	uint8_t encodedOT [7] = {0x00};
	string decodedOT = "";
        uint8_t markings = 0x00;
        uint16_t checksum = 0x00;
        uint16_t padding = 0x00;
        uint8_t encodedData [48] = {0x00};
        uint8_t decodedData [48] = {0x00};
        uint32_t status = 0x00;
        uint8_t level = 0x00;
        uint8_t pokerusTimer = 0x00;
        uint16_t curHP = 0x00, totHP = 0x00, atk = 0x00, def = 0x00,
                spd = 0x00, spAtk = 0x00, spDef = 0x00;

	int mod24Val = 0;
	int datSectOrder [4] = {0, 0, 0, 0};
	uint32_t decryptionKey = 0x00;
};

// Save file information and functions
class SavFile {
public:
	// load
	void getFiles();
	void loadGame();
	void loadRefFiles();

	// decode
	bool decodeSave();
	void encodePlayerName();
	void decodePlayerName();

	// edit
	void editData();

		// edit player
	bool editTrainer();
	void printPlayer();
	void setPlayerName();
	void toggleGender();
	void setTrainerID();
	void setSecretID();
	void setPlayTime();
	// EDIT INVENTORY

		// edit team
	bool editTeam();
	void printTeam(int teamStart, string teamNicknames[]);
	void editPokemon(uint32_t startAddr, bool isInTeam);
	void decodePokeData(Poke &inputPoke);
	void getDataSectionOrder(Poke &inputPoke);
	void encodePokeData(Poke &inputPoke);
	void writePokemon(Poke output);
	void printPokemonData(Poke inputPoke);
	void printState(Poke inputPoke, int offsetMult);
	void printMoveInfo(Poke inputPoke, int offsetMult);
	void printEVContest(Poke inputPoke, int offsetMult);
	void printOrigin(Poke inputPoke, int offsetMult);

		// edit pc boxes
	bool editPC();

		// actual editing functions
	void editNickname(Poke &inputPoke);
	void editNationality(Poke &inputPoke);
	void editTrainerName(Poke &inputPoke);
	void editMarkings(Poke &inputPoke);
	void toggleShiny(Poke &inputPoke);
	void editSpecies(Poke &inputPoke);
	void editItem(Poke &inputPoke);
	void editMove(Poke &inputPoke, int moveNum);
	void editEfforts(Poke &inputPoke);

	// save, saveas, overwrite
	void writeToFile();
	string changeSaveTo();
	void overwriteSave();
	
	// Data Integrity
	void verifyData();

	// Utility
	string decodeArray(uint8_t encoded [], int size);
	string encodeArray(uint8_t decoded [], uint8_t encoded [], int size);
	void printPoke(Poke inputPoke);

	// UI Handling
	void printOrdered (Poke in);

	// Return information about pokemon
	string getSpecies (Poke in);
	string getHeldItem (Poke in);
	string getMoveName(Poke in, unsigned int startPos, int offset);
	bool getIfShiny(Poke in);
	char getTrainerGender(Poke in);
	void printMove(Poke in, int offset);
	unsigned int printStat(Poke in, int offset);
	unsigned int getMovePP(Poke in, unsigned int startPos, int offset);
	unsigned int getPPUP(Poke in, unsigned int ppUpLoc, int moveNum);
	unsigned int getOTID(Poke in);
	unsigned int getOTSec(Poke in);
	unsigned int getHappiness(Poke in);
	unsigned int getStatEV(Poke in, int offset);
	unsigned int getStatIV(Poke in, int  offset);

private:
	// Save information
	uint8_t savArray [131072] = {0x00};
	const int MAX_SIZE = 131072;
	string saveToFile = "", loadFromFile = "";
	bool isLoaded = false, isDecoded = false, isModified = false;

	// Player information
	string playerName = "";
	uint8_t encodedName [8] = {0x00};
	uint8_t playerGender = 0x00;
	uint32_t teamSize = 0x00;

	// Save structure information
	uint32_t sectionLocations [14] = {0x00};
	uint32_t gameCode = 0x00, securityKey = 0x00;

	// Reference info
	vector <GenericInfo> pokeList;
	vector <GenericInfo> locList;
	vector <GenericInfo> itemList;
	vector <GenericInfo> moveList;
};

#endif
