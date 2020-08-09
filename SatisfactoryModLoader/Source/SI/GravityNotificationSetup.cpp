
#include "FGGameUI.h"
#include "SIModule.h"
#include "util/ReflectionHelper.h"
#include "util/Logging.h"
#include "mod/BPHookHelper.h"

void RegisterGravityNotification() {
	UClass* GameUI = LoadObject<UClass>(nullptr, TEXT("/Game/FactoryGame/Interface/UI/InGame/BP_GameUI.BP_GameUI_C"));
	check(GameUI);
	
	UFunction* ConstructFunction = GameUI->FindFunctionByName(TEXT("Construct"));
	check(ConstructFunction);
	HookBlueprintFunction(ConstructFunction, [](FBlueprintHookHelper& HookHelper) {
		UUserWidget* GameUI = Cast<UUserWidget>(HookHelper.GetContext());

		UClass* NotificationClass = LoadObject<UClass>(nullptr, TEXT("/Game/SI/Interface/UI/InGame/BPW_GravityNotification.BPW_GravityNotification_C"));

		UOverlay* Container = FReflectionHelper::GetObjectPropertyValue<UOverlay>(GameUI, TEXT("mHudContainer"));
		UUserWidget* Notification = UUserWidget::CreateWidgetInstance(*GameUI, NotificationClass, TEXT("BPW_GravityNotification"));
		UOverlaySlot* OverlaySlot = Container->AddChildToOverlay(Notification);
		OverlaySlot->SetPadding(*new FMargin(0, 200, 0, 0));
		
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GameUI, ASIInitMod::StaticClass(), FoundActors);
		if (FoundActors.Num() > 0)
		{
			StreamIntegration::SetModActor(FoundActors[0])->ReceiveOnNotificationSetup(Notification);
		}
	}, Return);
}
