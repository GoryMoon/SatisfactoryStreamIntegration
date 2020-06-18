#include "ActionHandler.h"



#include "FGAISystem.h"
#include "FGCharacterMovementComponent.h"
#include "FGHealthComponent.h"
#include "FGInventoryComponentBeltSlot.h"
#include "Utility.h"
#include "player/PlayerUtility.h"

void FActionHandler::HandleAction(FString Type, TSharedPtr<FJsonObject> JsonObject)
{
	const auto Method = Actions.Find(Type);
	if (Method != nullptr)
	{
		if (!Method->ExecuteIfBound(JsonObject))
			SI_ERROR("Something went wrong executing action");
		else
			SI_INFO("Ran Action: ", *Type);
	}
	else
		SI_WARN("Action not recognised ignoring");
}


AFGPlayerController* FActionHandler::GetTarget(const TSharedPtr<FJsonObject> Json) const
{
	FString JsonTarget;
	if (!Json->TryGetStringField("target", JsonTarget))
	{
		SI_DEBUG("Failed to get target. Defaulting to 'self'");
		JsonTarget = TEXT("self");
	}
	
	if (JsonTarget == TEXT("random"))
	{
		auto Players = SML::GetConnectedPlayers(Subsystem->GetWorld());
		return Players[FMath::RandRange(0, Players.Num() - 1)];
	}
	else if (JsonTarget == TEXT("self"))
	{
		return SML::GetPlayerByName(Subsystem->GetWorld(), StreamIntegration::GetConfig().Username);
	}
	else
	{
		return SML::GetPlayerByName(Subsystem->GetWorld(), JsonTarget);
	}
}

void FActionHandler::HandleInventoryBomb(const TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto* Character = static_cast<AFGCharacterPlayer*>(Player->GetCharacter());
		UFGInventoryComponent* Inventory = Character->GetInventory();
		const auto Spread = JsonObject->GetIntegerField("spread");
		
		TArray<FInventoryStack> Stacks{};
		Inventory->GetInventoryStacks(Stacks);
		for (auto Stack: Stacks)
		{
			StreamIntegration::DropItem(Character, Stack, Spread);
		}
		Inventory->Empty();
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void FActionHandler::HandleGiveItem(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto* Character = static_cast<AFGCharacterPlayer*>(Player->GetCharacter());

		const auto ID = JsonObject->GetStringField("id");
		const int32 Amount = JsonObject->GetIntegerField("amount");
		const bool bDrop = JsonObject->GetBoolField("drop");
		const auto Spread = JsonObject->GetIntegerField("spread");

		if (ID == "ALL")
		{
			TArray<FString> Items;
			StreamIntegration::ItemMap.GetKeys(Items);
			for (const auto ItemString : Items)
			{
				try
				{
					const auto Stack = StreamIntegration::CreateItemStack(Amount, ItemString);
					if (Stack.Item.IsValid())
					{
						if (bDrop)
							StreamIntegration::DropItem(Character, Stack, 0);
						else
							StreamIntegration::GiveItem(Character, Stack, Spread);
					}
				}
				catch (int e)
				{
					SI_ERROR("ERROR creating item: ", e);
				}
			}
		}
		else {
			try
			{
				const auto Stack = StreamIntegration::CreateItemStack(Amount, ID);
				if (Stack.Item.IsValid())
				{
					if (bDrop)
						StreamIntegration::DropItem(Character, Stack, 0);
					else
						StreamIntegration::GiveItem(Character, Stack, Spread);
				}
			}
			catch (int e)
			{
				SI_ERROR("ERROR creating item: ", e);
			}
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void FActionHandler::HandleHealPlayer(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto* Character = static_cast<AFGCharacterPlayer*>(Player->GetCharacter());

		const float Amount = JsonObject->GetNumberField("amount");
		
		const auto HealthComponent = Character->GetHealthComponent();
		HealthComponent->Heal(Amount);
		if (HealthComponent->GetCurrentHealth() <= 0)
			HealthComponent->Kill();
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void FActionHandler::HandleMovePlayer(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto* Character = static_cast<AFGCharacterPlayer*>(Player->GetCharacter());
		const float Amount = JsonObject->GetNumberField("amount");
		
		auto MoveVector = FMath::VRand() * Amount * 100;
		MoveVector.Z = Amount * 10;
		auto MovementComponent = Character->GetFGMovementComponent();
		MovementComponent->SetGeneralVelocity(MoveVector + MovementComponent->GetVelocity());
		Character->Jump();
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void FActionHandler::HandleSpawnMob(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto* Character = static_cast<AFGCharacterPlayer*>(Player->GetCharacter());
		const auto Amount = JsonObject->GetIntegerField("amount");
		const auto ID = JsonObject->GetStringField("id");
		const float Radius = JsonObject->GetNumberField("radius");

		StreamIntegration::SpawnCreature(Character, ID, Amount, Radius);
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}
