#include "IntegrationSubsystem.h"

#include "action/ActionHandler.h"
#include "player/PlayerUtility.h"
#include "player/component/SMLPlayerComponent.h"

void AIntegrationSubsystem::BeginPlay()
{
	Super::BeginPlay();
	SI_INFO("Initializing integration subsystem");
	bStopTask = false;
	ActionHandler = new FActionHandler;
	ActionHandler->SetSubsystem(MakeShareable(this));
	
	(new FAutoDeleteAsyncTask<FIntegrationTask>(this))->StartBackgroundTask();


	GetWorldTimerManager().SetTimer(UpdateTimerHandle, this, &AIntegrationSubsystem::Update, 1.0f, true);
}

void AIntegrationSubsystem::BeginDestroy()
{
	Super::BeginDestroy();
	bStopTask = true;
}

void AIntegrationSubsystem::FinishDestroy()
{
	Super::FinishDestroy();
	bStopTask = true;
}

void AIntegrationSubsystem::Destroyed()
{
	Super::Destroyed();
	bStopTask = true;
}


FString ToTitle(const FString S)
{
	bool bLast = true;
	FString Out = S.Replace(TEXT("_"), TEXT(" "));
	for (int i = 0; i < Out.Len(); i++)
	{
		const TCHAR c = Out[i];
		Out[i] = bLast ? toupper(c) : tolower(c);
		bLast = isspace(c) == 1;
	}
	return Out;
}

void AIntegrationSubsystem::Update()
{
	FWork Work;
	while (WorkQueue.Dequeue(Work))
	{
		try
		{
			if (Work.bIsMessage)
			{
				SendMessageToAll(Work.Data, FLinearColor::Yellow);
			}
			else
			{
				TSharedPtr<FJsonObject> JsonObject = SML::parseJsonLenient(Work.Data);

				if (JsonObject.IsValid())
				{
					const auto Type = JsonObject->GetStringField(TEXT("type"));
					auto From = JsonObject->GetStringField(TEXT("from"));
					
					ActionHandler->HandleAction(Type, JsonObject);

					SendMessageToAll(From + FString(" ran action ") + ToTitle(Type), FLinearColor::Green);
				}	
				else
					SI_ERROR("Failed to parse action json data");
			}
		}
		catch (int e)
		{
			SI_ERROR("Error executing action: ", e);
			return;
		}
	}
}


void AIntegrationSubsystem::SendMessageToAll(const FString& Message, const FLinearColor& Color) const
{
	TArray<AFGPlayerController*> ConnectedPlayers = SML::GetConnectedPlayers(GetWorld());
	for (AFGPlayerController* Controller : ConnectedPlayers) {
		USMLPlayerComponent::Get(Controller)->SendChatMessage(Message, Color);
	}
}

void FIntegrationTask::DoWork()
{
	std::string Line;
	std::ifstream Stream;
	while (StreamIntegration::IsRunning() && Subsystem != nullptr && !Subsystem->IsStopping())
	{
		
		while (StreamIntegration::IsRunning() && Subsystem != nullptr && !Subsystem->IsStopping())
		{
			Stream.open(PIPE_NAME, std::ifstream::in);
			
			if (Stream.is_open())
				break;

			FPlatformProcess::Sleep(0.5);
		}
		if (!StreamIntegration::IsRunning() || Subsystem == nullptr || Subsystem->IsStopping())
			break;
		
		SI_INFO("Connected to integration");

	
		while ((StreamIntegration::IsRunning() && Subsystem != nullptr && !Subsystem->IsStopping()) && std::getline(Stream, Line))
		{
			SI_DEBUG("Pipe: ", Line.c_str());
			if (Line.rfind("Action: ", 0) == 0)
			{
				auto Action = Line.substr(8);
				SI_DEBUG("Action: ", Action.c_str());
				Subsystem->WorkQueue.Enqueue(FWork{ false, FString(Action.c_str()) });
			}
			else if (Line.rfind("Message: ", 0) == 0)
			{
				auto Message = Line.substr(9);
				SI_DEBUG("Message: ", Message.c_str());
				Subsystem->WorkQueue.Enqueue(FWork{ true, FString(Message.c_str()) });
			}	
		}
		
		if (Stream.is_open())
			Stream.close();
	}
	if (Stream.is_open())
		Stream.close();
	
	SI_INFO("Shutting down integration task");
}
