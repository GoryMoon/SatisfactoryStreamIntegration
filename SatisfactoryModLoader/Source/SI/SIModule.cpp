#include "SIModule.h"
#include "../SML/mod/hooking.h"
#include "FGGameMode.h"
#include <fstream>

#include "mod/BlueprintLibrary.h"
#include "util/Logging.h"

void ParseConfig(const TSharedRef<FJsonObject>& json, StreamIntegration::SIConfig& config) {
	config.Username = json->GetStringField(TEXT("username"));
}

TSharedRef<FJsonObject> CreateConfigDefaults() {
	TSharedRef<FJsonObject> ref = MakeShareable(new FJsonObject());
	ref->SetStringField(TEXT("username"), "");
	return ref;
}



void FSIModule::StartupModule()
{
#if !WITH_EDITOR
	StreamIntegration::Running = true;
	const auto Json = SML::ReadModConfig("SI", CreateConfigDefaults());
	StreamIntegration::CurrentConfig = new StreamIntegration::SIConfig;
	ParseConfig(Json, *StreamIntegration::CurrentConfig);
#endif
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

	void SetTrigger(const bool b)
	{
		TriggerFuse = b;
	}

	bool GetTrigger()
	{
		return TriggerFuse;
	}
}

IMPLEMENT_GAME_MODULE(FSIModule, SI);
