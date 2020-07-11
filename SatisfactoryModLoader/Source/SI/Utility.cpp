#include "Utility.h"

#include "SIModule.h"

#include "util/Logging.h"
#include "player/PlayerUtility.h"
#include "player/component/SMLPlayerComponent.h"

#include "FGC4Explosive.h"
#include "FGCreatureSpawner.h"
#include "FGInventoryLibrary.h"
#include "FGItemPickup_Spawnable.h"
#include "FGCrate.h"

#include "AkGameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"


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
	TArray<AFGPlayerController*> ConnectedPlayers = SML::GetConnectedPlayers(WorldContext->GetWorld());
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
	return GetFromMap(ItemMap, ItemName, Item);
}

FInventoryStack StreamIntegration::Utility::Item::CreateItemStack(const int Amount, FString ItemId)
{
	SI_DEBUG("Creating itemstack: ", *ItemId);
	FInventoryItem Item;
	UClass* Class;
	if (GetItem(ItemId, &Class))
	{
		Item = UFGInventoryLibrary::MakeInventoryItem(Class);
	} else
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

auto StreamIntegration::Utility::Actor::SpawnCreature(AFGCharacterPlayer* Player, const FString CreatureID, const int Amount, const float Radius, const bool Persistent, const float ScaleMin, float ScaleMax) -> void
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
			}
		} catch (int e)
		{
			SI_ERROR("SpawnCreature ERROR: ", e);
		}
	} else
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


const TMap<FString, FString> StreamIntegration::Utility::Item::ItemMap = {
	{ "WAT1", "/Game/FactoryGame/Prototype/WAT/Desc_WAT1.Desc_WAT1_C" },
	{ "WAT2", "/Game/FactoryGame/Prototype/WAT/Desc_WAT2.Desc_WAT2_C" },
	{ "HardDrive", "/Game/FactoryGame/Resource/Environment/CrashSites/Desc_HardDrive.Desc_HardDrive_C" },
	{ "ServerRack", "/Game/FactoryGame/Resource/Environment/CrashSites/Desc_ServerRack.Desc_ServerRack_C" },
	{ "Crystal", "/Game/FactoryGame/Resource/Environment/Crystal/Desc_Crystal.Desc_Crystal_C" },
	{ "Crystal_mk2", "/Game/FactoryGame/Resource/Environment/Crystal/Desc_Crystal_mk2.Desc_Crystal_mk2_C" },
	{ "Crystal_mk3", "/Game/FactoryGame/Resource/Environment/Crystal/Desc_Crystal_mk3.Desc_Crystal_mk3_C" },
	{ "CrystalShard", "/Game/FactoryGame/Resource/Environment/Crystal/Desc_CrystalShard.Desc_CrystalShard_C" },
	{ "Nobelisk", "/Game/FactoryGame/Resource/Equipment/C4Dispenser/Desc_Nobelisk.Desc_Nobelisk_C" },
	{ "AluminaSolution", "/Game/FactoryGame/Resource/Parts/Alumina/Desc_AluminaSolution.Desc_AluminaSolution_C" },
	{ "AluminumIngot", "/Game/FactoryGame/Resource/Parts/AluminumIngot/Desc_AluminumIngot.Desc_AluminumIngot_C" },
	{ "AluminumPlate", "/Game/FactoryGame/Resource/Parts/AluminumPlate/Desc_AluminumPlate.Desc_AluminumPlate_C" },
	{ "AluminumPlateReinforced", "/Game/FactoryGame/Resource/Parts/AluminumPlateReinforced/Desc_AluminumPlateReinforced.Desc_AluminumPlateReinforced_C" },
	{ "AluminumScrap", "/Game/FactoryGame/Resource/Parts/AluminumScrap/Desc_AluminumScrap.Desc_AluminumScrap_C" },
	{ "Battery", "/Game/FactoryGame/Resource/Parts/Battery/Desc_Battery.Desc_Battery_C" },
	{ "Cable", "/Game/FactoryGame/Resource/Parts/Cable/Desc_Cable.Desc_Cable_C" },
	{ "CartridgeStandard", "/Game/FactoryGame/Resource/Parts/CartridgeStandard/Desc_CartridgeStandard.Desc_CartridgeStandard_C" },
	{ "Cement", "/Game/FactoryGame/Resource/Parts/Cement/Desc_Cement.Desc_Cement_C" },
	{ "CircuitBoard", "/Game/FactoryGame/Resource/Parts/CircuitBoard/Desc_CircuitBoard.Desc_CircuitBoard_C" },
	{ "CircuitBoardHighSpeed", "/Game/FactoryGame/Resource/Parts/CircuitBoardHighSpeed/Desc_CircuitBoardHighSpeed.Desc_CircuitBoardHighSpeed_C" },
	{ "ColorCartridge", "/Game/FactoryGame/Resource/Parts/ColorCartridge/Desc_ColorCartridge.Desc_ColorCartridge_C" },
	{ "CompactedCoal", "/Game/FactoryGame/Resource/Parts/CompactedCoal/Desc_CompactedCoal.Desc_CompactedCoal_C" },
	{ "Computer", "/Game/FactoryGame/Resource/Parts/Computer/Desc_Computer.Desc_Computer_C" },
	{ "ComputerQuantum", "/Game/FactoryGame/Resource/Parts/ComputerQuantum/Desc_ComputerQuantum.Desc_ComputerQuantum_C" },
	{ "ComputerSuper", "/Game/FactoryGame/Resource/Parts/ComputerSuper/Desc_ComputerSuper.Desc_ComputerSuper_C" },
	{ "CopperIngot", "/Game/FactoryGame/Resource/Parts/CopperIngot/Desc_CopperIngot.Desc_CopperIngot_C" },
	{ "CopperSheet", "/Game/FactoryGame/Resource/Parts/CopperSheet/Desc_CopperSheet.Desc_CopperSheet_C" },
	{ "CrystalOscillator", "/Game/FactoryGame/Resource/Parts/CrystalOscillator/Desc_CrystalOscillator.Desc_CrystalOscillator_C" },
	{ "DarkMatter", "/Game/FactoryGame/Resource/Parts/DarkMatter/Desc_DarkMatter.Desc_DarkMatter_C" },
	{ "ElectromagneticControlRod", "/Game/FactoryGame/Resource/Parts/ElectromagneticControlRod/Desc_ElectromagneticControlRod.Desc_ElectromagneticControlRod_C" },
	{ "Filter", "/Game/FactoryGame/Resource/Parts/Filter/Desc_Filter.Desc_Filter_C" },
	{ "FluidCanister", "/Game/FactoryGame/Resource/Parts/FluidCanister/Desc_FluidCanister.Desc_FluidCanister_C" },
	{ "Fuel", "/Game/FactoryGame/Resource/Parts/Fuel/Desc_Fuel.Desc_Fuel_C" },
	{ "LiquidFuel", "/Game/FactoryGame/Resource/Parts/Fuel/Desc_LiquidFuel.Desc_LiquidFuel_C" },
	{ "GoldIngot", "/Game/FactoryGame/Resource/Parts/GoldIngot/Desc_GoldIngot.Desc_GoldIngot_C" },
	{ "Gunpowder", "/Game/FactoryGame/Resource/Parts/GunPowder/Desc_Gunpowder.Desc_Gunpowder_C" },
	{ "HeavyOilResidue", "/Game/FactoryGame/Resource/Parts/HeavyOilResidue/Desc_HeavyOilResidue.Desc_HeavyOilResidue_C" },
	{ "PackagedOilResidue", "/Game/FactoryGame/Resource/Parts/HeavyOilResidue/Desc_PackagedOilResidue.Desc_PackagedOilResidue_C" },
	{ "HighSpeedConnector", "/Game/FactoryGame/Resource/Parts/HighSpeedConnector/Desc_HighSpeedConnector.Desc_HighSpeedConnector_C" },
	{ "HighSpeedWire", "/Game/FactoryGame/Resource/Parts/HighSpeedWire/Desc_HighSpeedWire.Desc_HighSpeedWire_C" },
	{ "HUBParts", "/Game/FactoryGame/Resource/Parts/HUBParts/Desc_HUBParts.Desc_HUBParts_C" },
	{ "HazmatFilter", "/Game/FactoryGame/Resource/Parts/IodineInfusedFilter/Desc_HazmatFilter.Desc_HazmatFilter_C" },
	{ "IronIngot", "/Game/FactoryGame/Resource/Parts/IronIngot/Desc_IronIngot.Desc_IronIngot_C" },
	{ "IronPlate", "/Game/FactoryGame/Resource/Parts/IronPlate/Desc_IronPlate.Desc_IronPlate_C" },
	{ "IronPlateReinforced", "/Game/FactoryGame/Resource/Parts/IronPlateReinforced/Desc_IronPlateReinforced.Desc_IronPlateReinforced_C" },
	{ "IronRod", "/Game/FactoryGame/Resource/Parts/IronRod/Desc_IronRod.Desc_IronRod_C" },
	{ "IronScrew", "/Game/FactoryGame/Resource/Parts/IronScrew/Desc_IronScrew.Desc_IronScrew_C" },
	{ "ModularFrame", "/Game/FactoryGame/Resource/Parts/ModularFrame/Desc_ModularFrame.Desc_ModularFrame_C" },
	{ "ModularFrameHeavy", "/Game/FactoryGame/Resource/Parts/ModularFrameHeavy/Desc_ModularFrameHeavy.Desc_ModularFrameHeavy_C" },
	{ "ModularFrameLightweight", "/Game/FactoryGame/Resource/Parts/ModularFrameLightweight/Desc_ModularFrameLightweight.Desc_ModularFrameLightweight_C" },
	{ "Motor", "/Game/FactoryGame/Resource/Parts/Motor/Desc_Motor.Desc_Motor_C" },
	{ "MotorLightweight", "/Game/FactoryGame/Resource/Parts/MotorLightweight/Desc_MotorLightweight.Desc_MotorLightweight_C" },
	{ "NobeliskExplosive", "/Game/FactoryGame/Resource/Parts/NobeliskExplosive/Desc_NobeliskExplosive.Desc_NobeliskExplosive_C" },
	{ "NuclearWaste", "/Game/FactoryGame/Resource/Parts/NuclearWaste/Desc_NuclearWaste.Desc_NuclearWaste_C" },
	{ "PetroleumCoke", "/Game/FactoryGame/Resource/Parts/PetroleumCoke/Desc_PetroleumCoke.Desc_PetroleumCoke_C" },
	{ "Plastic", "/Game/FactoryGame/Resource/Parts/Plastic/Desc_Plastic.Desc_Plastic_C" },
	{ "PolymerResin", "/Game/FactoryGame/Resource/Parts/PolymerResin/Desc_PolymerResin.Desc_PolymerResin_C" },
	{ "QuantumCrystal", "/Game/FactoryGame/Resource/Parts/QuantumCrystal/Desc_QuantumCrystal.Desc_QuantumCrystal_C" },
	{ "QuantumOscillator", "/Game/FactoryGame/Resource/Parts/QuantumOscillator/Desc_QuantumOscillator.Desc_QuantumOscillator_C" },
	{ "QuartzCrystal", "/Game/FactoryGame/Resource/Parts/QuartzCrystal/Desc_QuartzCrystal.Desc_QuartzCrystal_C" },
	{ "ResourceSinkCoupon", "/Game/FactoryGame/Resource/Parts/ResourceSinkCoupon/Desc_ResourceSinkCoupon.Desc_ResourceSinkCoupon_C" },
	{ "Rotor", "/Game/FactoryGame/Resource/Parts/Rotor/Desc_Rotor.Desc_Rotor_C" },
	{ "Rubber", "/Game/FactoryGame/Resource/Parts/Rubber/Desc_Rubber.Desc_Rubber_C" },
	{ "SAMFluctuator", "/Game/FactoryGame/Resource/Parts/SAMFluctuator/Desc_SAMFluctuator.Desc_SAMFluctuator_C" },
	{ "SAMIngot", "/Game/FactoryGame/Resource/Parts/SAMIngot/Desc_SAMIngot.Desc_SAMIngot_C" },
	{ "Silica", "/Game/FactoryGame/Resource/Parts/Silica/Desc_Silica.Desc_Silica_C" },
	{ "SpaceElevatorBlocker", "/Game/FactoryGame/Resource/Parts/SpaceElevatorBlocker/Desc_SpaceElevatorBlocker.Desc_SpaceElevatorBlocker_C" },
	{ "SpaceElevatorPart_1", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_1.Desc_SpaceElevatorPart_1_C" },
	{ "SpaceElevatorPart_2", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_2.Desc_SpaceElevatorPart_2_C" },
	{ "SpaceElevatorPart_3", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_3.Desc_SpaceElevatorPart_3_C" },
	{ "SpaceElevatorPart_4", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_4.Desc_SpaceElevatorPart_4_C" },
	{ "SpaceElevatorPart_5", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_5.Desc_SpaceElevatorPart_5_C" },
	{ "SpaceElevatorPart_6", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_6.Desc_SpaceElevatorPart_6_C" },
	{ "SpaceElevatorPart_7", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_7.Desc_SpaceElevatorPart_7_C" },
	{ "SpaceElevatorPart_8", "/Game/FactoryGame/Resource/Parts/SpaceElevatorParts/Desc_SpaceElevatorPart_8.Desc_SpaceElevatorPart_8_C" },
	{ "SpikedRebar", "/Game/FactoryGame/Resource/Parts/SpikedRebar/Desc_SpikedRebar.Desc_SpikedRebar_C" },
	{ "SpikedRebarScatter", "/Game/FactoryGame/Resource/Parts/SpikedRebar/Desc_SpikedRebarScatter.Desc_SpikedRebarScatter_C" },
	{ "Stator", "/Game/FactoryGame/Resource/Parts/Stator/Desc_Stator.Desc_Stator_C" },
	{ "SteelIngot", "/Game/FactoryGame/Resource/Parts/SteelIngot/Desc_SteelIngot.Desc_SteelIngot_C" },
	{ "SteelPipe", "/Game/FactoryGame/Resource/Parts/SteelPipe/Desc_SteelPipe.Desc_SteelPipe_C" },
	{ "SteelPlate", "/Game/FactoryGame/Resource/Parts/SteelPlate/Desc_SteelPlate.Desc_SteelPlate_C" },
	{ "SteelPlateReinforced", "/Game/FactoryGame/Resource/Parts/SteelPlateReinforced/Desc_SteelPlateReinforced.Desc_SteelPlateReinforced_C" },
	{ "SulfuricAcid", "/Game/FactoryGame/Resource/Parts/SulfuricAcid/Desc_SulfuricAcid.Desc_SulfuricAcid_C" },
	{ "LiquidTurboFuel", "/Game/FactoryGame/Resource/Parts/Turbofuel/Desc_LiquidTurboFuel.Desc_LiquidTurboFuel_C" },
	{ "TurboFuel", "/Game/FactoryGame/Resource/Parts/Turbofuel/Desc_TurboFuel.Desc_TurboFuel_C" },
	{ "UraniumCell", "/Game/FactoryGame/Resource/Parts/UraniumCell/Desc_UraniumCell.Desc_UraniumCell_C" },
	{ "UraniumPellet", "/Game/FactoryGame/Resource/Parts/UraniumPellet/Desc_UraniumPellet.Desc_UraniumPellet_C" },
	{ "Wire", "/Game/FactoryGame/Resource/Parts/Wire/Desc_Wire.Desc_Wire_C" },
	{ "PackagedOil", "/Game/FactoryGame/Resource/RawResources/CrudeOil/Desc_PackagedOil.Desc_PackagedOil_C" },
	{ "PackagedWater", "/Game/FactoryGame/Resource/RawResources/Water/Desc_PackagedWater.Desc_PackagedWater_C" },
	{ "HogParts", "/Game/FactoryGame/Resource/Parts/AnimalParts/Desc_HogParts.Desc_HogParts_C" },
	{ "SpitterParts", "/Game/FactoryGame/Resource/Parts/AnimalParts/Desc_SpitterParts.Desc_SpitterParts_C" },
	{ "Biofuel", "/Game/FactoryGame/Resource/Parts/BioFuel/Desc_Biofuel.Desc_Biofuel_C" },
	{ "LiquidBiofuel", "/Game/FactoryGame/Resource/Parts/BioFuel/Desc_LiquidBiofuel.Desc_LiquidBiofuel_C" },
	{ "PackagedBiofuel", "/Game/FactoryGame/Resource/Parts/BioFuel/Desc_PackagedBiofuel.Desc_PackagedBiofuel_C" },
	{ "Fabric", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_Fabric.Desc_Fabric_C" },
	{ "FlowerPetals", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_FlowerPetals.Desc_FlowerPetals_C" },
	{ "GenericBiomass", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_GenericBiomass.Desc_GenericBiomass_C" },
	{ "Leaves", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_Leaves.Desc_Leaves_C" },
	{ "Mycelia", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_Mycelia.Desc_Mycelia_C" },
	{ "Pigment", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_Pigment.Desc_Pigment_C" },
	{ "Vines", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_Vines.Desc_Vines_C" },
	{ "Wood", "/Game/FactoryGame/Resource/Parts/GenericBiomass/Desc_Wood.Desc_Wood_C" },
	{ "NuclearFuelRod", "/Game/FactoryGame/Resource/Parts/NuclearFuelRod/Desc_NuclearFuelRod.Desc_NuclearFuelRod_C" },
	{ "Coal", "/Game/FactoryGame/Resource/RawResources/Coal/Desc_Coal.Desc_Coal_C" },
	{ "LiquidOil", "/Game/FactoryGame/Resource/RawResources/CrudeOil/Desc_LiquidOil.Desc_LiquidOil_C" },
	{ "Geyser", "/Game/FactoryGame/Resource/RawResources/Geyser/Desc_Geyser.Desc_Geyser_C" },
	{ "OreBauxite", "/Game/FactoryGame/Resource/RawResources/OreBauxite/Desc_OreBauxite.Desc_OreBauxite_C" },
	{ "OreCopper", "/Game/FactoryGame/Resource/RawResources/OreCopper/Desc_OreCopper.Desc_OreCopper_C" },
	{ "OreGold", "/Game/FactoryGame/Resource/RawResources/OreGold/Desc_OreGold.Desc_OreGold_C" },
	{ "OreIron", "/Game/FactoryGame/Resource/RawResources/OreIron/Desc_OreIron.Desc_OreIron_C" },
	{ "OreUranium", "/Game/FactoryGame/Resource/RawResources/OreUranium/Desc_OreUranium.Desc_OreUranium_C" },
	{ "RawQuartz", "/Game/FactoryGame/Resource/RawResources/RawQuartz/Desc_RawQuartz.Desc_RawQuartz_C" },
	{ "SAM", "/Game/FactoryGame/Resource/RawResources/SAM/Desc_SAM.Desc_SAM_C" },
	{ "Stone", "/Game/FactoryGame/Resource/RawResources/Stone/Desc_Stone.Desc_Stone_C" },
	{ "Sulfur", "/Game/FactoryGame/Resource/RawResources/Sulfur/Desc_Sulfur.Desc_Sulfur_C" },
	{ "Water", "/Game/FactoryGame/Resource/RawResources/Water/Desc_Water.Desc_Water_C" },
	{ "Chainsaw", "/Game/FactoryGame/Equipment/Chainsaw/Desc_Chainsaw.Desc_Chainsaw_C" },
	{ "HookShot", "/Game/FactoryGame/Equipment/Hookshot/BP_EquipmentDescriptorHookShot.BP_EquipmentDescriptorHookShot_C" },
	{ "BuildGun", "/Game/FactoryGame/Resource/Equipment/BuildGun/BP_EquipmentDescriptorBuildGun.BP_EquipmentDescriptorBuildGun_C" },
	{ "NobeliskDetonator", "/Game/FactoryGame/Resource/Equipment/C4Dispenser/Desc_NobeliskDetonator.Desc_NobeliskDetonator_C" },
	{ "ColorGun", "/Game/FactoryGame/Resource/Equipment/ColorGun/BP_EquipmentDescriptorColorGun.BP_EquipmentDescriptorColorGun_C" },
	{ "Cup", "/Game/FactoryGame/Resource/Equipment/Cup/BP_EquipmentDescriptorCup.BP_EquipmentDescriptorCup_C" },
	{ "DowsingStick", "/Game/FactoryGame/Resource/Equipment/DowsingStick/Desc_DowsingStick.Desc_DowsingStick_C" },
	{ "Gasmask", "/Game/FactoryGame/Resource/Equipment/GasMask/BP_EquipmentDescriptorGasmask.BP_EquipmentDescriptorGasmask_C" },
	{ "ObjectScanner", "/Game/FactoryGame/Resource/Equipment/GemstoneScanner/BP_EquipmentDescriptorObjectScanner.BP_EquipmentDescriptorObjectScanner_C" },
	{ "GolfCart", "/Game/FactoryGame/Resource/Equipment/GolfCart/Desc_GolfCart.Desc_GolfCart_C" },
	{ "HazardSuit", "/Game/FactoryGame/Resource/Equipment/HazardSuit/BP_EquipmentDescriptorHazardSuit.BP_EquipmentDescriptorHazardSuit_C" },
	{ "HazmatSuit", "/Game/FactoryGame/Resource/Equipment/HazmatSuit/BP_EquipmentDescriptorHazmatSuit.BP_EquipmentDescriptorHazmatSuit_C" },
	{ "JetPack", "/Game/FactoryGame/Resource/Equipment/JetPack/BP_EquipmentDescriptorJetPack.BP_EquipmentDescriptorJetPack_C" },
	{ "JetPackMk2", "/Game/FactoryGame/Resource/Equipment/JetPack/BP_EquipmentDescriptorJetPackMk2.BP_EquipmentDescriptorJetPackMk2_C" },
	{ "JumpingStilts", "/Game/FactoryGame/Resource/Equipment/JumpingStilts/BP_EquipmentDescriptorJumpingStilts.BP_EquipmentDescriptorJumpingStilts_C" },
	{ "Machinegun", "/Game/FactoryGame/Resource/Equipment/Machinegun/BP_EquipmentDescriptorMachinegun.BP_EquipmentDescriptorMachinegun_C" },
	{ "RebarGun", "/Game/FactoryGame/Resource/Equipment/NailGun/Desc_RebarGun.Desc_RebarGun_C" },
	{ "RebarGunProjectile", "/Game/FactoryGame/Resource/Equipment/NailGun/Desc_RebarGunProjectile.Desc_RebarGunProjectile_C" },
	{ "NobeliskDetonatorEquip", "/Game/FactoryGame/Resource/Equipment/NobeliskDetonator/BP_EquipmentDescriptorNobeliskDetonator.BP_EquipmentDescriptorNobeliskDetonator_C" },
	{ "PortableMiner", "/Game/FactoryGame/Resource/Equipment/PortableMiner/BP_ItemDescriptorPortableMiner.BP_ItemDescriptorPortableMiner_C" },
	{ "RebarScatterGunProjectile", "/Game/FactoryGame/Resource/Equipment/RebarScatterGun/Desc_RebarScatterGunProjectile.Desc_RebarScatterGunProjectile_C" },
	{ "ResourceMiner", "/Game/FactoryGame/Resource/Equipment/ResourceMiner/BP_EquipmentDescriptorResourceMiner.BP_EquipmentDescriptorResourceMiner_C" },
	{ "Rifle", "/Game/FactoryGame/Resource/Equipment/Rifle/BP_EquipmentDescriptorRifle.BP_EquipmentDescriptorRifle_C" },
	{ "RifleMk2", "/Game/FactoryGame/Resource/Equipment/Rifle/BP_EquipmentDescriptorRifleMk2.BP_EquipmentDescriptorRifleMk2_C" },
	{ "ShockShank", "/Game/FactoryGame/Resource/Equipment/ShockShank/BP_EquipmentDescriptorShockShank.BP_EquipmentDescriptorShockShank_C" },
	{ "StunSpear", "/Game/FactoryGame/Resource/Equipment/StunSpear/BP_EquipmentDescriptorStunSpear.BP_EquipmentDescriptorStunSpear_C" },
	{ "ToolBelt", "/Game/FactoryGame/Resource/Equipment/ToolBelt/Desc_ToolBelt.Desc_ToolBelt_C" },
	{ "CharacterClap_Statue", "/Game/FactoryGame/Resource/Equipment/Decoration/Desc_CharacterClap_Statue.Desc_CharacterClap_Statue_C" },
	{ "CharacterRunStatue", "/Game/FactoryGame/Resource/Equipment/Decoration/Desc_CharacterRunStatue.Desc_CharacterRunStatue_C" },
	{ "CharacterSpin_Statue", "/Game/FactoryGame/Resource/Equipment/Decoration/Desc_CharacterSpin_Statue.Desc_CharacterSpin_Statue_C" },
	{ "DoggoStatue", "/Game/FactoryGame/Resource/Equipment/Decoration/Desc_DoggoStatue.Desc_DoggoStatue_C" },
	{ "GoldenNut_Statue", "/Game/FactoryGame/Resource/Equipment/Decoration/Desc_GoldenNut_Statue.Desc_GoldenNut_Statue_C" },
	{ "Hog_Statue", "/Game/FactoryGame/Resource/Equipment/Decoration/Desc_Hog_Statue.Desc_Hog_Statue_C" },
	{ "SpaceGiraffeStatue", "/Game/FactoryGame/Resource/Equipment/Decoration/Desc_SpaceGiraffeStatue.Desc_SpaceGiraffeStatue_C" },
	{ "Berry", "/Game/FactoryGame/Resource/Environment/Berry/Desc_Berry.Desc_Berry_C" },
	{ "Medkit", "/Game/FactoryGame/Resource/Equipment/Medkit/Desc_Medkit.Desc_Medkit_C" },
	{ "Nut", "/Game/FactoryGame/Resource/Environment/Nut/Desc_Nut.Desc_Nut_C" },
	{ "Shroom", "/Game/FactoryGame/Resource/Environment/DesertShroom/Desc_Shroom.Desc_Shroom_C" },
	{ "HealthGainDescriptor", "/Game/FactoryGame/Resource/BP_HealthGainDescriptor.BP_HealthGainDescriptor_C" },
	{ "Beacon", "/Game/FactoryGame/Resource/Equipment/Beacon/BP_EquipmentDescriptorBeacon.BP_EquipmentDescriptorBeacon_C" },
	{ "Parachute", "/Game/FactoryGame/Resource/Equipment/Beacon/Desc_Parachute.Desc_Parachute_C" },
	{ "CyberWagon", "/Game/FactoryGame/Buildable/Vehicle/Cyberwagon/Desc_CyberWagon.Desc_CyberWagon_C" },
	{ "Explorer", "/Game/FactoryGame/Buildable/Vehicle/Explorer/Desc_Explorer.Desc_Explorer_C" },
	{ "Tractor", "/Game/FactoryGame/Buildable/Vehicle/Tractor/Desc_Tractor.Desc_Tractor_C" },
	{ "Locomotive", "/Game/FactoryGame/Buildable/Vehicle/Train/Locomotive/Desc_Locomotive.Desc_Locomotive_C" },
	{ "FreightWagon", "/Game/FactoryGame/Buildable/Vehicle/Train/Wagon/Desc_FreightWagon.Desc_FreightWagon_C" },
	{ "Truck", "/Game/FactoryGame/Buildable/Vehicle/Truck/Desc_Truck.Desc_Truck_C" }
};

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