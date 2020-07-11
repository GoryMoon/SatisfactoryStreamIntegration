#pragma once
#include "EngineMinimal.h"
#include "FGCharacterPlayer.h"
#include "FGInventoryComponent.h"

namespace StreamIntegration
{
	namespace Utility
	{
		SI_API extern UClass* FindClass(const TCHAR* ClassName);
		SI_API extern bool GetFromMap(TMap<FString, FString> Map, FString Name, OUT UClass** OutClass);
		SI_API extern void SendMessageToAll(class UObject* WorldContext, const FString& Message, const FLinearColor& Color);
		SI_API extern FString ToTitle(const FString S);

		namespace Actor
		{
			SI_API extern void SpawnBomb(AFGCharacterPlayer* Player, int Amount, float Time, float Height, float Radius, const float Damage, const float DamageRadius);
		
			SI_API extern bool GetCreature(FString CreatureName, OUT UClass** Creature);
			SI_API extern void SpawnCreature(AFGCharacterPlayer* Player, FString CreatureID, int Amount, float Radius, bool Persistent, float ScaleMin, float ScaleMax);
			SI_API extern const TMap<FString, FString> CreatureMap;
		}

		namespace Item
		{
			SI_API extern bool GetItem(FString ItemName, OUT UClass** Item);
			SI_API extern FInventoryStack CreateItemStack(int Amount, FString ItemId);
			SI_API extern bool DropItem(AActor* Actor, const FInventoryStack& Stack, int Spread);
			SI_API extern void GiveItem(AFGCharacterPlayer* Player, const FInventoryStack& Stack, int Spread);
			SI_API extern bool SpawnCrate(AActor* Character, TArray<FInventoryStack> CrateInventory);
			SI_API extern const TMap<FString, FString> ItemMap;
		}
	}
}
