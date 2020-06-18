#include "SISubsystemHolder.h"

void USISubsystemHolder::InitSubsystems()
{
	SpawnSubsystem(IntegrationSubsystem, AIntegrationSubsystem::StaticClass(), "IntegrationSubsystem");
}
