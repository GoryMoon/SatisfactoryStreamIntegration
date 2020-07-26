#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SIBlueprintFunctionLibrary.generated.h"

/**
 * 
 */
UCLASS()
class SI_API USIBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	/**
	 * Returns if the fuse should be triggered
	 */
	UFUNCTION(BlueprintPure, Category = "SI")
	static bool ShouldTriggerFuse();

	/**
	 * Resets the trigger fuse state
	 */
	UFUNCTION(BlueprintCallable, Category = "SI")
	static void ResetTriggerFuse();
};
