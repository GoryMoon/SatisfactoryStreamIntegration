#include "CharacterUtility.h"
#include "util/Logging.h"

template<typename T>
static FORCEINLINE T* FindOrLoadObject(const TCHAR* ObjectToFind)
{
	const FString PathName(ObjectToFind);
	UClass* Class = T::StaticClass();
	Class->GetDefaultObject(); // force the CDO to be created if it hasn't already
	T* ObjectPtr = LoadObject<T>(NULL, *PathName);
	if (ObjectPtr)
	{
		ObjectPtr->AddToRoot();
	}
	return ObjectPtr;
}

void StreamIntegration::Utility::Character::PlayClapEmote(AFGCharacterPlayer* Character)
{
	UFunction* Func = Character->GetClass()->FindFunctionByName(FName("ShowEmote"));
	int32 ID = 1;
	Character->ProcessEvent(Func, &ID);
}

void StreamIntegration::Utility::Character::PlayNarutoEmote(AFGCharacterPlayer* Character)
{
	auto Anim = Character->GetMesh1P()->GetAnimInstance();
	UAnimMontage* NarutoAnim = FindOrLoadObject<UAnimMontage>(TEXT("/Game/FactoryGame/Character/Player/Animation/FirstPerson/EmoteSigns_01_Montage.EmoteSigns_01_Montage"));
	Anim->Montage_Play(NarutoAnim);
		
	UFunction* Func = Character->GetClass()->FindFunctionByName(FName("Server_playSignsEmote"));
	Character->ProcessEvent(Func, NULL);
}

void StreamIntegration::Utility::Character::PlaySpinEmote(AFGCharacterPlayer* Character)
{
	UFunction* Func = Character->GetClass()->FindFunctionByName(FName("ShowEmote"));
	int32 ID = 2;
	Character->ProcessEvent(Func, &ID);
}
