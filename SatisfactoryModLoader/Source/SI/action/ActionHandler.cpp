#include "ActionHandler.h"

#include "Utility.h"
#include "CharacterUtility.h"

#include "player/PlayerControllerHelper.h"

#include "FGAISystem.h"
#include "FGCharacterMovementComponent.h"
#include "FGCircuitSubsystem.h"
#include "FGHealthComponent.h"
#include "FGInventoryComponentEquipment.h"
#include "FGWheeledVehicle.h"

#include "WheeledVehicleMovementComponent.h"

bool AActionHandler::HandleAction(FString From, FString Type, TSharedPtr<FJsonObject> JsonObject)
{
	const FActionDelegate* Method = Actions.Find(Type);
	if (Method != nullptr)
	{
		const float DelayMin = JsonObject->GetNumberField("delay_min");
		const float DelayMax = JsonObject->GetNumberField("delay_max");
		float Delay = DelayMin;
		if (DelayMin < DelayMax)
		{
			Delay = FMath::RandRange(DelayMin, DelayMax);
		}
		
		FTimerDelegate Delegate;
		Delegate.BindLambda([this](TSharedPtr<FJsonObject> Json, const FActionDelegate* Method, FString From, FString Type)
			{
				if (!Method->ExecuteIfBound(Json))
				{
					SI_ERROR("Something went wrong executing action");
					return;
				}
				
				SI_INFO("Ran Action: ", *Type);
				const bool Silent = Json->GetBoolField("silent");
				if (Silent) return;
			
				StreamIntegration::Utility::SendMessageToAll(this, From + FString(" ran action ") + StreamIntegration::Utility::ToTitle(Type), FLinearColor::Green);
			}, JsonObject, Method, From, Type);

		if (Delay > 0)
		{
			FTimerHandle TimerHandle;
			GetWorld()->GetTimerManager().SetTimer(TimerHandle, Delegate, Delay, false, Delay);
		}
		else
		{
			return Delegate.ExecuteIfBound();
		}
		return true;
	}
	
	SI_WARN("Action not recognised ignoring");
	return false;
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
		auto Players = FPlayerControllerHelper::GetConnectedPlayers(GetWorld());
		return Players[FMath::RandRange(0, Players.Num() - 1)];
	}
	if (JsonTarget == TEXT("self"))
	{
		return FPlayerControllerHelper::GetPlayerByName(GetWorld(), StreamIntegration::GetConfig().Username);
	}
	return FPlayerControllerHelper::GetPlayerByName(GetWorld(), JsonTarget);
}

void AActionHandler::HandleInventoryBomb(const TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
		{
			AActor* Actor = Character;
			if (Character->IsDrivingVehicle())
			{
				const auto Vehicle = Cast<AFGWheeledVehicle>(Character->GetDrivenVehicle());
				if (!IsValid(Vehicle))
				{
					return;
				}
				Actor = Vehicle;
			}
			const auto Spread = JsonObject->GetIntegerField("spread");
			const auto DropEquipment = JsonObject->GetBoolField("drop_equipment");
			
			TInlineComponentArray<UFGInventoryComponent*, 24> InventoryComponents(Character);
			TArray<FInventoryStack> Stacks{};
			for (auto InventoryComponent : InventoryComponents)
			{
				if (!DropEquipment && InventoryComponent->IsA<UFGInventoryComponentEquipment>())
				{
					continue;
				}
				InventoryComponent->GetInventoryStacks(Stacks);
			}
	
			if (Stacks.Num() > 0)
			{
				const FInventoryStack CrateStack = Stacks[0];
				Stacks.RemoveAt(0);
				
				TArray<FInventoryStack> CrateInventory{};
				CrateInventory.Add(CrateStack);
				if (!StreamIntegration::Utility::Item::SpawnCrate(Actor, CrateInventory))
				{
					return;
				}
			}

			for (int i = 0; i < Stacks.Num(); ++i)
			{
				auto Stack = Stacks[i];
				if (!StreamIntegration::Utility::Item::DropItem(Actor, Stack, Spread))
				{
					TArray<FInventoryStack> CrateInventory{};
					for (int j = i; j < Stacks.Num(); ++j)
					{
						CrateInventory.Add(Stacks[j]);
					}
					if (!StreamIntegration::Utility::Item::SpawnCrate(Actor, CrateInventory))
					{
						return;
					}
					break;
				}
			}
			
			for (auto InventoryComponent : InventoryComponents)
			{
				if (!DropEquipment && InventoryComponent->IsA<UFGInventoryComponentEquipment>())
				{
					continue;
				}
				InventoryComponent->Empty();
			}
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
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
		{
			const auto ID = JsonObject->GetStringField("id");
			const int32 Amount = JsonObject->GetIntegerField("amount");
			const bool bDrop = JsonObject->GetBoolField("drop");
			const auto Spread = JsonObject->GetIntegerField("spread");

			try
			{
				const auto Stack = StreamIntegration::Utility::Item::CreateItemStack(Amount, ID);
				if (Stack.HasItems())
				{
					if (bDrop && !Character->IsDrivingVehicle())
						StreamIntegration::Utility::Item::DropItem(Character, Stack, Spread);
					else
						StreamIntegration::Utility::Item::GiveItem(Character, Stack, Spread);
				}
				else
				{
					SI_WARN("Invalid item: ", *ID);
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

void AActionHandler::HandleHealPlayer(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
		{
			if (Character->IsAliveAndWell() && !Player->NeedRespawn()) {
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
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
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
			if (Character->IsDrivingVehicle())
			{
				const auto Vehicle = Cast<AFGWheeledVehicle>(Character->GetDrivenVehicle());
				if (IsValid(Vehicle))
				{
					auto VehicleMovementComponent = Vehicle->GetVehicleMovementComponent();
					VehicleMovementComponent->Velocity += MoveVector;
					
					auto VehicleTransform = Vehicle->GetActorTransform();
					VehicleTransform.SetLocation(VehicleTransform.GetLocation() + MoveVector);
					Vehicle->SetActorTransform(VehicleTransform, false, nullptr, ETeleportType::TeleportPhysics);
					
					SI_DEBUG("Set new vehicle position");
				}
			}
			else
			{

				auto MovementComponent = Character->GetFGMovementComponent();
				MovementComponent->SetGeneralVelocity(MoveVector + MovementComponent->GetVelocity());
				Character->Jump();

				const UCurveFloat* OldCurve = nullptr;
				
				UObjectProperty* ObjProperty = FindField<UObjectProperty>(Character->GetClass(), "mFallDamageCurveOverride");
				if (ObjProperty != NULL)
				{
					OldCurve = ObjProperty->ContainerPtrToValuePtr<UCurveFloat>(Character);
					if (!IsValid(OldCurve) || (OldCurve->FloatCurve.Keys.Num() != 0 && !OldCurve->FloatCurve.Keys.IsValidIndex(0)))
					{
						SI_DEBUG("Old curve no valid, using null");
						OldCurve = nullptr;
					}
				}
				Character->SetFallDamageOverride(StreamIntegration::GetFallDamageOverride());

				MovePlayerDelegate.BindUFunction(this, FName(TEXT("ResetFallDamage")), Character, OldCurve);
				GetWorldTimerManager().SetTimer(MovePlayerTimerHandle, MovePlayerDelegate, NoFallDamage, false);
			}
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
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
		{
			const int32 Amount = JsonObject->GetIntegerField("amount");
			const FString ID = JsonObject->GetStringField("id");
			const float Radius = JsonObject->GetNumberField("radius");
			const bool Persistent = JsonObject->GetBoolField("persistent");
			const float ScaleMin = JsonObject->GetNumberField("scale_min");
			const float ScaleMax = JsonObject->GetNumberField("scale_max");
			const float DespawnTime = JsonObject->GetNumberField("despawn_time");
			const bool Kill = JsonObject->GetBoolField("kill_on_despawn");

			StreamIntegration::Utility::Actor::SpawnCreature(Character, ID, Amount, Radius, Persistent, ScaleMin, ScaleMax, DespawnTime, Kill);
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
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
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
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
		{
			if (Character->IsAliveAndWell() && !Player->NeedRespawn() && !Character->IsDrivingVehicle()) {
				const FString Style = JsonObject->GetStringField("style");
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

void AActionHandler::HandleTriggerFuse(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (IsValid(Player))
	{
		const float Chance = JsonObject->GetNumberField("chance");
		const auto GameState = Cast<AFGGameState>(UGameplayStatics::GetGameState(this));
		if (IsValid(GameState))
		{
			if (FMath::FRandRange(0, 100) <= Chance)
			{
				StreamIntegration::SetTrigger(true);
				SI_DEBUG("Trigering fuse!");
			}
			else
			{
				auto CircuitSubsystem = GameState->GetCircuitSubsystem();
				UFunction* Func = CircuitSubsystem->GetClass()->FindFunctionByName(FName("PowerCircuit_OnFuseSet"));
				CircuitSubsystem->ProcessEvent(Func, nullptr);
			}
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

void AActionHandler::ResetLowGravity(UFGCharacterMovementComponent* MovementComponent)
{
	if (IsValid(MovementComponent) && !MovementComponent->IsUnreachable())
	{
		MovementComponent->GravityScale = 1;
		const auto ModActor = StreamIntegration::GetModActor();
		if (ModActor->IsValidLowLevel())
		{
			ModActor->mOnGravityResetFinished.Broadcast();
		}
	}
}

void AActionHandler::HandleLowGravity(TSharedPtr<FJsonObject> JsonObject)
{
	const auto Player = GetTarget(JsonObject);
	if (IsValid(Player))
	{
		const auto Character = StreamIntegration::Utility::Character::GetPlayerCharacter(Player);
		if (IsValid(Character))
		{
			const float Amount = FMath::Max(FMath::Min(static_cast<float>(JsonObject->GetNumberField("amount")), 1.0f), -1.0f);
			const float Time = JsonObject->GetNumberField("reset_time");

			const auto MovementComponent = Character->GetFGMovementComponent();
			MovementComponent->GravityScale = Amount;

			Character->GetWorldTimerManager().ClearTimer(PlayerGravityTimerHandle);
			PlayerGravityDelegate.BindUFunction(this, FName(TEXT("ResetLowGravity")), MovementComponent);
			Character->GetWorldTimerManager().SetTimer(PlayerGravityTimerHandle, PlayerGravityDelegate, Time, false);
			
			const auto ModActor = StreamIntegration::GetModActor();
			if (ModActor->IsValidLowLevel())
			{
				ModActor->mOnGravityResetTimeNotification.Broadcast(Time);
			}
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}

static FLinearColor UpdateColor(const TArray<TSharedPtr<FJsonValue>> Data, FLinearColor Color)
{
	const auto Index = FMath::RandRange(0, Data.Num() - 1);
	const TSharedPtr<FJsonObject> ColorData = Data[Index]->AsObject();
	const float R = ColorData->GetNumberField("red");
	const float G = ColorData->GetNumberField("green");
	const float B = ColorData->GetNumberField("blue");

	if (R == -3 || G == -3 || B == -3)
	{
		return FLinearColor::MakeRandomColor();
	}
	
	return *new FLinearColor(
		R == -1 ? Color.R: R <= -2 ? FMath::FRand(): FMath::Max(R, 1.0f),
		G == -1 ? Color.G: G <= -2 ? FMath::FRand(): FMath::Max(G, 1.0f),
		B == -1 ? Color.B: B <= -2 ? FMath::FRand(): FMath::Max(B, 1.0f)
	);
}

void AActionHandler::HandleColorChange(TSharedPtr<FJsonObject> JsonObject) const
{
	const auto Player = GetTarget(JsonObject);
	if (IsValid(Player))
	{
		auto BuildableSubsystem = AFGBuildableSubsystem::Get(Player);
		if (IsValid(BuildableSubsystem))
		{
			const auto Slot = FMath::Max(FMath::Min(JsonObject->GetIntegerField("slot"), BuildableSubsystem->GetNbColorSlotsExposedToPlayers() - 1), 0);
			const auto PrimaryColors = JsonObject->GetArrayField("primary_colors");
			const auto SecondaryColors = JsonObject->GetArrayField("secondary_colors");

			const auto PrimaryColor = BuildableSubsystem->GetColorSlotPrimary_Linear(Slot);
			if (PrimaryColors.Num() > 0)
				BuildableSubsystem->SetColorSlotPrimary_Linear(Slot, UpdateColor(PrimaryColors, PrimaryColor));

			const auto SecondaryColor = BuildableSubsystem->GetColorSlotSecondary_Linear(Slot);
			if (SecondaryColors.Num() > 0)
				BuildableSubsystem->SetColorSlotSecondary_Linear(Slot, UpdateColor(SecondaryColors, SecondaryColor));
		}
	}
	else
	{
		SI_ERROR("Player was NULL, config is probably incorrect");
	}
}
