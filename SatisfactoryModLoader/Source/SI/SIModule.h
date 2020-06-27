#pragma once

#include "Curves/CurveFloat.h"
#include "Modules/ModuleManager.h"


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

	SI_API extern const SIConfig& GetConfig();

	static bool Running;

	SI_API extern bool IsRunning();

	SI_API extern UCurveFloat* GetFallDamageOverride();
}

class FSIModule : public FDefaultGameModuleImpl {
public:
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;

	virtual bool IsGameModule() const override { return true; }
};

