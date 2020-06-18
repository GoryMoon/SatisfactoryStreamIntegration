#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"
#include "integration/IntegrationSubsystem.h"

DECLARE_DELEGATE_OneParam(FActionDelegate, TSharedPtr<FJsonObject>);

class SI_API FActionHandler
{
protected:
	TMap<FString, const FActionDelegate> Actions;
	TSharedPtr<class AIntegrationSubsystem> Subsystem;

	class AFGPlayerController* GetTarget(TSharedPtr<FJsonObject> Json) const;

	void HandleInventoryBomb(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleGiveItem(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleHealPlayer(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleMovePlayer(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleSpawnMob(TSharedPtr<FJsonObject> JsonObject) const;
	
public:
	FActionHandler()
	{
		Actions.Add(TEXT("inventory_bomb"), FActionDelegate::CreateRaw(this, &FActionHandler::HandleInventoryBomb));
		Actions.Add(TEXT("give_item"), FActionDelegate::CreateRaw(this, &FActionHandler::HandleGiveItem));
		Actions.Add(TEXT("heal_player"), FActionDelegate::CreateRaw(this, &FActionHandler::HandleHealPlayer));
		Actions.Add(TEXT("move_player"), FActionDelegate::CreateRaw(this, &FActionHandler::HandleMovePlayer));
		Actions.Add(TEXT("spawn_mob"), FActionDelegate::CreateRaw(this, &FActionHandler::HandleSpawnMob));
	}

	void SetSubsystem(const TSharedPtr<AIntegrationSubsystem> System)
	{
		Subsystem = System;
	}

	void HandleAction(FString Type, TSharedPtr<FJsonObject> JsonObject);
};
