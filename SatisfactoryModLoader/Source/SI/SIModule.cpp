#include "SIModule.h"
#include "FGGameMode.h"
#include "FGGameUI.h"
#include <fstream>

#include "mod/BlueprintLibrary.h"
#include "util/Logging.h"
#include "SatisfactoryModLoader.h"
#include "mod/BPHookHelper.h"

void ParseConfig(const TSharedRef<FJsonObject>& json, StreamIntegration::SIConfig& config) {
	config.Username = json->GetStringField(TEXT("username"));
}

TSharedRef<FJsonObject> CreateConfigDefaults() {
	TSharedRef<FJsonObject> ref = MakeShareable(new FJsonObject());
	ref->SetStringField(TEXT("username"), "");
	return ref;
}

extern void RegisterGravityNotification();

void FSIModule::StartupModule()
{
#if !WITH_EDITOR
	StreamIntegration::Running = true;
	RegisterGravityNotification();
	
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

	ASIInitMod* SetModActor(AActor* Actor)
	{
		ModActor = Cast<ASIInitMod>(Actor);
		return ModActor;
	}

	ASIInitMod* GetModActor()
	{
		return ModActor;
	}
}

IMPLEMENT_GAME_MODULE(FSIModule, SI);
