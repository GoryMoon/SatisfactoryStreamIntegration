#include "SIModule.h"
#include "../SML/mod/hooking.h"
#include "FGGameMode.h"
#include <fstream>

#include "mod/BlueprintLibrary.h"

void parseConfig(const TSharedRef<FJsonObject>& json, StreamIntegration::SIConfig& config) {
	config.Username = json->GetStringField(TEXT("username"));
}

TSharedRef<FJsonObject> createConfigDefaults() {
	TSharedRef<FJsonObject> ref = MakeShareable(new FJsonObject());
	ref->SetStringField(TEXT("username"), "");
	return ref;
}



void FSIModule::StartupModule()
{
#if !WITH_EDITOR
	StreamIntegration::Running = true;
	const auto Json = SML::readModConfig("SI", createConfigDefaults());
	StreamIntegration::CurrentConfig = new StreamIntegration::SIConfig;
	parseConfig(Json, *StreamIntegration::CurrentConfig);
#endif

	/*SUBSCRIBE_METHOD("?InitGameState@AFGGameMode@@UEAAXXZ", AFGGameMode::InitGameState, [](auto& scope, AFGGameMode* gameMode) {
		AExampleActor* actor = gameMode->GetWorld()->SpawnActor<AExampleActor>(FVector::ZeroVector, FRotator::ZeroRotator);
		actor->DoStuff();
	});*/
}

void FSIModule::ShutdownModule()
{
#if !WITH_EDITOR
	StreamIntegration::Running = false;
#endif
}

namespace StreamIntegration
{
	const SIConfig& GetConfig() {
		return *CurrentConfig;
	}

	bool IsRunning()
	{
		return Running;
	}

	UCurveFloat* GetFallDamageOverride()
	{
		if (NoFallDamage == nullptr)
		{
			NoFallDamage = Cast<UCurveFloat>(FStringAssetReference("/Game/SI/NoFallDamage.NoFallDamage").TryLoad());
		}
		return NoFallDamage;
	}
}

IMPLEMENT_GAME_MODULE(FSIModule, SI);
