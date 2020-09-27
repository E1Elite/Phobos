#include <ScenarioClass.h>
#include <Unsorted.h>
#include <Theater.h>

#include "Utilities\trim.h"
#include "CustomTheater.h"
#include "Phobos.h"

DynamicVectorClass<CustomTheater*> CustomTheater::Array;
DynamicVectorClass<MixFileClass*> CustomTheater::SpecificMixs;

int CustomTheater::actualIndex = -1;
CustomTheater* CustomTheater::actualTheater = nullptr;
int &CustomTheater::LastTheater = *reinterpret_cast<int *>(0xB0B4FC);

void CustomTheater::LoadAllFromINI()
{
	CCINIClass* pINI = Phobos::OpenConfig("Theaters.ini");
	if (pINI != nullptr)
	{
		const char* mainSection = "THEATERS";
		int len = pINI->GetKeyCount(mainSection);
		for (int i = 0; i < len; ++i)
		{
			const char* key = pINI->GetKeyName(mainSection, i);
			pINI->ReadString(mainSection, key, "", Phobos::readBuffer);
			if (!Get(Phobos::readBuffer) && pINI->GetSection(Phobos::readBuffer))
			{
				CustomTheater* pTH = GameCreate<CustomTheater>(Phobos::readBuffer);
				Array.AddItem(pTH);
				pTH->LoadFromINI(pINI);
			}
		}
	}
	Phobos::CloseConfig(pINI);
}

void CustomTheater::LoadFromINI(CCINIClass* pINI)
{
	const char* pSection = ID;
	
	pINI->ReadString(pSection, "ControlFileName", "TEMPERAT.INI", Phobos::readBuffer);
	strcpy_s(ControlFileName, sizeof(ControlFileName), Phobos::readBuffer);
	
	pINI->ReadString(pSection, "Palette.Root", "TEMPERAT.PAL", Phobos::readBuffer);
	strcpy_s(PaletteRoot, sizeof(PaletteRoot), Phobos::readBuffer);
	
	pINI->ReadString(pSection, "Palette.Unit", "UNITTEM.PAL", Phobos::readBuffer);
	strcpy_s(PaletteUnit, sizeof(PaletteUnit), Phobos::readBuffer);
	
	pINI->ReadString(pSection, "Palette.ISO", "ISOTEM.PAL", Phobos::readBuffer);
	strcpy_s(PaletteISO, sizeof(PaletteISO), Phobos::readBuffer);
	
	pINI->ReadString(pSection, "Suffix", "TEM", Phobos::readBuffer);
	strcpy_s(Suffix, sizeof(Suffix), Phobos::readBuffer);
	
	pINI->ReadString(pSection, "Letter", "T", Phobos::readBuffer);
	strcpy_s(Letter, sizeof(Letter), Phobos::readBuffer);
	
	RadarBrightness = static_cast<float>(pINI->ReadDouble(pSection, "RadarBrightness", 1.0));

	IsArctic = pINI->ReadBool(pSection, "IsArctic", false);
	
	pINI->ReadString(pSection, "MixArray", "", Phobos::readBuffer);
	char* context = nullptr;
	for (auto pCur = strtok_s(Phobos::readBuffer, ",", &context); pCur; pCur = strtok_s(nullptr, ",", &context))
	{
		SpecificMixNames.AddItem(Trim::FullTrim(_strdup(pCur)));
	}
}

int CustomTheater::GetIndex(const char* id)
{
	for (int i = 0; i < Array.Count; ++i)
	{
		if (strcmp(Array.GetItem(i)->ID, id) == 0)
		{
			return i;
		}
	}
	return -1;
}

CustomTheater* CustomTheater::Get(const char* id)
{
	for (int i = 0; i < Array.Count; ++i)
	{
		CustomTheater* pTH = Array.GetItem(i);
		if (strcmp(pTH->ID, id) == 0)
		{
			return pTH;
		}
	}
	return nullptr;
}

CustomTheater* CustomTheater::Get(int id)
{
	return CustomTheater::Array.GetItem(id);
}

void CustomTheater::LoadSpecificMixs()
{
	UnloadSpecificMixs();
	for (int i = 0; i < SpecificMixNames.Count; ++i)
	{
		char* pName = SpecificMixNames.GetItem(i);
		SpecificMixs.AddItem(
			GameCreate<MixFileClass>(pName));
	}
}

void CustomTheater::UnloadSpecificMixs()
{
	for (int i = 0; i < SpecificMixs.Count; ++i)
	{
		MixFileClass *pMix = SpecificMixs.GetItem(i);
		GameDelete(pMix);
		pMix = nullptr;
	}
	SpecificMixs.Clear();
}

void CustomTheater::Applay()
{
	if (this == nullptr)
	{
		Debug::Log("CustomTheater::Applay = NULL\n");
	}
#define _memcpy(DST, SRC) memcpy(DST, SRC, sizeof(DST))
	DWORD protect;
	VirtualProtect(Theater::TH_Snow, sizeof(Theater), PAGE_EXECUTE_READWRITE, &protect);
	_memcpy(Theater::TH_Snow->Identifier, ID);
	_memcpy(Theater::TH_Snow->Extension,Suffix);
	_memcpy(Theater::TH_Snow->Letter, Letter);
	_memcpy(&Theater::TH_Snow->RadarTerrainBrightness, &RadarBrightness);
	VirtualProtect(Theater::TH_Snow, sizeof(Theater), protect, NULL);
#undef _memcpy
}


// =============================
// hooks

DEFINE_HOOK(5349CF, Theaters_Init, 0)
{
	//Debug::Log("Init custom theater %s\n", &Theaters[__TT__2]);
	Game::SetProgress(8);
	GET(int, index, ECX);

	auto pTheater = CustomTheater::Get(index);
	CustomTheater::actualTheater = pTheater;
	Debug::Log("Init custom theater %s\n", pTheater->ID);

	pTheater->actualIndex = index;
	
	pTheater->Applay();
	pTheater->LoadSpecificMixs();
	Game::SetProgress(6);
	Debug::Log("Init custom theater %s\n", pTheater->ID);

	R->EDI(0);
	R->EDX(0);
	ScenarioClass::Instance->Theater = TheaterType::Snow;
	pTheater->LastTheater = (int)TheaterType::Snow;

	R->EAX(FileSystem::LoadFile(pTheater->PaletteRoot, 0));
	Debug::Log("Init custom theater %s\n", pTheater->ID);

	return 0x534C09;
}

DEFINE_HOOK(534CA9, Theaters_Init2, 0)
{
	R->ESI(FileSystem::LoadFile(CustomTheater::actualTheater->PaletteUnit, 0));
	Game::SetProgress(12);
	return 0x534CCA;
}

DEFINE_HOOK(5454F0, Theaters_ControlFileName, 0)
{
	R->ECX(CustomTheater::actualTheater->ControlFileName);
	return 0x545513;
}

DEFINE_HOOK(54547F, Theaters_ISOPal, 0)
{
	R->ECX(CustomTheater::actualTheater->PaletteISO);
	return 0x5454A2;
}

DEFINE_HOOK(47589F, Theaters_GetIndex, 0)
{
	GET(char *, name, ECX);
	R->EAX(CustomTheater::GetIndex(name));
	return 0x4758A4;
}

DEFINE_HOOK(5FACDF, Theaters_LoadFromINI, 5)
{
	CustomTheater::LoadAllFromINI();
	return 0;
}

DEFINE_HOOK(689310, Theaters_Save_Prefix, 5)
{
	ScenarioClass::Instance->Theater = TheaterType(CustomTheater::actualIndex);
	return 0;
}

DEFINE_HOOK(68945B, Theaters_Save_Suffix, 5)
{
	ScenarioClass::Instance->Theater = TheaterType::Snow;
	return 0;
}

DEFINE_HOOK(4ACFD8, Theater_Skip_ReReadINI, 0)
{
	return 0x4ACFF6;
}

DEFINE_HOOK(48DBE0, Theater_FindIndex, 0)
{
	//The function Theater::FindIndex should always return TheaterType::Snow. For compatibility with Ares
	R->EAX(TheaterType::Snow);
	return 0x48DC0C;
}

DEFINE_HOOK_AGAIN(5BE6B1, Theaters_LastTheater__SkipTest, 5)
DEFINE_HOOK(59A28F, Theaters_LastTheater__SkipTest, 5)
{
	R->EAX(TheaterType::None);
	return 0;
}

DEFINE_HOOK(4AD7BC, Theaters_LastTheater__SkipTest2, 6)
{
	R->EDX(TheaterType::None);
	return 0;
}

//DEFINE_HOOK(47C302, Theaters_RadarBrightness, 0)
//{
//	LEA_STACK(void*, pRGBClass, 0x0C);
//	R->ECX(pRGBClass);
//
//	R->EAX(*reinterpret_cast<DWORD*>(&	//
//		CustomTheater::actualTheater->RadarBrightness
//		));
//	return 0x47C31E;
//}