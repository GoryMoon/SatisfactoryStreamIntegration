#include "ActionHandler.h"

#include "FGAISystem.h"
#include "FGCharacterMovementComponent.h"
#include "FGHealthComponent.h"
#include "FGInventoryComponentBeltSlot.h"
#include "Utility.h"
#include "CharacterUtility.h"
#include "player/PlayerUtility.h"

void AActionHandler::HandleAction(FString Type, TSharedPtr<FJsonObject> JsonObject)
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


AFGPlayerController* AActionHandler::GetTarget(const TSharedPtr<FJsonObject> Json) const
{
	FString JsonTarget;
	if (!Json->TryGetStringField("target", JsonTarget))
	{
		SI_DEBUG("Failed to get target. Defaulting to 'self'");
		JsonTarget = TEXT("self");
	}
	
	if (JsonTarget == TEXT("random"))
	{
		auto Players = SML::GetConnectedPlayers(GetWorld());
		return Players[FMath::RandRange(0, Players.Num() - 1)];
	}
	else if (JsonTarget == TEXT("self"))
	{
		return SML::GetPlayerByName(GetWorld(), StreamIntegration::GetConfig().Username);
	}
	else
	{
		return SML::GetPlayerByName(GetWorld(), JsonTarget);
	}
}

void AActionHandler::HandleInventoryBomb(const TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto Character = Cast<AFGCharacterPlayer>(Player->GetCharacter());
		if (Character)
		{
			UFGInventoryComponent* Inventory = Character->GetInventory();
			const auto Spread = JsonObject->GetIntegerField("spread");

			TArray<FInventoryStack> Stacks{};
			Inventory->GetInventoryStacks(Stacks);
			for (auto Stack : Stacks)
			{
				StreamIntegration::Utility::Item::DropItem(Character, Stack, Spread);
			}
			Inventory->Empty();
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void AActionHandler::HandleGiveItem(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto Character = Cast<AFGCharacterPlayer>(Player->GetCharacter());
		if (Character)
		{
			const auto ID = JsonObject->GetStringField("id");
			const int32 Amount = JsonObject->GetIntegerField("amount");
			const bool bDrop = JsonObject->GetBoolField("drop");
			const auto Spread = JsonObject->GetIntegerField("spread");

			if (ID == "ALL")
			{
				TArray<FString> Items;
				StreamIntegration::Utility::Item::ItemMap.GetKeys(Items);
				for (const auto ItemString : Items)
				{
					try
					{
						const auto Stack = StreamIntegration::Utility::Item::CreateItemStack(Amount, ItemString);
						if (Stack.Item.IsValid())
						{
							if (bDrop)
								StreamIntegration::Utility::Item::DropItem(Character, Stack, Spread);
							else
								StreamIntegration::Utility::Item::GiveItem(Character, Stack, Spread);
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
					const auto Stack = StreamIntegration::Utility::Item::CreateItemStack(Amount, ID);
					if (Stack.Item.IsValid())
					{
						if (bDrop)
							StreamIntegration::Utility::Item::DropItem(Character, Stack, Spread);
						else
							StreamIntegration::Utility::Item::GiveItem(Character, Stack, Spread);
					}
				}
				catch (int e)
				{
					SI_ERROR("ERROR creating item: ", e);
				}
			}
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void AActionHandler::HandleHealPlayer(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto Character = Cast<AFGCharacterPlayer>(Player->GetCharacter());
		if (Character)
		{
			const auto Controller = Cast<AFGPlayerController>(Character->GetController());
			if (Controller && Character->IsAliveAndWell() && !Controller->NeedRespawn()) {
				const float Amount = JsonObject->GetNumberField("amount");

				const auto HealthComponent = Character->GetHealthComponent();
				HealthComponent->Heal(Amount);
				if (HealthComponent->GetCurrentHealth() <= 0)
					HealthComponent->Kill();
			}
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void AActionHandler::HandleMovePlayer(TSharedPtr<FJsonObject> JsonObject)
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto Character = Cast<AFGCharacterPlayer>(Player->GetCharacter());
		if (Character)
		{
			const float Amount = JsonObject->GetNumberField("amount");
			float AmountVertical = JsonObject->GetNumberField("amount_vertical");
			const float NoFallDamage = JsonObject->GetNumberField("no_fall_damage");

			if (AmountVertical == -1)
			{
				AmountVertical = Amount / 10;
			}

			auto MoveVector = FMath::VRand() * (Amount * 100);
			MoveVector.Z = AmountVertical * 100;
			auto MovementComponent = Character->GetFGMovementComponent();
			MovementComponent->SetGeneralVelocity(MoveVector + MovementComponent->GetVelocity());
			Character->Jump();

			const UCurveFloat* OldCurve = nullptr;
			if (UProperty* Property = Character->GetClass()->FindPropertyByName(TEXT("mFallDamageCurveOverride")))
			{
				if (UObjectProperty* ObjProperty = Cast<UObjectProperty>(Property))
				{
					OldCurve = ObjProperty->ContainerPtrToValuePtr<UCurveFloat>(Character);
					if (!IsValid(OldCurve) || (OldCurve->FloatCurve.Keys.Num() != 0 && !OldCurve->FloatCurve.Keys.IsValidIndex(0)))
					{
						SI_DEBUG("Old curve no valid, using null");
						OldCurve = nullptr;
					}
				}
			}
			Character->SetFallDamageOverride(StreamIntegration::GetFallDamageOverride());

			MovePlayerDelegate.BindUFunction(this, FName("ResetFallDamage"), Character, OldCurve);
			GetWorldTimerManager().SetTimer(MovePlayerTimerHandle, MovePlayerDelegate, NoFallDamage, false);
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void AActionHandler::ResetFallDamage(AFGCharacterPlayer* Player, UCurveFloat* Curve)
{
	SI_DEBUG("Setting override to: ", Curve);
	Player->SetFallDamageOverride(Curve);
	SI_DEBUG("Resetting falldamage override");
}

void AActionHandler::HandleSpawnMob(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto* Character = Cast<AFGCharacterPlayer>(Player->GetCharacter());
		if (Character)
		{
			const int32 Amount = JsonObject->GetIntegerField("amount");
			const FString ID = JsonObject->GetStringField("id");
			const float Radius = JsonObject->GetNumberField("radius");
			const bool Persistent = JsonObject->GetBoolField("persistent");

			StreamIntegration::Utility::Actor::SpawnCreature(Character, ID, Amount, Radius, Persistent);
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void AActionHandler::HandleDropBomb(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		auto Character = Cast<AFGCharacterPlayer>(Player->GetCharacter());
		if (Character)
		{
			const int32 Amount = JsonObject->GetIntegerField("amount");
			const float Time = JsonObject->GetNumberField("time");
			const float Height = JsonObject->GetNumberField("height");
			const float Radius = JsonObject->GetNumberField("radius");
			const float Damage = JsonObject->GetNumberField("damage");
			const float DamageRadius = JsonObject->GetNumberField("damage_radius");

			StreamIntegration::Utility::Actor::SpawnBomb(Character, Amount, Time, Height, Radius, Damage, DamageRadius);
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void AActionHandler::HandleEmote(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (Player != nullptr)
	{
		const auto Character = Cast<AFGCharacterPlayer>(Player->GetCharacter());
		const FString Style = JsonObject->GetStringField("style");

		if (Character)
		{
			const auto Controller = Cast<AFGPlayerController>(Character->GetController());
			if (Controller && Character->IsAliveAndWell() && !Controller->NeedRespawn()) {

				if (Style == TEXT("Clap"))
				{
					StreamIntegration::Utility::Character::PlayClapEmote(Character);
				}
				else if (Style == TEXT("Naruto"))
				{
					StreamIntegration::Utility::Character::PlayNarutoEmote(Character);
				}
				else if (Style == TEXT("Spin"))
				{
					StreamIntegration::Utility::Character::PlaySpinEmote(Character);
				}
			}
		}
		
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
	
}


