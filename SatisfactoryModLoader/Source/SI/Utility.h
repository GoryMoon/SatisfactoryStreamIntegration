#pragma once
#include "EngineMinimal.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"
#include "FGItemDescriptor.h"
#include "SIModule.h"

namespace StreamIntegration
{
	extern void SetBoolProperty(UObject* Obj, FName Prop, bool bValue);
	extern void SetFloatProperty(UObject* Obj, FName Prop, float Value);
	
	SI_API extern UClass* FindClass(const TCHAR* ClassName);
	SI_API extern bool GetFromMap(TMap<FString, FString> Map, FString Name, OUT UClass** OutClass);
	SI_API extern bool GetItem(FString ItemName, OUT UClass** Item);
	SI_API extern bool GetCreature(FString CreatureName, OUT UClass** Creature);
	
	SI_API extern FInventoryStack CreateItemStack(int Amount, FString ItemId);
	SI_API extern void DropItem(AFGCharacterPlayer* Player, const FInventoryStack& Stack, int Spread);
	SI_API extern void GiveItem(AFGCharacterPlayer* Player, const FInventoryStack& Stack, int Spread);

	SI_API extern void SpawnCreature(AFGCharacterPlayer* Player, FString CreatureID, int Amount, float Radius = 10, bool Persistent = true);
	SI_API extern void SpawnBomb(AFGCharacterPlayer* Player, int Amount, float Time, float Height, float Radius, const float Damage, const float DamageRadius);
	
	SI_API extern const TMap<FString, FString> ItemMap;
	SI_API extern const TMap<FString, FString> CreatureMap;
}
