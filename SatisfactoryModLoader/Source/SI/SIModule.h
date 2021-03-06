#pragma once

#include "ThreadSafeBool.h"
#include "Curves/CurveFloat.h"
#include "Modules/ModuleManager.h"
#include "SIInitMod.h"


#define SI_INFO(msg, ...) SML::Logging::info(TEXT("[SI] " msg), __VA_ARGS__)
#define SI_DEBUG(msg, ...) SML::Logging::debug(TEXT("[SI] " msg), __VA_ARGS__)
#define SI_WARN(msg, ...) SML::Logging::warning(TEXT("[SI] " msg), __VA_ARGS__)
#define SI_ERROR(msg, ...) SML::Logging::error(TEXT("[SI] " msg), __VA_ARGS__)

namespace StreamIntegration
{
	struct SIConfig
	{
		FString Username = "";
	};
	
	static SIConfig* CurrentConfig;
	static UCurveFloat* NoFallDamage = nullptr;
	static FThreadSafeBool TriggerFuse = false;
	static ASIInitMod* ModActor = nullptr;
	
	SI_API extern const SIConfig& GetConfig();

	static bool Running;

	SI_API extern bool IsRunning();

	SI_API extern UCurveFloat* GetFallDamageOverride();

	SI_API extern void SetTrigger(bool b);
	
	SI_API extern bool GetTrigger();

	SI_API extern ASIInitMod* SetModActor(AActor* actor);
	SI_API extern ASIInitMod* GetModActor();
}

class FSIModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }
};

