// Fill out your copyright notice in the Description page of Project Settings.

#include "UltraCharacter.h"
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
#include "Components/ArrowComponent.h"
#include "Engine/CollisionProfile.h"
#include "Net/Core/PropertyConditions/PropertyConditions.h"
#include "Net/UnrealNetwork.h"
#include "DisplayDebugHelpers.h"
#include "Engine/Canvas.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "TestMovement.h"
#include "UltraAbilityComponent.h"

AUltraCharacter::AUltraCharacter()
{
	UltraCapsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	UltraCapsule->InitCapsuleSize(42.f, 96.0f);

	TestMovement->UpdatedComponent = UltraCapsule;

	AbilityComponent = CreateDefaultSubobject<UUltraAbilityComponent>(TEXT("AbilityComponent"));
	AbilityComponent->UpdatedComponent = GetCapsuleComponent();

	Mouse2StartTime = -99;
	EStartTime = -99;
	QStartTime = -99;
}

void AUltraCharacter::SaveCurrentData()
{
	Super::SaveCurrentData();

	FUltraSavedData Data;

	Data.TimeStamp = TimeStamp;

	Data.Mouse2AbilityIsPlaying = Mouse2AbilityIsPlaying;
	Data.Mouse2Charge = Mouse2Charge;
	Data.Mouse2MovementIsPlaying = Mouse2MovementIsPlaying;
	Data.Mouse2MovementStart = Mouse2MovementStart;
	Data.Mouse2StartChargeTime = Mouse2StartChargeTime;
	Data.Mouse2StartForwardVector = Mouse2StartForwardVector;
	Data.Mouse2StartMovementTime = Mouse2StartMovementTime;
	Data.Mouse2StartTime = Mouse2StartTime;

	Data.EAbilityIsPlaying = EAbilityIsPlaying;
	Data.EStartForwardVector = EStartForwardVector;
	Data.EStartTime = EStartTime;

	Data.QAbilityIsPlaying = QAbilityIsPlaying;
	Data.QMovementIsPlaying = QMovementIsPlaying;
	Data.QStartForwardVector = QStartForwardVector;
	Data.QStartMovementTime = QStartMovementTime;
	Data.QStartTime = QStartTime;

	UltraSavedDataMap.Emplace(Data.TimeStamp, Data);
}

void AUltraCharacter::UltraResetToData(const FUltraSavedData ultradata)
{
	Mouse2AbilityIsPlaying = ultradata.Mouse2AbilityIsPlaying;
	Mouse2Charge = ultradata.Mouse2Charge;
	Mouse2MovementIsPlaying = ultradata.Mouse2MovementIsPlaying;
	Mouse2MovementStart = ultradata.Mouse2MovementStart;
	Mouse2StartChargeTime = ultradata.Mouse2StartChargeTime;
	Mouse2StartForwardVector = ultradata.Mouse2StartForwardVector;
	Mouse2StartMovementTime = ultradata.Mouse2StartMovementTime;
	Mouse2StartTime = ultradata.Mouse2StartTime;

	EAbilityIsPlaying = ultradata.EAbilityIsPlaying;
	EStartForwardVector = ultradata.EStartForwardVector;
	EStartTime = ultradata.EStartTime;

	QAbilityIsPlaying = ultradata.QAbilityIsPlaying;
	QMovementIsPlaying = ultradata.QMovementIsPlaying;
	QStartForwardVector = ultradata.QStartForwardVector;
	QStartMovementTime = ultradata.QStartMovementTime;
	QStartTime = ultradata.QStartTime;
}

void AUltraCharacter::ResetToTimeStamp(int ResetTimeStamp)
{
	Super::ResetToTimeStamp(ResetTimeStamp);

	FUltraSavedData PendingData = UltraSavedDataMap.FindRef(ResetTimeStamp);

	if (PendingData.TimeStamp == -1)
	{
		return;
	}

	UltraResetToData(PendingData);
}

/*
void AUltraCharacter::HitDetected(ATestCharacter* hitplayer, const bool mouse2, const bool mouse1, const bool EHit, const bool QHit)
{
	FUltraHits hits;
	hits.TimeStamp = TimeStamp;
	hits.HitPlayer = hitplayer;
	hits.Mouse2Hit = mouse2;
	hits.EHit = EHit;
	hits.QHit = QHit;

	UltraHitsMap.Emplace(hits.TimeStamp, hits);

	if (GetLocalRole() == ROLE_AutonomousProxy)
	{
		SendHits(hits);
	}
}

bool AUltraCharacter::CheckHits(ATestCharacter*& hitplayer, const bool mouse1, const bool mouse2, const bool EHit, const bool QHit)
{
	FUltraHits hits = UltraHitsMap.FindRef(TimeStamp);
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

	UltraHitsMap.Remove(TimeStamp);

	return hitdetected;
}

void AUltraCharacter::SendHits_Implementation(const FUltraHits hits)
{
	if (hits.TimeStamp > TimeStamp)
	{
		UltraHitsMap.Emplace(hits.TimeStamp, hits);
	}
}
*/

void AUltraCharacter::MakeCorrectionData()
{
	//if (Mouse2AbilityIsPlaying)
	{
		FUltraCorrectionData Data;

		Data.TimeStamp = TimeStamp;

		Data.Position = GetActorLocation();
		Data.Rotation = Rotation;
		Data.PreviousPosition = PreviousLocation;
		Data.LastVelocity = LastVelocity;
		Data.NoMovement = NoMovement;
		Data.NoFriction = bNoFriction;
		Data.MoveSpeed = MoveSpeed;

		Data.Mouse2AbilityIsPlaying = Mouse2AbilityIsPlaying;
		Data.Mouse2Charge = Mouse2Charge;
		Data.Mouse2MovementIsPlaying = Mouse2MovementIsPlaying;
		Data.Mouse2MovementStart = Mouse2MovementStart;
		Data.Mouse2StartChargeTime = Mouse2StartChargeTime;
		Data.Mouse2StartForwardVector = Mouse2StartForwardVector;
		Data.Mouse2StartMovementTime = Mouse2StartMovementTime;
		Data.Mouse2StartTime = Mouse2StartTime;

		Data.EAbilityIsPlaying = EAbilityIsPlaying;
		Data.EStartForwardVector = EStartForwardVector;
		Data.EStartTime = EStartTime;

		Data.QAbilityIsPlaying = QAbilityIsPlaying;
		Data.QMovementIsPlaying = QMovementIsPlaying;
		Data.QStartForwardVector = QStartForwardVector;
		Data.QStartMovementTime = QStartMovementTime;
		Data.QStartTime = QStartTime;

		SendUltraClientCorrection(Data);

		return;
	}

	//Super::MakeCorrectionData();
}

void AUltraCharacter::SendUltraClientCorrection_Implementation(const FUltraCorrectionData correctiondata)
{
	AfterCorrectionReceived(correctiondata.TimeStamp);

	SetActorLocation(correctiondata.Position);
	Rotation = correctiondata.Rotation;
	PreviousLocation = correctiondata.PreviousPosition;
	LastVelocity = correctiondata.LastVelocity;
	NoMovement = correctiondata.NoMovement;
	bNoFriction = correctiondata.NoFriction;
	MoveSpeed = correctiondata.MoveSpeed;

	Mouse2AbilityIsPlaying = correctiondata.Mouse2AbilityIsPlaying;
	Mouse2Charge = correctiondata.Mouse2Charge;
	Mouse2MovementIsPlaying = correctiondata.Mouse2MovementIsPlaying;
	Mouse2MovementStart = correctiondata.Mouse2MovementStart;
	Mouse2StartChargeTime = correctiondata.Mouse2StartChargeTime;
	Mouse2StartForwardVector = correctiondata.Mouse2StartForwardVector;
	Mouse2StartMovementTime = correctiondata.Mouse2StartMovementTime;
	Mouse2StartTime = correctiondata.Mouse2StartTime;

	EAbilityIsPlaying = correctiondata.EAbilityIsPlaying;
	EStartForwardVector = correctiondata.EStartForwardVector;
	EStartTime = correctiondata.EStartTime;

	QAbilityIsPlaying = correctiondata.QAbilityIsPlaying;
	QMovementIsPlaying = correctiondata.QMovementIsPlaying;
	QStartForwardVector = correctiondata.QStartForwardVector;
	QStartMovementTime = correctiondata.QStartMovementTime;
	QStartTime = correctiondata.QStartTime;
}

void AUltraCharacter::ClearOldData()
{
	Super::ClearOldData();

	for (auto& Elem : UltraSavedDataMap)
	{
		if (TimeStamp - Elem.Key >= ClearDataThreshold)
		{
			UltraSavedDataMap.Remove(Elem.Key);
		}
	}
}

void AUltraCharacter::EndAllMovementAbilities()
{
	//AbilityComponent->Mouse2AbilityEnd();
}

void AUltraCharacter::TickOnAllComponents(float DeltaTime)
{
	Super::TickOnAllComponents(DeltaTime);

	AbilityComponent->AbilityTick(DeltaTime, bIsSimulating);
}

float AUltraCharacter::GetCapsuleHeight()
{
	return GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

float AUltraCharacter::GetCapsuleRadius()
{
	return GetCapsuleComponent()->GetScaledCapsuleRadius();
}