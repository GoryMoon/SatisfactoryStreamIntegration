#pragma once

#include "SIModule.h"
#include "GameFramework/Info.h"

#include "CoreMinimal.h"
#include "util/Logging.h"
#include "Utility.h"
#include "Delegates/Delegate.h"

#include "ActionHandler.generated.h"

DECLARE_DELEGATE_OneParam(FActionDelegate, TSharedPtr<FJsonObject>);

UCLASS()
class SI_API AActionHandler: public AInfo
{
	GENERATED_BODY()
	
public:
	AActionHandler()
	{

		Actions.Add(TEXT("inventory_bomb"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleInventoryBomb));
		Actions.Add(TEXT("give_item"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleGiveItem));
		Actions.Add(TEXT("heal_player"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleHealPlayer));
		Actions.Add(TEXT("move_player"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleMovePlayer));
		Actions.Add(TEXT("spawn_mob"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleSpawnMob));
		Actions.Add(TEXT("drop_bomb"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleDropBomb));
		Actions.Add(TEXT("emote"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleEmote));
		Actions.Add(TEXT("trigger_fuse"), FActionDelegate::CreateUObject(this, &AActionHandler::HandleTriggerFuse));
	}

	bool HandleAction(FString From, FString Type, TSharedPtr<FJsonObject> JsonObject);

	UFUNCTION()
	void ResetFallDamage(AFGCharacterPlayer* Player, UCurveFloat* Curve);
	
protected:	
	TMap<FString, const FActionDelegate> Actions;

	class AFGPlayerController* GetTarget(TSharedPtr<FJsonObject> Json) const;

	void HandleInventoryBomb(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleGiveItem(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleHealPlayer(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleMovePlayer(TSharedPtr<FJsonObject> JsonObject);
	void HandleSpawnMob(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleDropBomb(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleEmote(TSharedPtr<FJsonObject> JsonObject) const;
	void HandleTriggerFuse(TSharedPtr<FJsonObject> JsonObject) const;
	
	FTimerHandle MovePlayerTimerHandle;
	FTimerDelegate MovePlayerDelegate;
};
