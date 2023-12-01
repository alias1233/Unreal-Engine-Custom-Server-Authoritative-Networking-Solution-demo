// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Templates/SubclassOf.h"
#include "UObject/CoreNet.h"
#include "Engine/NetSerialization.h"
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementReplication.h"
#include "Animation/AnimationAsset.h"
#include "GameFramework/RootMotionSource.h"
#include "TestCharacter.generated.h"

USTRUCT() struct FInputs
{
	GENERATED_BODY()

	UPROPERTY()
	int TimeStamp;

	UPROPERTY()
	FRotator Rotation;

	UPROPERTY()
	bool W;
	UPROPERTY()
	bool S;
	UPROPERTY()
	bool A;
	UPROPERTY()
	bool D;

	UPROPERTY()
	bool SpaceBar;

	UPROPERTY()
	bool Mouse1;
	UPROPERTY()
	bool Mouse2;
	UPROPERTY()
	bool E;
	UPROPERTY()
	bool Q;

	FInputs()
	{
		TimeStamp = -1;
	}

	inline bool operator==(const FInputs& other) const
	{
		return TimeStamp == other.TimeStamp;
	}
};

USTRUCT() struct FClientData
{
	GENERATED_BODY()

	UPROPERTY()
	int TimeStamp;
	UPROPERTY()
	FVector Position;
	UPROPERTY()
	FVector PreviousPosition;
	UPROPERTY()
	FVector LastVelocity;

	FClientData()
	{
		TimeStamp = -1;
	}

	inline bool operator==(const FClientData& other) const
	{
		return TimeStamp == other.TimeStamp;
	}
};

USTRUCT() struct FSavedData
{
	GENERATED_BODY()

	UPROPERTY()
	int TimeStamp;

	UPROPERTY()
	FVector Position;
	UPROPERTY()
	FRotator Rotation;
	UPROPERTY()
	FVector PreviousPosition;
	UPROPERTY()
	FVector LastVelocity;
	UPROPERTY()
	bool NoMovement;
	UPROPERTY()
	float MoveSpeed;

	FSavedData()
	{
		TimeStamp = -1;
	}
};

USTRUCT() struct FHits
{
	GENERATED_BODY()

	UPROPERTY()
	int TimeStamp;

	UPROPERTY()
	ATestCharacter* HitPlayer;

	UPROPERTY()
	bool Mouse2Hit;
	UPROPERTY()
	bool EHit;
	UPROPERTY()
	bool QHit;

	FHits()
	{
		TimeStamp = -1;
	}

	inline bool operator==(const FHits& other) const
	{
		return TimeStamp == other.TimeStamp;
	}
};

USTRUCT() struct FServerAuthoritativeData
{
	GENERATED_BODY()

	UPROPERTY()
	int TimeStamp;

	UPROPERTY()
	int Duration;
	UPROPERTY()
	FVector LaunchVelocity;
	UPROPERTY()
	bool NoFriction;

	FServerAuthoritativeData()
	{
		TimeStamp = -1;
	}

	inline bool operator==(const FServerAuthoritativeData& other) const
	{
		return TimeStamp == other.TimeStamp;
	}
};

UCLASS()
class DOOMFIST_API ATestCharacter : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATestCharacter();

	static FName CharacterMovementComponentName;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Moveset) 
	class UTestMovement* TestMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Moveset) 
	class UStatsComponent* StatsComponent;

	UPROPERTY(BlueprintReadOnly, Transient, DuplicateTransient, Category = MovementComponent)
	TObjectPtr<USceneComponent> UpdatedComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputMappingContext* DefaultMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* WAction;

	void WPressed();
	void WReleased();
	void WTask();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SAction;

	void SPressed();
	void SReleased();
	void STask();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* AAction;

	void APressed();
	void AReleased ();
	void ATask();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* DAction;

	void DPressed();
	void DReleased();
	void DTask();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* SpaceBarAction;

	void SpaceBarPressed();
	void SpaceBarReleased();
	void SpaceBarTask();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Mouse1Action;

	void Mouse1Pressed();
	void Mouse1Released();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* Mouse2Action;

	void Mouse2Pressed();
	void Mouse2Released();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* EAction;

	void EPressed();
	void EReleased();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* QAction;

	void QPressed();
	void QReleased();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* LookAction;

	void Look(const FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	class UInputAction* EscAction;

	void OpenOptionsMenu();
	void CloseOptionsMenu();

public:
	//Blueprint Set Variables
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) 
	float BaseMoveSpeed;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) 
	float JumpForce;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) 
	float Gravity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) 
	float GroundFriction;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true")) 
	float AirFriction;

	UPROPERTY(EditDefaultsOnly) 
	int CorrectionDistance;
	UPROPERTY(EditDefaultsOnly) 
	int CorrectionVelocity;
	UPROPERTY(EditDefaultsOnly) 
	int LeniantCorrectionDistance;
	UPROPERTY(EditDefaultsOnly) 
	int LeniantCorrectionVelocity;
	UPROPERTY(EditDefaultsOnly)
	float MinTimeBetweenCorrections;

	UPROPERTY(EditDefaultsOnly)
	float MinTimeBetweenDataSend;

	UPROPERTY(EditDefaultsOnly)
	int ExternalMovementSourceDuration;
	UPROPERTY(EditDefaultsOnly)
	float CheckMovableRange;

	UPROPERTY(EditDefaultsOnly)
	int ClearDataThreshold;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int TickTime;
	UPROPERTY(EditDefaultsOnly) 
	int ClientDataInterval;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int FixedTickPerSecond;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int ServerDelay;
	UPROPERTY(EditDefaultsOnly) 
	int MaxFixedTicksPerFrame;
	UPROPERTY(EditDefaultsOnly)
	int ServerMaxFixedTicksPerFrame;

	//Blueprint Set Assets
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = Widgets, meta = (AllowPrivateAccess = "true")) 
	TSubclassOf<UUserWidget> OptionsMenu;

protected:
	//Input
	int NoInputTimeStamp;
	float NoInputStartTime;

	bool bSendAllNotAckedInputs;
	float SendAllNotAckedInputStartTime;

	void AssignInputs();
	void AfterAssignInputs(FInputs& input);
	FInputs FindInput();
	FInputs SimulateFindInput();
	void HandleInput(const FInputs input);

	virtual void TickOnAllComponents(float DeltaTime);

	void ReplayMovesAfterCorrection();

	UFUNCTION(Server, unreliable)
	void Send3Input(const FInputs input, const FInputs previnput, const FInputs previnput2);
	UFUNCTION(Server, unreliable)
	void Send2Input(const FInputs input, const FInputs input2);
	UFUNCTION(Server, unreliable)
	void Send1Input(const FInputs input);

	void SendAllPendingInputs();

	UFUNCTION(Server, unreliable)
	void SendArrayInput(const TArray<FInputs>& pendinginputs);

	void RewindAndReplayMoves(int StartSimulationTimeStamp);

	virtual void ResetToTimeStamp(int ResetTimeStamp);

	virtual void SaveCurrentData();
	void ResetToData(const FSavedData data);

	//ClientCorrections
	bool bIgnoreClientError;

	void CheckClientError();
	void CheckClientTimeError(int clienttime);

	UFUNCTION(Server, unreliable)
	void SendClientData(const FClientData clientdata);
	UFUNCTION(Client, unreliable)
	void SendClientCorrection(int timestamp1, const FVector position, const FRotator rotation, const FVector previouslocation, const FVector lastvelocity, const float movespeed);
	UFUNCTION(Client, unreliable)
	void AckMove(int timestamp1);
	UFUNCTION(Server, unreliable)
	void ReceivedCorrection(int timeCorrectionReceived);

	UFUNCTION(Client, unreliable)
	void SendClientTimeCorrection(int timetoadd);

	UFUNCTION(Client, reliable)
	void ServerAuthoritativeMove(int timestamp1, const FVector position, const FVector lastvelocity, const FVector previouslocation, const bool nofriction);

	UFUNCTION(Client, unreliable)
	void NotifyClientOfLoss();
	UFUNCTION(Client, unreliable)
	void NotifyClientLossHandled();

	UFUNCTION(BlueprintCallable)
	void HitDetected(ATestCharacter* hitplayer, const bool mouse2, const bool mouse1, const bool E, const bool Q);
	UFUNCTION(BlueprintCallable)
	bool CheckHits(ATestCharacter*& hitplayer, const bool mouse1, const bool mouse2, const bool E, const bool Q);
	UFUNCTION(Server, unreliable)
	void SendHits(const FHits hits);

	bool SendPendingHit;
	FHits PendingHit;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	//Called every fixed frame
	virtual void FixedTick(float DeltaTime);

	virtual void ServerTick(float DeltaTime);
	virtual void AutonomousProxyTick(float DeltaTime);
	virtual void SimulatedProxyTick(float DeltaTime);

	virtual void PendingActions();

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	int TimeStamp;

	int SimulateTimeStamp;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bIsSimulating;

	virtual void MakeCorrectionData();
	virtual void EndAllMovementAbilities();
	virtual void AfterCorrectionReceived();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Reset();

	UFUNCTION(BlueprintCallable)
	void Launch(const FVector launchvelocity, const float pingInMilliSeconds, const bool nofriction);
	UFUNCTION(BlueprintCallable)
	void SetPlayerToPreviousPosition(ATestCharacter* hitplayer);

	virtual void OnRep_ReplicatedMovement() override;

	FVector SimulatedProxyPosition;
	bool bReceivedMovementData;
	float SimulatedProxyLastTimeReceivedMovementUpdate;
	float SimulatedProxyTimeBetweenMovementUpdates;

	FORCEINLINE class UTestMovement* GetTestMovement() const { return TestMovement; }

	FORCEINLINE class UStatsComponent* GetStatsComponentMovement() const { return StatsComponent; }

	FCollisionQueryParams GetIgnoreCharacterParams() const;

	virtual float GetCapsuleHeight();
	virtual float GetCapsuleRadius();

private:
	void SendInputData();
	bool InputDiffers(const FInputs input);

	void HandleClientData();

	int InputCount;
	int ClientDataTracker;

	float FixedTickRate;
	float FixedTickDelta;

	float LastTimeSentServerResponse;
	float LastCorrectionSentTime;
	int MostRecentAckedMoveTimeStamp;
	bool bClientReceivedCorrection;
	int ClientReceivedCorrectionTime;

	bool bLenientClientErrorDetection;
	FVector PreviousClientLocation;
	FVector PreviousServerLocation;

	void HandleExternalMove();

	bool bExternalMovementSource;
	int ExternalMovementSourceStartTime;
	bool SendExternalMovement;
	
	int EarlyInputsCount;
	int TotalEarlyTime;
	int LateInputsCount;
	float TotalLateTime;
	float LastTimeSentClientTimeCorrection;
	int TotalTimes;
	float TotalTimeDifference;
	int TimeDiff;

public:
	TMap<int, FInputs> InputsMap;
	TMap<int, FClientData> ClientDataMap;
	TMap<int, FSavedData> SavedDataMap;
	TMap<int, FServerAuthoritativeData> ServerAuthoritativeDataMap;
	TMap<int, FHits> HitsMap;

	virtual void ClearOldData();

	FVector InputVelocity;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	float MoveSpeed;

	FInputs CurrentInput;
	FInputs ServerInput;
	FInputs LastServerInput;
	FInputs PrevInput;
	FInputs PrevInput2;
	FInputs ClientInput;
	FInputs ClientPrevInput;
	FInputs ClientPrevInput2;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool NoMovement;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FVector PreviousLocation;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FVector GravityVelocity;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FVector LastVelocity;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FRotator Rotation;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	FRotator ForwardRotation;

	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool bNoFriction;

	UPROPERTY(BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	bool bIsGrounded;

	bool SendCorrectionData;

	bool ReplayMoves;

	int PauseSimulationStartTime;
	bool bPauseSimulation;
	float SimulatedProxyPing;

	bool ReceivedClientInputOnce;

	bool bAnyInputChanged;
	float LastTimeSentInputs;

	bool W;
	bool W1;
	bool PrevW;
	bool WChanged;
	bool WTrue;

	bool S;
	bool S1;
	bool PrevS;
	bool SChanged;
	bool STrue;

	bool A;
	bool A1;
	bool PrevA;
	bool AChanged;
	bool ATrue;

	bool D;
	bool D1;
	bool PrevD;
	bool DChanged;
	bool DTrue;

	bool SpaceBar;
	bool SpaceBar1;
	bool PrevSpaceBar;
	bool SpaceBarChanged;
	bool SpaceBarTrue;

	bool Mouse1;
	bool Mouse11;
	bool PrevMouse1;
	bool Mouse1Changed;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool Mouse1True;

	bool Mouse2;
	bool Mouse21;
	bool PrevMouse2;
	bool Mouse2Changed;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool Mouse2True;

	bool E;
	bool E1;
	bool PrevE;
	bool EChanged;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool ETrue;

	bool Q;
	bool Q1;
	bool PrevQ;
	bool QChanged;
	UPROPERTY(BlueprintReadWrite, meta = (AllowPrivateAccess = "true"))
	bool QTrue;
};
