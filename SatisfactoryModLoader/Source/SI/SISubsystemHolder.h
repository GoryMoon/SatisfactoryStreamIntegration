#pragma once

#include "CoreMinimal.h" 
#include "mod/ModSubsystems.h"
#include "integration/IntegrationSubsystem.h"

#include "SISubsystemHolder.generated.h"

/**
 * 
 */
UCLASS()
class SI_API USISubsystemHolder : public UModSubsystemHolder
{
	GENERATED_BODY()
private:
	UPROPERTY()
	AIntegrationSubsystem* IntegrationSubsystem;
public:
	FORCEINLINE AIntegrationSubsystem* GetIntegrationSubsystem() const { return IntegrationSubsystem; }
	void InitSubsystems() override;
	
};
