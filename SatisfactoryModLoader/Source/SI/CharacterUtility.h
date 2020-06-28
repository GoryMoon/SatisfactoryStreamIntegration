#pragma once

#include "CoreMinimal.h"
#include "FGCharacterPlayer.h"
#include "FGPlayerController.h"
#include "SIModule.h"

namespace StreamIntegration
{
	namespace Utility
	{
		namespace Character
		{
			SI_API extern void PlayClapEmote(AFGCharacterPlayer* Character);
			SI_API extern void PlayNarutoEmote(AFGCharacterPlayer* Character);
			SI_API extern void PlaySpinEmote(AFGCharacterPlayer* Character);

		}
	}
}
