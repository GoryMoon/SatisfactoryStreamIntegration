// ILikeBanas

#pragma once

#include "CoreMinimal.h"
#include "mod/actor/SMLInitMod.h"
#include "Delegates/Delegate.h"
#include "SIInitMod.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGravityResetTimeNotification, float, timeLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnGravityResetFinished);

/**
 * 
 */
UCLASS()
class SI_API ASIInitMod : public ASMLInitMod
{
	GENERATED_BODY()

public:	
	/** Resets the trigger fuse state */
	UFUNCTION(BlueprintImplementableEvent, Category = "SI", meta = (DisplayName = "OnNotificationSetup"))
	void ReceiveOnNotificationSetup(UUserWidget* NotificationWidget);

	/** Broadcast a notification when we are about to reset gravity */
	UPROPERTY(BlueprintAssignable, Category = "Notification")
	FOnGravityResetTimeNotification mOnGravityResetTimeNotification;

	/** Broadcast a notification when we are finished resetting gravity */
	UPROPERTY(BlueprintAssignable, Category = "Notification")
	FOnGravityResetFinished mOnGravityResetFinished;
	
};
