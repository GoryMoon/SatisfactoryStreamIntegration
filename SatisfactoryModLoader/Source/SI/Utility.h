#pragma once
#include "EngineMinimal.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"
#include "FGItemDescriptor.h"
#include "SIModule.h"

namespace StreamIntegration
{
	
	SI_API extern UClass* FindClass(const TCHAR* ClassName);
	SI_API extern bool GetFromMap(TMap<FString, FString> Map, FString Name, OUT UClass** OutClass);
	SI_API extern bool GetItem(FString ItemName, OUT UClass** Item);
	SI_API extern bool GetCreature(FString CreatureName, OUT UClass** Creature);
	
	SI_API extern FInventoryStack CreateItemStack(int Amount, FString ItemId);
	SI_API extern void DropItem(AFGCharacterPlayer* Player, const FInventoryStack& Stack, int Spread);
	SI_API extern void GiveItem(AFGCharacterPlayer* Player, const FInventoryStack& Stack, int Spread);

	SI_API extern void SpawnCreature(AFGCharacterPlayer* Player, FString CreatureID, int Amount, float Radius = -1);
	
	SI_API extern const TMap<FString, FString> ItemMap;
	SI_API extern const TMap<FString, FString> CreatureMap;
}
