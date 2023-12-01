// Fill out your copyright notice in the Description page of Project Settings.

#include "TestCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Animation/AnimMontage.h"
#include "Engine/World.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/Core/PropertyConditions/PropertyConditions.h"
#include "Net/UnrealNetwork.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/DamageEvents.h"
#include "Kismet/KismetMathLibrary.h"
#include "Blueprint/UserWidget.h"
#include "TestMovement.h"
#include "StatsComponent.h"

FName ATestCharacter::CharacterMovementComponentName(TEXT("CharMoveComp"));

// Sets default values
ATestCharacter::ATestCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	TestMovement = CreateDefaultSubobject<UTestMovement>(ATestCharacter::CharacterMovementComponentName);

	StatsComponent = CreateDefaultSubobject<UStatsComponent>(TEXT("StatsComponent"));
	StatsComponent->SetIsReplicated(true);

	BaseMoveSpeed = 600;
	JumpForce = 1000;
	Gravity = 3000;
	GroundFriction = 10;
	AirFriction = 0.5;

	CorrectionDistance = 15;
	CorrectionVelocity = 25;
	LeniantCorrectionDistance = 100;
	LeniantCorrectionVelocity = 250;
	CheckMovableRange = 500;
	MinTimeBetweenCorrections = 1;
	ClearDataThreshold = 120;

	TickTime = 1;
	ClientDataInterval = 10;
	FixedTickPerSecond = 60;
	ServerDelay = 2;
	MaxFixedTicksPerFrame = 10;
	ServerMaxFixedTicksPerFrame = 10;

	ExternalMovementSourceDuration = 30;
	MinTimeBetweenDataSend = 1;

	NoInputTimeStamp = -1;
}

// Called when the game starts or when spawned
void ATestCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	PreviousLocation = GetActorLocation();
	MoveSpeed = BaseMoveSpeed;

	FixedTickRate = 1.f / FixedTickPerSecond;

	TimeStamp = 0;
}

// Called every frame
void ATestCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PendingActions();

	if (FixedTickDelta > 0.5)
	{
		FixedTickDelta = 0.5;
	}

	if (DeltaTime > 0.5)
	{
		DeltaTime = 0.5;
	}

	FixedTickDelta = FixedTickDelta + DeltaTime;
	int tickcount = 0;

	if (GetLocalRole() == ROLE_Authority)
	{
		while (FixedTickDelta >= FixedTickRate && tickcount < ServerMaxFixedTicksPerFrame)
		{
			FixedTickDelta = FixedTickDelta - FixedTickRate;

			if (ReceivedClientInputOnce)
			{
				FixedTick(FixedTickRate);
			}

			++tickcount;
		}
	}

	else
	{
		while (FixedTickDelta >= FixedTickRate && tickcount < MaxFixedTicksPerFrame)
		{
			FixedTickDelta = FixedTickDelta - FixedTickRate;
			FixedTick(FixedTickRate);

			++tickcount;
		}
	}
}

void ATestCharacter::FixedTick(float DeltaTime)
{
	++InputCount;

	if (InputCount >= TickTime)
	{
		InputCount = 0;
		++TimeStamp;

		if (GetLocalRole() == ROLE_Authority)
		{
			ServerTick(DeltaTime);
		}

		if (GetLocalRole() == ROLE_AutonomousProxy)
		{
			AutonomousProxyTick(DeltaTime);
		}

		if (GetLocalRole() == ROLE_SimulatedProxy)
		{
			SimulatedProxyTick(DeltaTime);
		}
	}

	if (GetLocalRole() != ROLE_SimulatedProxy)
	{
		if (TimeStamp % 60 == 0)
		{
			ClearOldData();

			//->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("%i"), InputsMap.Num()));
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Blue, FString::Printf(TEXT("%i"), SavedDataMap.Num()));
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%i"), ClientDataMap.Num()));
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Purple, FString::Printf(TEXT("%i"), ServerAuthoritativeDataMap.Num()));
			//GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Black, FString::Printf(TEXT("%i"), HitsMap.Num()));
		}
	}
}

void ATestCharacter::ServerTick(float DeltaTime)
{
	if (!ReceivedClientInputOnce)
	{
		return;
	}

	//Handling Inputs

	SaveCurrentData();

	if (NoInputTimeStamp != -1)
	{
		bool HasAllInputs = true;

		const FInputs OriginalMissingInput = InputsMap.FindRef(NoInputTimeStamp);
		const FInputs CurrentTimeStampInput = InputsMap.FindRef(TimeStamp);

		HasAllInputs = (OriginalMissingInput.TimeStamp != -1 && CurrentTimeStampInput.TimeStamp != -1);

		if (HasAllInputs)
		{
			NotifyClientLossHandled();

			ReplayMoves = true;
			SimulateTimeStamp = NoInputTimeStamp;

			bIgnoreClientError = false;
		}
	}

	if (ReplayMoves)
	{
		RewindAndReplayMoves(SimulateTimeStamp);
	}

	FInputs PendingInput = FindInput();

	if (PendingInput.TimeStamp != -1)
	{
		CurrentInput = PendingInput;
	}

	else
	{
		if (NoInputTimeStamp == -1)
		{
			NoInputTimeStamp = TimeStamp;

			NoInputStartTime = GetWorld()->GetTimeSeconds();

			bIgnoreClientError = true;

			NotifyClientOfLoss();
		}
	}

	if (NoInputTimeStamp != -1)
	{
		if (GetWorld()->GetTimeSeconds() - NoInputStartTime > 1)
		{
			NoInputTimeStamp = -1;

			bIgnoreClientError = false;

			NotifyClientLossHandled();
		}
	}

	//Handling Logic

	HandleInput(CurrentInput);
	ForwardRotation = FRotator(0, Rotation.Yaw, 0);

	TestMovement->UpdatedComponent->SetWorldRotation(ForwardRotation);

	InputVelocity.Normalize();
	InputVelocity = InputVelocity * MoveSpeed;

	TickOnAllComponents(DeltaTime);

	InputVelocity = FVector::ZeroVector;

	if (GetWorld()->GetTimeSeconds() - LastTimeSentServerResponse > 3)
	{
		LastTimeSentServerResponse = GetWorld()->GetTimeSeconds();

		AckMove(TimeStamp);
	}

	if (!bExternalMovementSource)
	{
		CheckClientError();
	}

	else
	{
		if (InputCount == 0)
		{
			if (SendExternalMovement)
			{
				SendExternalMovement = false;

				ServerAuthoritativeMove(TimeStamp, GetActorLocation(), LastVelocity, PreviousLocation, bNoFriction);
			}
		}

		if (TimeStamp - ExternalMovementSourceStartTime >= ExternalMovementSourceDuration)
		{
			bExternalMovementSource = false;
			bNoFriction = false;
		}
	}
}

void ATestCharacter::AutonomousProxyTick(float DeltaTime)
{
	//Handling Corrections and Input

	AssignInputs();

	if (ReplayMoves)
	{
		ReplayMovesAfterCorrection();
	}

	FInputs Input;
	Input.TimeStamp = TimeStamp;
	Input.Rotation = Controller->GetControlRotation();

	AfterAssignInputs(Input);

	if (Input.TimeStamp != -1)
	{
		CurrentInput = Input;
		InputsMap.Emplace(Input.TimeStamp, Input);
	}

	if (bSendAllNotAckedInputs)
	{
		SendAllPendingInputs();

		if (GetWorld()->GetTimeSeconds() - SendAllNotAckedInputStartTime >= 0.5)
		{
			bSendAllNotAckedInputs = false;
		}
	}

	else
	{
		SendInputData();
	}

	//Handling Logic

	HandleInput(CurrentInput);
	ForwardRotation = FRotator(0, Rotation.Yaw, 0);

	InputVelocity.Normalize();
	InputVelocity = InputVelocity * MoveSpeed;

	TickOnAllComponents(DeltaTime);

	InputVelocity = FVector::ZeroVector;

	if (bExternalMovementSource)
	{
		if (TimeStamp - ExternalMovementSourceStartTime >= ExternalMovementSourceDuration)
		{
			bExternalMovementSource = false;
			bNoFriction = false;
		}
	}

	HandleClientData();
}

void ATestCharacter::SimulatedProxyTick(float DeltaTime)
{
	if (bExternalMovementSource)
	{
		TestMovement->SimulatedMovementTick(DeltaTime, bReceivedMovementData);

		SimulatedProxyPosition = GetActorLocation();

		bReceivedMovementData = false;

		if (TimeStamp - ExternalMovementSourceStartTime >= ExternalMovementSourceDuration)
		{
			bExternalMovementSource = false;
			bNoFriction = false;

			LastVelocity = FVector::ZeroVector;

			bPauseSimulation = true;
			PauseSimulationStartTime = TimeStamp;
		}
	}

	if (bPauseSimulation)
	{
		SimulatedProxyPosition = GetActorLocation();

		if (TimeStamp - PauseSimulationStartTime > ((2 * SimulatedProxyPing) / 1000) * FixedTickPerSecond / TickTime)
		{
			bPauseSimulation = false;
		}
	}
}

void ATestCharacter::PendingActions()
{
	if (SendPendingHit)
	{
		SendPendingHit = false;

		SendHits(PendingHit);
	}
}

#pragma region Hits

void ATestCharacter::HitDetected(ATestCharacter* hitplayer, const bool mouse2, const bool mouse1, const bool EHit, const bool QHit)
{
	FHits hits;
	hits.TimeStamp = TimeStamp;
	hits.HitPlayer = hitplayer;
	hits.Mouse2Hit = mouse2;
	hits.EHit = EHit;
	hits.QHit = QHit;

	HitsMap.Emplace(hits.TimeStamp, hits);

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		SendHits(hits);

		SendPendingHit = true;
		PendingHit = hits;
	}
}

bool ATestCharacter::CheckHits(ATestCharacter*& hitplayer, const bool mouse1, const bool mouse2, const bool EHit, const bool QHit)
{
	FHits hits = HitsMap.FindRef(TimeStamp);
	bool hitdetected = false;

	if (hits.TimeStamp == -1)
	{
		return false;
	}

	if (mouse2)
	{
		if (hits.Mouse2Hit)
		{
			hitdetected = true;

			hitplayer = hits.HitPlayer;
		}
	}
	if (EHit)
	{
		if (hits.EHit)
		{
			hitdetected = true;

			hitplayer = hits.HitPlayer;
		}
	}
	if (QHit)
	{
		if (hits.QHit)
		{
			hitdetected = true;

			hitplayer = hits.HitPlayer;
		}
	}

	return hitdetected;
}

void ATestCharacter::SendHits_Implementation(const FHits hits)
{
	if (hits.TimeStamp >= TimeStamp)
	{
		HitsMap.Emplace(hits.TimeStamp, hits);
	}
}

void ATestCharacter::HandleExternalMove()
{
	FServerAuthoritativeData Data = ServerAuthoritativeDataMap.FindRef(TimeStamp);

	if (Data.TimeStamp != -1)
	{
		LastVelocity = Data.LaunchVelocity;
		bNoFriction = Data.NoFriction;
		bExternalMovementSource = true;
		ExternalMovementSourceStartTime = TimeStamp;
	}

	if (bExternalMovementSource)
	{
		if (TimeStamp - ExternalMovementSourceStartTime >= ExternalMovementSourceDuration)
		{
			bExternalMovementSource = false;
			bNoFriction = false;
		}
	}
}

void ATestCharacter::SetPlayerToPreviousPosition(ATestCharacter* hitplayer)
{
	float PlayerPing = GetPlayerState()->GetPingInMilliseconds();

	int RewindTime = static_cast<int>((PlayerPing / 1000) * FixedTickPerSecond / TickTime);

	hitplayer->ResetToTimeStamp(hitplayer->TimeStamp - RewindTime);

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%f"), PlayerPing));
	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%i"), RewindTime));
}

void ATestCharacter::Launch(const FVector launchvelocity, const float pingInMilliSeconds, const bool nofriction)
{
	FServerAuthoritativeData Data;

	Data.TimeStamp = TimeStamp;
	Data.LaunchVelocity = launchvelocity;
	Data.NoFriction = nofriction;

	ServerAuthoritativeDataMap.Emplace(Data.TimeStamp, Data);

	LastVelocity = launchvelocity;
	PreviousLocation = GetActorLocation();
	SimulatedProxyPosition = PreviousLocation;

	bExternalMovementSource = true;
	SendExternalMovement = true;
	bNoFriction = nofriction;
	ExternalMovementSourceStartTime = TimeStamp;

	SimulatedProxyPing = pingInMilliSeconds;
}

void ATestCharacter::ServerAuthoritativeMove_Implementation(int timestamp1, const FVector position, const FVector lastvelocity, const FVector previouslocation, const bool nofriction)
{
	FServerAuthoritativeData Data;
	Data.TimeStamp = timestamp1;
	Data.LaunchVelocity = lastvelocity;
	Data.NoFriction = nofriction;

	SetActorLocation(position);
	LastVelocity = lastvelocity;
	PreviousLocation = previouslocation;
	bNoFriction = nofriction;

	bExternalMovementSource = true;
	ExternalMovementSourceStartTime = TimeStamp;
}

#pragma endregion

#pragma region ServerRewinding

void ATestCharacter::NotifyClientOfLoss_Implementation()
{
	bSendAllNotAckedInputs = true;

	SendAllNotAckedInputStartTime = GetWorld()->GetTimeSeconds();
}

void ATestCharacter::NotifyClientLossHandled_Implementation()
{
	bSendAllNotAckedInputs = false;
}

void ATestCharacter::SendAllPendingInputs()
{
	TArray<FInputs> PendingInputs;

	for (auto& Elem : InputsMap)
	{
		if (Elem.Key <= TimeStamp)
		{
			PendingInputs.Emplace(Elem.Value);
		}
	}

	SendArrayInput(PendingInputs);
}

void ATestCharacter::SendArrayInput_Implementation(const TArray<FInputs>& pendinginputs)
{
	ReceivedClientInputOnce = true;

	int mostRecentInputTime = -1;
	int inputTimeStamp = -1;

	for (auto& i : pendinginputs)
	{
		inputTimeStamp = i.TimeStamp;

		InputsMap.Emplace(inputTimeStamp, i);

		if (inputTimeStamp > mostRecentInputTime)
		{
			mostRecentInputTime = inputTimeStamp;
		}
	}

	CheckClientTimeError(mostRecentInputTime);

	GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Red, FString::Printf(TEXT("%i"), pendinginputs.Num()));
}

void ATestCharacter::RewindAndReplayMoves(int StartSimulationTimeStamp)
{
	ResetToTimeStamp(StartSimulationTimeStamp);

	int currentTime = TimeStamp;
	TimeStamp = SimulateTimeStamp;

	bIsSimulating = true;

	while (TimeStamp < currentTime)
	{
		SaveCurrentData();

		FInputs PendingInput = SimulateFindInput();

		if (PendingInput.TimeStamp != -1)
		{
			CurrentInput = PendingInput;
		}

		for (int i = 0; i < TickTime; i++)
		{
			HandleInput(CurrentInput);

			ForwardRotation = FRotator(0, Rotation.Yaw, 0);

			InputVelocity.Normalize();
			InputVelocity = InputVelocity * MoveSpeed;

			HandleExternalMove();

			TickOnAllComponents(FixedTickRate);

			InputVelocity = FVector::ZeroVector;

			if (!bExternalMovementSource)
			{
				//CheckClientError();
			}
		}

		++TimeStamp;
	}

	bIsSimulating = false;
	ReplayMoves = false;
	NoInputTimeStamp = -1;
}

void ATestCharacter::ResetToTimeStamp(int ResetTimeStamp)
{
	FSavedData PendingData = SavedDataMap.FindRef(ResetTimeStamp);

	if (PendingData.TimeStamp == -1)
	{
		return;
	}

	ResetToData(PendingData);
}

void ATestCharacter::SaveCurrentData()
{
	FSavedData Data;

	Data.TimeStamp = TimeStamp;
	Data.Position = GetActorLocation();
	Data.Rotation = Rotation;
	Data.PreviousPosition = PreviousLocation;
	Data.LastVelocity = LastVelocity;
	Data.NoMovement = NoMovement;
	Data.MoveSpeed = MoveSpeed;

	SavedDataMap.Emplace(Data.TimeStamp, Data);
}

void ATestCharacter::ResetToData(const FSavedData Data)
{
	SetActorLocation(Data.Position);
	Rotation = Data.Rotation;
	PreviousLocation = Data.PreviousPosition;
	LastVelocity = Data.LastVelocity;
	NoMovement = Data.NoMovement;
	MoveSpeed = Data.MoveSpeed;
}

#pragma endregion

#pragma region ClientCorrections

void ATestCharacter::CheckClientError()
{
	if (bIgnoreClientError)
	{
		return;
	}

	if (TimeStamp == ClientReceivedCorrectionTime)
	{
		bClientReceivedCorrection = true;
	}

	if (GetWorld()->GetTimeSeconds() - LastCorrectionSentTime < MinTimeBetweenCorrections && !bClientReceivedCorrection)
	{
		return;
	}

	const FClientData i = ClientDataMap.FindRef(TimeStamp);

	if (i.TimeStamp == -1)
	{
		return;
	}

	bool bNOerrordetected;

	FVector ClientPosition = i.Position;
	FVector ClientPreviousPosition = i.PreviousPosition;
	FVector ClientLastVelocity = i.LastVelocity;

	FVector ServerLocation = GetActorLocation();

	/*
	const FVector& Start = ServerLocation;
	const float Radius = CheckMovableRange;
	ECollisionChannel TraceChannel = ECC_Pawn;
	TArray <FOverlapResult> Overlaps;
	const FCollisionQueryParams TraceParams = GetIgnoreCharacterParams();

	bool bActorsInRange = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Start,
		FQuat(),
		TraceChannel,
		FCollisionShape::MakeSphere(Radius),
		TraceParams
	);

	if (bActorsInRange)
	{
		for (auto& overlap : Overlaps)
		{
			AActor* hitactor = overlap.GetActor();
			ATestCharacter* hittestcharacter = Cast<ATestCharacter>(hitactor);

			if (hittestcharacter->IsValidLowLevel())
			{
				bLenientClientErrorDetection = true;
			}
		}
	}

	if (bLenientClientErrorDetection)
	{
		float serverdistancetraveled = (ServerLocation - PreviousServerLocation).Length();
		float servervelocitymagnitude = LastVelocity.Length();
		float serverdiffpreviouscurrentlocation = (ServerLocation - PreviousLocation).Length();

		float clientdistancetraveled = (ClientPosition - PreviousClientLocation).Length();
		float clientvelocitymagnitude = ClientLastVelocity.Length();
		float clientdiffpreviouscurrentlocation = (ClientPosition - ClientPreviousPosition).Length();

		bNOerrordetected = clientdistancetraveled <= serverdistancetraveled + LeniantCorrectionDistance && clientvelocitymagnitude < servervelocitymagnitude + LeniantCorrectionVelocity && clientdiffpreviouscurrentlocation < serverdiffpreviouscurrentlocation + LeniantCorrectionVelocity;
	}

	else
	*/

	{
		float distancebetweenclientserver = (ClientPosition - ServerLocation).Length();
		float diffclientservervelocity = (ClientLastVelocity - LastVelocity).Length();
		float diffclientserverpreviouslocation = (ClientPreviousPosition - PreviousLocation).Length();

		bNOerrordetected = distancebetweenclientserver <= CorrectionDistance && diffclientservervelocity <= CorrectionVelocity && diffclientserverpreviouslocation <= CorrectionVelocity;
	}

	if (bNOerrordetected)
	{
		SetActorLocation(ClientPosition);
		PreviousLocation = ClientPreviousPosition;
		LastVelocity = ClientLastVelocity;

		LastTimeSentServerResponse = GetWorld()->GetTimeSeconds();

		AckMove(TimeStamp);

		for (auto& Elem : InputsMap)
		{
			if (Elem.Key < TimeStamp)
			{
				InputsMap.Remove(Elem.Key);
			}
		}

		PreviousClientLocation = ClientPosition;
		PreviousServerLocation = ServerLocation;
	}

	else
	{
		LastCorrectionSentTime = GetWorld()->GetTimeSeconds();
		bClientReceivedCorrection = false;

		LastTimeSentServerResponse = GetWorld()->GetTimeSeconds();

		MakeCorrectionData();

		DrawDebugCapsule(GetWorld(), ServerLocation, GetCapsuleHeight(), GetCapsuleRadius(), FQuat::Identity, FColor(100, 255, 100), false, 3);
		DrawDebugCapsule(GetWorld(), ClientPosition, GetCapsuleHeight(), GetCapsuleRadius(), FQuat::Identity, FColor(255, 255, 100), false, 3);
	}

	bLenientClientErrorDetection = false;

	ClientDataMap.Remove(TimeStamp);

	for (auto& Elem : ClientDataMap)
	{
		if (Elem.Key <= TimeStamp)
		{
			ClientDataMap.Remove(Elem.Key);
		}
	}
}

void ATestCharacter::ReplayMovesAfterCorrection()
{
	int currentTime = TimeStamp;
	TimeStamp = SimulateTimeStamp;
	bool firstsimulation = true;

	bIsSimulating = true;

	if (TickTime == 1)
	{
		TimeStamp++;
	}

	while (TimeStamp < currentTime)
	{
		FInputs PendingInput = SimulateFindInput();

		if (PendingInput.TimeStamp != -1)
		{
			CurrentInput = PendingInput;
		}

		for (int i = 0; i < TickTime; i++)
		{
			HandleInput(CurrentInput);

			ForwardRotation = FRotator(0, Rotation.Yaw, 0);

			InputVelocity.Normalize();
			InputVelocity = InputVelocity * MoveSpeed;

			HandleExternalMove();

			TickOnAllComponents(FixedTickRate);

			InputVelocity = FVector::ZeroVector;

			if (firstsimulation)
			{
				firstsimulation = false;

				i++;
			}
		}

		++TimeStamp;
	}

	ReceivedCorrection(TimeStamp);

	bIsSimulating = false;
	ReplayMoves = false;
}

void ATestCharacter::AckMove_Implementation(int timestamp1)
{
	MostRecentAckedMoveTimeStamp = timestamp1;

	for (auto& Elem : InputsMap)
	{
		if (Elem.Key < MostRecentAckedMoveTimeStamp)
		{
			InputsMap.Remove(Elem.Key);
		}
	}
}

void ATestCharacter::SendClientTimeCorrection_Implementation(int timetoadd)
{
	TimeStamp = TimeStamp + timetoadd;
}

void ATestCharacter::HandleClientData()
{
	++ClientDataTracker;

	if (ClientDataTracker >= ClientDataInterval)
	{
		ClientDataTracker = 0;

		FClientData Data;
		Data.TimeStamp = TimeStamp;
		Data.Position = GetActorLocation();
		Data.PreviousPosition = PreviousLocation;
		Data.LastVelocity = LastVelocity;

		SendClientData(Data);
	}
}

void ATestCharacter::SendClientData_Implementation(const FClientData clientdata)
{
	if (clientdata.TimeStamp >= TimeStamp)
	{
		ClientDataMap.Emplace(clientdata.TimeStamp, clientdata);
	}

	CheckClientTimeError(clientdata.TimeStamp);
}

void ATestCharacter::SendClientCorrection_Implementation(int timestamp1, const FVector position, const FRotator rotation, const FVector previouslocation, const FVector lastvelocity, const float movespeed)
{
	AfterCorrectionReceived();

	SimulateTimeStamp = timestamp1;
	SetActorLocation(position);
	Rotation = rotation;
	PreviousLocation = previouslocation;
	LastVelocity = lastvelocity;
	MoveSpeed = movespeed;
}

void ATestCharacter::ReceivedCorrection_Implementation(int timeCorrectionReceived)
{
	ClientReceivedCorrectionTime = timeCorrectionReceived;
}

void ATestCharacter::AfterCorrectionReceived()
{
	ReplayMoves = true;
	EndAllMovementAbilities();
}

void ATestCharacter::CheckClientTimeError(int clienttime)
{
	if (GetWorld()->GetTimeSeconds() - LastTimeSentClientTimeCorrection < 2)
	{
		return;
	}

	TotalTimes++;

	int CorrectTime = TimeStamp + ServerDelay;

	TotalTimeDifference = TotalTimeDifference + CorrectTime - clienttime;

	if (clienttime < TimeStamp)
	{
		LateInputsCount++;
	}

	if (clienttime > TimeStamp + 2 * ServerDelay)
	{
		EarlyInputsCount++;
	}

	if (LateInputsCount >= 3 || EarlyInputsCount >= 3 || GetWorld()->GetTimeSeconds() - LastTimeSentClientTimeCorrection > 7)
	{
		LateInputsCount = 0;
		EarlyInputsCount = 0;
		LastTimeSentClientTimeCorrection = GetWorld()->GetTimeSeconds();

		int timediff = static_cast<int>(TotalTimeDifference / TotalTimes);

		if (timediff != 0)
		{
			SendClientTimeCorrection(timediff);

			GEngine->AddOnScreenDebugMessage(-1, 10.f, FColor::Green, FString::Printf(TEXT("%i"), static_cast<int>(TotalTimeDifference / TotalTimes)));
		}

		TotalTimeDifference = 0;
		TotalTimes = 0;
	}
}

void ATestCharacter::MakeCorrectionData()
{
	SendClientCorrection(TimeStamp, GetActorLocation(), Rotation, PreviousLocation, LastVelocity, MoveSpeed);
}

void ATestCharacter::EndAllMovementAbilities()
{
}

#pragma endregion

#pragma region ClientData

#pragma region RPCS

void ATestCharacter::Send3Input_Implementation(const FInputs input, const FInputs previnput, const FInputs previnput2)
{
	ReceivedClientInputOnce = true;

	if (input.TimeStamp > TimeStamp)
	{
		InputsMap.Emplace(input.TimeStamp, input);
	}

	if (input.TimeStamp != -1)
	{
		CheckClientTimeError(input.TimeStamp);
	}

	if (previnput.TimeStamp > TimeStamp)
	{
		InputsMap.Emplace(previnput.TimeStamp, previnput);
	}

	if (previnput2.TimeStamp > TimeStamp)
	{
		InputsMap.Emplace(previnput2.TimeStamp, previnput2);
	}
}

void ATestCharacter::Send2Input_Implementation(const FInputs input, const FInputs input2)
{
	ReceivedClientInputOnce = true;

	if (input.TimeStamp > TimeStamp)
	{
		InputsMap.Emplace(input.TimeStamp, input);
	}

	if (input.TimeStamp != -1)
	{
		CheckClientTimeError(input.TimeStamp);
	}

	if (input2.TimeStamp > TimeStamp)
	{
		InputsMap.Emplace(input2.TimeStamp, input2);
	}
}

void ATestCharacter::Send1Input_Implementation(const FInputs input)
{
	ReceivedClientInputOnce = true;

	if (input.TimeStamp >= TimeStamp)
	{
		InputsMap.Emplace(input.TimeStamp, input);
	}

	if (input.TimeStamp != -1)
	{
		CheckClientTimeError(input.TimeStamp);
	}
}

#pragma endregion

#pragma region HandlingData

void ATestCharacter::SendInputData()
{
	PrevInput2 = PrevInput;
	PrevInput = CurrentInput;

	if (PrevInput.TimeStamp != -1 && PrevInput2.TimeStamp != -1)
	{
		//Send2Input(PrevInput, PrevInput2);

		//FInputs EmptyInput;
		//PrevInput = EmptyInput;
		//PrevInput2 = EmptyInput;
	}

	Send1Input(CurrentInput);
}

void ATestCharacter::AssignInputs()
{
	W = W1;
	S = S1;
	A = A1;
	D = D1;
	SpaceBar = SpaceBar1;
	Mouse1 = Mouse11;
	Mouse2 = Mouse21;
	E = E1;
	Q = Q1;
}

void ATestCharacter::AfterAssignInputs(FInputs& input)
{
	input.W = W;
	input.S = S;
	input.A = A;
	input.D = D;
	input.SpaceBar = SpaceBar;
	input.Mouse1 = Mouse1;
	input.Mouse2 = Mouse2;
	input.E = E;
	input.Q = Q;
}

FInputs ATestCharacter::FindInput()
{
	FInputs i = InputsMap.FindRef(TimeStamp);

	return i;
}

FInputs ATestCharacter::SimulateFindInput()
{
	FInputs i = InputsMap.FindRef(TimeStamp);

	InputsMap.Remove(TimeStamp);

	return i;
}

void ATestCharacter::HandleInput(const FInputs input)
{
	Rotation = input.Rotation;

	if (input.W)
	{
		WTask();
	}
	if (input.S)
	{
		STask();
	}
	if (input.A)
	{
		ATask();
	}
	if (input.D)
	{
		DTask();
	}

	SpaceBarTrue = input.SpaceBar;
	Mouse1True = input.Mouse1;
	Mouse2True = input.Mouse2;
	ETrue = input.E;
	QTrue = input.Q;
}

#pragma endregion

#pragma region InputActions

void ATestCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//Replicated Inputs
		EnhancedInputComponent->BindAction(Mouse1Action, ETriggerEvent::Started, this, &ATestCharacter::Mouse1Pressed);
		EnhancedInputComponent->BindAction(Mouse1Action, ETriggerEvent::Completed, this, &ATestCharacter::Mouse1Released);
		EnhancedInputComponent->BindAction(Mouse2Action, ETriggerEvent::Started, this, &ATestCharacter::Mouse2Pressed);
		EnhancedInputComponent->BindAction(Mouse2Action, ETriggerEvent::Completed, this, &ATestCharacter::Mouse2Released);
		EnhancedInputComponent->BindAction(EAction, ETriggerEvent::Started, this, &ATestCharacter::EPressed);
		EnhancedInputComponent->BindAction(EAction, ETriggerEvent::Completed, this, &ATestCharacter::EReleased);
		EnhancedInputComponent->BindAction(QAction, ETriggerEvent::Started, this, &ATestCharacter::QPressed);
		EnhancedInputComponent->BindAction(QAction, ETriggerEvent::Completed, this, &ATestCharacter::QReleased);

		EnhancedInputComponent->BindAction(SpaceBarAction, ETriggerEvent::Started, this, &ATestCharacter::SpaceBarPressed);
		EnhancedInputComponent->BindAction(SpaceBarAction, ETriggerEvent::Completed, this, &ATestCharacter::SpaceBarReleased);

		EnhancedInputComponent->BindAction(SAction, ETriggerEvent::Started, this, &ATestCharacter::SPressed);
		EnhancedInputComponent->BindAction(SAction, ETriggerEvent::Completed, this, &ATestCharacter::SReleased);
		EnhancedInputComponent->BindAction(AAction, ETriggerEvent::Started, this, &ATestCharacter::APressed);
		EnhancedInputComponent->BindAction(AAction, ETriggerEvent::Completed, this, &ATestCharacter::AReleased);
		EnhancedInputComponent->BindAction(WAction, ETriggerEvent::Started, this, &ATestCharacter::WPressed);
		EnhancedInputComponent->BindAction(WAction, ETriggerEvent::Completed, this, &ATestCharacter::WReleased);
		EnhancedInputComponent->BindAction(DAction, ETriggerEvent::Started, this, &ATestCharacter::DPressed);
		EnhancedInputComponent->BindAction(DAction, ETriggerEvent::Completed, this, &ATestCharacter::DReleased);

		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATestCharacter::Look);

		//Client Only
		EnhancedInputComponent->BindAction(EscAction, ETriggerEvent::Started, this, &ATestCharacter::OpenOptionsMenu);
		EnhancedInputComponent->BindAction(EscAction, ETriggerEvent::Completed, this, &ATestCharacter::CloseOptionsMenu);
	}
}

void ATestCharacter::WPressed()
{
	W1 = true;
}
void ATestCharacter::WReleased()
{
	W1 = false;
}
void ATestCharacter::WTask()
{
	InputVelocity = InputVelocity + UKismetMathLibrary::GetForwardVector(ForwardRotation);
}

void ATestCharacter::SPressed()
{
	S1 = true;
}
void ATestCharacter::SReleased()
{
	S1 = false;
}
void ATestCharacter::STask()
{
	InputVelocity = InputVelocity + -UKismetMathLibrary::GetForwardVector(ForwardRotation);
}

void ATestCharacter::APressed()
{
	A1 = true;
}
void ATestCharacter::AReleased()
{
	A1 = false;
}
void ATestCharacter::ATask()
{
	InputVelocity = InputVelocity + -UKismetMathLibrary::GetRightVector(ForwardRotation);
}

void ATestCharacter::DPressed()
{
	D1 = true;
}
void ATestCharacter::DReleased()
{
	D1 = false;
}
void ATestCharacter::DTask()
{
	InputVelocity = InputVelocity + UKismetMathLibrary::GetRightVector(ForwardRotation);
}

void ATestCharacter::SpaceBarPressed()
{
	SpaceBar1 = true;
}
void ATestCharacter::SpaceBarReleased()
{
	SpaceBar1 = false;
}

void ATestCharacter::Mouse1Pressed()
{
	Mouse11 = true;
}
void ATestCharacter::Mouse1Released()
{
	Mouse11 = false;
}

void ATestCharacter::Mouse2Pressed()
{
	Mouse21 = true;
}
void ATestCharacter::Mouse2Released()
{
	Mouse21 = false;
}

void ATestCharacter::EPressed()
{
	E1 = true;
}
void ATestCharacter::EReleased()
{
	E1 = false;
}

void ATestCharacter::QPressed()
{
	Q1 = true;
}
void ATestCharacter::QReleased()
{
	Q1 = false;
}

void ATestCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(-LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ATestCharacter::OpenOptionsMenu()
{
	if (!OptionsMenu)
	{
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (!PC)
	{
		return;
	}

	UUserWidget* OptionsMenuWidget = CreateWidget<UUserWidget>(PC, OptionsMenu);
	OptionsMenuWidget->AddToViewport();

	DisableInput(PC);
	PC->bShowMouseCursor = true;
}

void ATestCharacter::CloseOptionsMenu()
{

}

#pragma endregion

#pragma endregion

void ATestCharacter::TickOnAllComponents(float DeltaTime)
{
	TestMovement->MovementTick(DeltaTime);
}

void ATestCharacter::OnRep_ReplicatedMovement()
{
	if (bExternalMovementSource || bPauseSimulation)
	{
		SetActorLocation(SimulatedProxyPosition);

		return;
	}

	bReceivedMovementData = true;
	SimulatedProxyTimeBetweenMovementUpdates = GetWorld()->GetTimeSeconds() - SimulatedProxyLastTimeReceivedMovementUpdate;
	SimulatedProxyLastTimeReceivedMovementUpdate = GetWorld()->GetTimeSeconds();

	bIsGrounded = TestMovement->IsGrounded();

	LastVelocity = (GetActorLocation() - PreviousLocation) / SimulatedProxyTimeBetweenMovementUpdates;
	PreviousLocation = GetActorLocation();

	Super::OnRep_ReplicatedMovement();
}

void ATestCharacter::ClearOldData()
{
	for (auto& Elem : InputsMap)
	{
		if (TimeStamp - Elem.Key >= ClearDataThreshold)
		{
			InputsMap.Remove(Elem.Key);
		}
	}
	for (auto& Elem : ClientDataMap)
	{
		if (TimeStamp - Elem.Key >= ClearDataThreshold)
		{
			ClientDataMap.Remove(Elem.Key);
		}
	}
	for (auto& Elem : SavedDataMap)
	{
		if (TimeStamp - Elem.Key >= ClearDataThreshold)
		{
			SavedDataMap.Remove(Elem.Key);
		}
	}
	for (auto& Elem : ServerAuthoritativeDataMap)
	{
		if (TimeStamp - Elem.Key >= ClearDataThreshold)
		{
			ServerAuthoritativeDataMap.Remove(Elem.Key);
		}
	}
	for (auto& Elem : HitsMap)
	{
		if (TimeStamp - Elem.Key >= ClearDataThreshold)
		{
			HitsMap.Remove(Elem.Key);
		}
	}
}

FCollisionQueryParams ATestCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams Params;

	TArray<AActor*> CharacterChildren;
	GetAllChildActors(CharacterChildren);
	Params.AddIgnoredActors(CharacterChildren);
	Params.AddIgnoredActor(this);

	return Params;
}

float ATestCharacter::GetCapsuleHeight()
{
	return 0;
}

float ATestCharacter::GetCapsuleRadius()
{
	return 0;
}

#pragma region unused

/*
	input.W = W;
	PrevW = W;
	WChanged = false;

	input.S = S;
	PrevS = S;
	SChanged = false;

	input.A = A;
	PrevA = A;
	AChanged = false;

	input.D = D;
	PrevD = D;
	DChanged = false;

	input.SpaceBar = SpaceBar;
	PrevSpaceBar = SpaceBar;
	SpaceBarChanged = false;

	input.Mouse1 = Mouse1;
	PrevMouse1 = Mouse1;
	Mouse1Changed = false;

	input.Mouse2 = Mouse2;
	PrevMouse2 = Mouse2;
	Mouse2Changed = false;
	*/

/*
	if (W1 != PrevW && !WChanged)
	{
		W = W1;
		WChanged = true;
		bAnyInputChanged = true;
	}
	if (S1 != PrevS && !SChanged)
	{
		S = S1;
		SChanged = true;
		bAnyInputChanged = true;
	}
	if (A1 != PrevA && !AChanged)
	{
		A = A1;
		AChanged = true;
		bAnyInputChanged = true;
	}
	if (D1 != PrevD && !DChanged)
	{
		D = D1;
		DChanged = true;
		bAnyInputChanged = true;
	}
	if (SpaceBar1 != PrevSpaceBar && !SpaceBarChanged)
	{
		SpaceBar = SpaceBar1;
		SpaceBarChanged = true;
		bAnyInputChanged = true;
	}
	if (Mouse11 != PrevMouse1 && !Mouse1Changed)
	{
		Mouse1 = Mouse11;
		Mouse1Changed = true;
		bAnyInputChanged = true;
	}
	if (Mouse21 != PrevMouse2 && !Mouse2Changed)
	{
		Mouse2 = Mouse21;
		Mouse2Changed = true;
		bAnyInputChanged = true;
	}
	*/

/*
	ClientPrevInput2 = ClientPrevInput;
	ClientPrevInput = CurrentInput;

	bool SInputValid = ServerInput.TimeStamp != -1;
	bool PInputValid = PrevInput.TimeStamp != -1;
	bool P2InputValid = PrevInput2.TimeStamp != -1;

	if (SInputValid)
	{
		LastTimeSentInputs = GetWorld()->GetTimeSeconds();

		if (PInputValid && P2InputValid)
		{
			Send3Input(ServerInput, PrevInput, PrevInput2);
		}

		else if (PInputValid)
		{
			Send2Input(ServerInput, PrevInput, false);
		}

		else if (P2InputValid)
		{
			Send2Input(ServerInput, PrevInput2, true);
		}

		else
		{
			Send1Input(ServerInput);
		}
	}

	else if (PInputValid)

	if(PInputValid)
	{
		if (P2InputValid)
		{
			//Send2Input(PrevInput, PrevInput2, true);
		}

		else
		{
			//Send1Input(PrevInput);
		}
	}

	else if (P2InputValid)
	{
		//Send1Input(PrevInput2);
	}

	//if (GetWorld()->GetTimeSeconds() - LastTimeSentInputs > 1)
	{
		//LastTimeSentInputs = GetWorld()->GetTimeSeconds();

		//Send1LeastRecentInput(CurrentInput);

		Send3Input(CurrentInput, ClientPrevInput, ClientPrevInput2);
	}

	*/
bool ATestCharacter::InputDiffers(const FInputs input)
{
	if (
		!input.Rotation.Equals(LastServerInput.Rotation, 0.001) ||
		input.W != LastServerInput.W ||
		input.S != LastServerInput.S ||
		input.D != LastServerInput.D ||
		input.A != LastServerInput.A ||
		input.SpaceBar != LastServerInput.SpaceBar ||
		input.Mouse1 != LastServerInput.Mouse1 ||
		input.Mouse2 != LastServerInput.Mouse2 ||
		input.E != LastServerInput.E ||
		input.Q != LastServerInput.Q
		)
	{
		return true;
	}

	return false;
}
void ATestCharacter::Reset()
{
	W1 = false;
	S1 = false;
	A1 = false;
	D1 = false;
	SpaceBar1 = false;
	Mouse11 = false;
	Mouse21 = false;
}

#pragma endregion