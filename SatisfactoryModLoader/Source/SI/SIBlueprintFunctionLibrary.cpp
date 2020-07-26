#include "SIBlueprintFunctionLibrary.h"
#include "SIModule.h"

bool USIBlueprintFunctionLibrary::ShouldTriggerFuse()
{
	return StreamIntegration::GetTrigger();
}

void USIBlueprintFunctionLibrary::ResetTriggerFuse()
{
	StreamIntegration::SetTrigger(false);
}
