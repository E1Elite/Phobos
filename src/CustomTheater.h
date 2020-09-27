#pragma once
#include <MixFileClass.h>

class CustomTheater
{
private:
	static DynamicVectorClass<CustomTheater*> Array;
	static DynamicVectorClass<MixFileClass*> SpecificMixs;

public:
	static int actualIndex;
	static CustomTheater* actualTheater;
	static int& LastTheater;

	CustomTheater(char* ID) {
		strcpy_s(this->ID, sizeof(this->ID), ID);
	}

	static void LoadAllFromINI();

	static int GetIndex(const char* id);
	static CustomTheater* Get(const char* id);
	static CustomTheater* Get(int id);
	
	static void UnloadSpecificMixs();

	void LoadFromINI(CCINIClass*);
	void LoadSpecificMixs();
	void Applay();

	DynamicVectorClass<char*> SpecificMixNames;
	char	ID[0x10];
	char	Suffix[0x4];
	char	Letter[0x2];
	char	ControlFileName[20];
	float	RadarBrightness;
	char	PaletteRoot[20];
	char	PaletteUnit[20];
	char	PaletteISO[20];
	bool    IsArctic;
};

