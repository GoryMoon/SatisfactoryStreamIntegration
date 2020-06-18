#pragma once

#include "CoreMinimal.h"
#include "Work.generated.h"

USTRUCT()
struct SI_API FWork
{
	GENERATED_BODY()

	FWork()
		: bIsMessage(true),
		Data(TEXT("Default"))
	{}

	explicit FWork(const bool bIsMessage, const FString Data)
		: bIsMessage(bIsMessage),
		  Data(Data)
	{}

	bool bIsMessage;
	FString Data;
	
	FORCEINLINE ~FWork() = default;
};
