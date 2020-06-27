#pragma once

#include "SIModule.h"

#include "CoreMinimal.h"
#include "action/ActionHandler.h"
#include "action/Work.h"

#include "util/Logging.h"
#include "Async/AsyncWork.h"
#include "Delegates/Delegate.h"


#include "IntegrationSubsystem.generated.h"

#define PIPE_NAME "\\\\.\\pipe\\Satisfactory"

/**
 * 
 */
UCLASS()
class SI_API AIntegrationSubsystem : public AFGSubsystem
{
	GENERATED_BODY()

public:
	AIntegrationSubsystem() = default;

	// AActor functions
	void BeginPlay() override;
	void BeginDestroy() override;
	void FinishDestroy() override;
	void Destroyed() override;
	// AActor
	
	TQueue<FWork, EQueueMode::Mpsc> WorkQueue;
	
	FORCEINLINE bool IsStopping() const
	{
		return bStopTask;
	}
	
protected:
	UFUNCTION()
	void Update();

	FTimerHandle UpdateTimerHandle;
	FThreadSafeBool bStopTask = false;
	class AActionHandler* ActionHandler = nullptr;
	
	void SendMessageToAll(const FString& Message, const FLinearColor& Color) const;
};


class SI_API FIntegrationTask: public FNonAbandonableTask
{
	friend class FAutoDeleteAsyncTask<FIntegrationTask>;
public:
	
	FORCEINLINE FIntegrationTask(AIntegrationSubsystem* Subsystem): Subsystem(Subsystem) {}
	FORCEINLINE void DoWork();

	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FIntegrationTask, STATGROUP_ThreadPoolAsyncTasks);
	}

protected:
	
	AIntegrationSubsystem* Subsystem = nullptr;
};