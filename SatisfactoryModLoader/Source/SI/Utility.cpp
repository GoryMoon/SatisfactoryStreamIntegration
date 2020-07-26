#include "Utility.h"

#include "SIModule.h"

#include "util/Logging.h"
#include "player/PlayerControllerHelper.h"
#include "player/component/SMLPlayerComponent.h"

#include "FGC4Explosive.h"
#include "FGCreatureSpawner.h"
#include "FGInventoryLibrary.h"
#include "FGItemPickup_Spawnable.h"
#include "FGCrate.h"
#include "FGHealthComponent.h"
#include "FGBlueprintFunctionLibrary.h"
#include "FGItemDescriptor.h"

#include "AkGameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetStringLibrary.h"


static FORCEINLINE UClass* LoadClassFromPath(const FString& Path)
{
	if (Path == "") return NULL;

	return StaticLoadClass(UObject::StaticClass(), NULL, *Path, NULL, LOAD_None, NULL);
}

UClass* StreamIntegration::Utility::FindClass(const TCHAR* ClassName)
{
	UClass* LoadedActorOwnerClass = FindObject<UClass>(ANY_PACKAGE, ClassName);
	if (LoadedActorOwnerClass == NULL)
	{
		LoadedActorOwnerClass = LoadObject<UClass>(NULL, ClassName);
	}

    if (LoadedActorOwnerClass == NULL)
    {
		SI_ERROR("Class not found, loading from full class path... ", ClassName);
        LoadedActorOwnerClass = LoadClassFromPath(ClassName);

        if (LoadedActorOwnerClass == NULL)
        {
			SI_ERROR("Class not found, should check if it still exists ", ClassName);
        }
    }
    return LoadedActorOwnerClass;
}

bool StreamIntegration::Utility::GetFromMap(TMap<FString, FString> Map, const FString Name, UClass** OutClass)
{
	const auto ClassName = Map.Find(Name);
	if (ClassName != nullptr)
	{
		const auto Class = FindClass(ToCStr(*ClassName));
		if (Class != NULL)
		{
			*OutClass = Class;
			return true;
		}
	}
	*OutClass = nullptr;
	return false;
}

void StreamIntegration::Utility::SendMessageToAll(class UObject* WorldContext, const FString& Message, const FLinearColor& Color)
{
	TArray<AFGPlayerController*> ConnectedPlayers = FPlayerControllerHelper::GetConnectedPlayers(WorldContext->GetWorld());
	for (AFGPlayerController* Controller : ConnectedPlayers) {
		USMLPlayerComponent::Get(Controller)->SendChatMessage(Message, Color);
	}
}

FString StreamIntegration::Utility::ToTitle(const FString S)
{
	bool bLast = true;
	FString Out = S.Replace(TEXT("_"), TEXT(" "));
	for (int i = 0; i < Out.Len(); i++)
	{
		const TCHAR c = Out[i];
		Out[i] = bLast ? toupper(c) : tolower(c);
		bLast = isspace(c) == 1;
	}
	return Out;
}


bool StreamIntegration::Utility::Item::GetItem(const FString ItemName, UClass **Item)
{
	TArray<TSubclassOf<UFGItemDescriptor>> ItemDescriptors;
	UFGBlueprintFunctionLibrary::Cheat_GetAllDescriptors(ItemDescriptors);
	for (const auto ItemDescriptor : ItemDescriptors)
	{
		if (FCString::Stricmp(*UFGItemDescriptor::GetItemName(ItemDescriptor).ToString(), *ItemName) == 0)
		{
			*Item = *ItemDescriptor;
			return true;
		}
	}
	return false;
}

FInventoryStack StreamIntegration::Utility::Item::CreateItemStack(const int Amount, FString ItemId)
{
	SI_DEBUG("Creating itemstack: ", *ItemId);
	FInventoryItem Item;
	UClass* Class = nullptr;
	if (GetItem(ItemId, &Class))
	{
		Item = UFGInventoryLibrary::MakeInventoryItem(Class);
	}
	else
	{
		SI_ERROR("Could not find item, returning an empty item");
		Item = UFGInventoryLibrary::GetNullInventoryItem();
	}
	return UFGInventoryLibrary::MakeInventoryStack(Amount, Item);
}

bool StreamIntegration::Utility::Item::DropItem(AActor* Actor, const FInventoryStack& Stack, const int Spread)
{
	FRotator OutRotation;
	FVector OutPosition;
	AFGItemPickup_Spawnable::FindGroundLocationInfrontOfActor(Actor, 230 + FMath::RandRange(0, Spread * 100), Stack, OutPosition, OutRotation);
	const auto ItemDrop = AFGItemPickup_Spawnable::CreateItemDrop(Actor->GetWorld(), Stack, OutPosition, OutRotation);
	SI_DEBUG("Dropped item: ", (IsValid(ItemDrop) ? "True": "False"));
	return IsValid(ItemDrop);
}

void StreamIntegration::Utility::Item::GiveItem(AFGCharacterPlayer* Player, const FInventoryStack& Stack, const int Spread)
{
	const auto Amount = Player->GetInventory()->AddStack(Stack, true);
	if (Amount < Stack.NumItems)
		DropItem(Player, UFGInventoryLibrary::MakeInventoryStack(Stack.NumItems - Amount, Stack.Item), Spread);
	else
		SI_DEBUG("Gave item");
}

bool StreamIntegration::Utility::Item::SpawnCrate(AActor* Character, TArray<FInventoryStack> CrateInventory)
{
	TArray<AActor*> IgnoredActors{};
	IgnoredActors.Add(Character);

	FVector EndPosition = Character->GetActorLocation();
	FVector* SpawnPosition = new FVector(EndPosition.X, EndPosition.Y, EndPosition.Z + 30);
	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(Character);
	QueryParams.MobilityType = EQueryMobilityType::Static;

	if (Character->GetWorld()->LineTraceSingleByChannel(HitResult, *SpawnPosition, EndPosition, ECC_WorldStatic, QueryParams))
	{
		SpawnPosition->Z = HitResult.ImpactPoint.Z + 8;
	}
	else
	{
		SpawnPosition->Z = EndPosition.Z;
	}

	AFGCrate* Crate;
	AFGItemPickup_Spawnable::SpawnInventoryCrate(Character->GetWorld(), CrateInventory, *SpawnPosition, IgnoredActors, Crate);
	if (IsValid(Crate))
	{
		Crate->OnRequestReprecentMarker();
		return true;
	}
	return false;
}


bool StreamIntegration::Utility::Actor::GetCreature(FString CreatureName, UClass** Creature)
{
	return GetFromMap(CreatureMap, CreatureName, Creature);
}

auto StreamIntegration::Utility::Actor::SpawnCreature(AFGCharacterPlayer* Player, const FString CreatureID, const int Amount, const float Radius, const bool Persistent, const float ScaleMin, float ScaleMax, float DespawnTime, bool Kill) -> void
{
	const auto Transform = Player->GetTransform();
	auto World = Player->GetWorld();

	UClass* CreatureClass;
	if (GetCreature(CreatureID, &CreatureClass))
	{
		try
		{
			for (int i = 0; i < Amount; ++i)
			{
				const auto Location = Transform.GetLocation();
				auto Vector = FMath::VRand();
				(Vector.X *= Radius * 100) += Location.X;
				(Vector.Y *= Radius * 100) += Location.Y;
				Vector.Z = Location.Z;

				SI_DEBUG("Trying to spawn creature at ", *Vector.ToString());
				
				auto Scale = FVector::OneVector;
				Scale *= FMath::RandRange(ScaleMin, ScaleMax);
				FTransform* SpawnTransform = new FTransform(Player->GetViewRotation(), Vector, Scale);

				const auto Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(Player, CreatureClass, *SpawnTransform, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
				UGameplayStatics::FinishSpawningActor(Actor, *SpawnTransform);
				if (!CreatureID.Equals(TEXT("Manta")) && Persistent)
				{
					UKismetSystemLibrary::SetBoolPropertyByName(Actor, "mNeedsSpawner", false);
					Cast<AFGCreature>(Actor)->SetPersistent(true);

					const auto StructProperty = FindField<UStructProperty>(Actor->GetClass(), "mKillOrphanHandle");
					if (StructProperty != NULL)
					{
						const auto Handle = StructProperty->ContainerPtrToValuePtr<FTimerHandle>(Actor);
						Actor->GetWorldTimerManager().ClearTimer(*Handle);
					}
					if (CreatureID.Equals(TEXT("BabyCrab")))
					{
						FTimerDynamicDelegate Timer;
						Timer.BindUFunction(Actor, "KillOrphanCrabs");
						FTimerHandle TimerHandle = Actor->GetWorldTimerManager().K2_FindDynamicTimerHandle(Timer);
						Actor->GetWorldTimerManager().ClearTimer(TimerHandle);
					}
					
					SI_DEBUG("Set creature to persistent");
				}
				if (DespawnTime > 0)
				{
					FTimerHandle Handle;
					FTimerDelegate DespawnDelegate;
					DespawnDelegate.BindLambda([](AActor* Creature, bool Kill)
						{
							if (IsValid(Creature))
							{
								if (Kill)
									Cast<AFGCreature>(Creature)->GetHealthComponent()->Kill();
								else
									Creature->Destroy();
							}
						}, Actor, Kill && !CreatureID.Equals(TEXT("Manta")));
					
					Actor->GetWorldTimerManager().SetTimer(Handle, DespawnDelegate, DespawnTime, false);
				}
			}
		} catch (int e)
		{
			SI_ERROR("SpawnCreature ERROR: ", e);
		}
	}
	else
	{
		SI_ERROR("Could not find creature: ", *CreatureID);
	}
}

void StreamIntegration::Utility::Actor::SpawnBomb(AFGCharacterPlayer* Player, const int Amount, const float Time, const float Height, const float Radius, const float Damage, const float DamageRadius)
{
	const auto Transform = Player->GetTransform();
	auto World = Player->GetWorld();

	if (UClass* CreatureClass = FindClass(TEXT("/Game/FactoryGame/Equipment/C4Dispenser/BP_C4Explosive.BP_C4Explosive_C")))
	{
		try
		{
			for (int i = 0; i < Amount; ++i)
			{
				const auto Location = Transform.GetLocation();
				auto Vector = FMath::VRand();
				(Vector.X *= Radius * 100) += Location.X;
				(Vector.Y *= Radius * 100) += Location.Y;
				Vector.Z = Location.Z + Height * 100;

				SI_DEBUG("Trying to spawn bomb at ", *Vector.ToString());
				FActorSpawnParameters Parmas;
				Parmas.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

				const auto Actor = Cast<AFGC4Explosive>(World->SpawnActor(CreatureClass, &Vector, 0, Parmas));

				UKismetSystemLibrary::SetFloatPropertyByName(Actor, "mBaseDamage", Damage);
				UKismetSystemLibrary::SetFloatPropertyByName(Actor, "mDamageRadius", DamageRadius * 100);
				
				FTimerDelegate DetonateTimer;
				DetonateTimer.BindLambda([](AFGC4Explosive* Actor)
				{
					Actor->PlayExplosionEffects();
					Actor->Detonate();
				}, Actor);

				FTimerHandle DetonateHandle;
				Actor->GetWorldTimerManager().SetTimer(DetonateHandle, DetonateTimer, Time, false);
			}
		}
		catch (int e)
		{
			SI_ERROR("SpawnCreature ERROR: ", e);
		}
	}
	else
	{
		SI_ERROR("Could not find bomb class");
	}
}

const TMap<FString, FString> StreamIntegration::Utility::Actor::CreatureMap = {
	{ "Manta", "/Game/FactoryGame/Character/Creature/Wildlife/Manta/BP_Manta.BP_Manta_C" },
	{ "SpaceGiraffe", "/Game/FactoryGame/Character/Creature/Wildlife/SpaceGiraffe/Char_SpaceGiraffe.Char_SpaceGiraffe_C" },
	{ "SpaceRabbit", "/Game/FactoryGame/Character/Creature/Wildlife/SpaceRabbit/Char_SpaceRabbit.Char_SpaceRabbit_C" },
	{ "NonFlyingBird", "/Game/FactoryGame/Character/Creature/Wildlife/NonFlyingBird/Char_NonFlyingBird.Char_NonFlyingBird_C" },

	{ "BabyCrab", "/Game/FactoryGame/Character/Creature/Enemy/Crab/BabyCrab/Char_BabyCrab.Char_BabyCrab_C" },
	{ "CrabHatcher", "/Game/FactoryGame/Character/Creature/Enemy/CrabHatcher/Char_CrabHatcher.Char_CrabHatcher_C" },
	{ "Hog", "/Game/FactoryGame/Character/Creature/Enemy/Hog/Char_Hog.Char_Hog_C" },
	{ "AlphaHog", "/Game/FactoryGame/Character/Creature/Enemy/Hog/AlphaHog/Char_AlphaHog.Char_AlphaHog_C" },
	{ "Spitter", "/Game/FactoryGame/Character/Creature/Enemy/Spitter/Char_Spitter.Char_Spitter_C" },
	{ "Spitter_Small", "/Game/FactoryGame/Character/Creature/Enemy/Spitter/SmallSpitter/Char_Spitter_Small.Char_Spitter_Small_C" },
	{ "Spitter_Alternative", "/Game/FactoryGame/Character/Creature/Enemy/Spitter/AlternativeSpitter/Char_Spitter_Alternative.Char_Spitter_Alternative_C" },
	{ "CaveStinger", "/Game/FactoryGame/Character/Creature/Enemy/Stinger/Char_CaveStinger.Char_CaveStinger_C" },
	{ "Stinger", "/Game/FactoryGame/Character/Creature/Enemy/Stinger/Char_Stinger.Char_Stinger_C" },
	{ "EliteCaveStinger", "/Game/FactoryGame/Character/Creature/Enemy/Stinger/BigStinger/Char_EliteCaveStinger.Char_EliteCaveStinger_C" },
	{ "EliteStinger", "/Game/FactoryGame/Character/Creature/Enemy/Stinger/BigStinger/Char_EliteStinger.Char_EliteStinger_C" },
	{ "CaveStinger_Child", "/Game/FactoryGame/Character/Creature/Enemy/Stinger/SmallStinger/Char_CaveStinger_Child.Char_CaveStinger_Child_C" },
	{ "Stinger_Child", "/Game/FactoryGame/Character/Creature/Enemy/Stinger/SmallStinger/Char_Stinger_Child.Char_Stinger_Child_C" },

};