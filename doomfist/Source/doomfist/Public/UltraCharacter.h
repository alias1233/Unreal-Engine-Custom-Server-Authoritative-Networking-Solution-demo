// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TestCharacter.h"
#include "InputActionValue.h"
#include "UObject/ObjectMacros.h"
#include "UObject/UObjectGlobals.h"
#include "Templates/SubclassOf.h"
#include "UObject/CoreNet.h"
#include "Engine/NetSerialization.h"
#include "Engine/EngineTypes.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Pawn.h"
#include "TestCharacter.h"
#include "GameFramework/CharacterMovementReplication.h"
#include "Animation/AnimationAsset.h"
#include "GameFramework/RootMotionSource.h"
#include "UltraCharacter.generated.h"

/**
 * 
 */

USTRUCT() struct FUltraCorrectionData
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
	bool NoFriction;
	UPROPERTY()
	float MoveSpeed;

	UPROPERTY()
	int Mouse2StartTime;
	UPROPERTY()
	bool Mouse2AbilityIsPlaying;
	UPROPERTY()
	int Mouse2StartChargeTime;
	UPROPERTY()
	int Mouse2StartMovementTime;
	UPROPERTY()
	bool Mouse2MovementIsPlaying;
	UPROPERTY()
	int Mouse2Charge;
	UPROPERTY()
	FVector Mouse2StartForwardVector;
	UPROPERTY()
	bool Mouse2MovementStart;

	UPROPERTY()
	int EStartTime;
	UPROPERTY()
	bool EAbilityIsPlaying;
	UPROPERTY()
	FVector EStartForwardVector;

	UPROPERTY()
	int QStartTime;
	UPROPERTY()
	bool QAbilityIsPlaying;
	UPROPERTY()
	int QStartMovementTime;
	UPROPERTY()
	bool QMovementIsPlaying;
	UPROPERTY()
	FVector QStartForwardVector;

	FUltraCorrectionData()
	{
		TimeStamp = -1;
	}

	inline bool operator==(const FUltraCorrectionData& other) const
	{
		return TimeStamp == other.TimeStamp;
	}
};

/*
USTRUCT() struct FUltraHits
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

	FUltraHits()
	{
		TimeStamp = -1;
	}

	inline bool operator==(const FUltraCorrectionData& other) const
	{
		return TimeStamp == other.TimeStamp;
	}
};
*/

USTRUCT() struct FUltraSavedData
{
	GENERATED_BODY()

	UPROPERTY()
	int TimeStamp;

	UPROPERTY()
	int Mouse2StartTime;
	UPROPERTY()
	bool Mouse2AbilityIsPlaying;
	UPROPERTY()
	int Mouse2StartChargeTime;
	UPROPERTY()
	int Mouse2StartMovementTime;
	UPROPERTY()
	bool Mouse2MovementIsPlaying;
	UPROPERTY()
	int Mouse2Charge;
	UPROPERTY()
	FVector Mouse2StartForwardVector;
	UPROPERTY()
	bool Mouse2MovementStart;

	UPROPERTY()
	int EStartTime;
	UPROPERTY()
	bool EAbilityIsPlaying;
	UPROPERTY()
	FVector EStartForwardVector;

	UPROPERTY()
	int QStartTime;
	UPROPERTY()
	bool QAbilityIsPlaying;
	UPROPERTY()
	int QStartMovementTime;
	UPROPERTY()
	bool QMovementIsPlaying;
	UPROPERTY()
	FVector QStartForwardVector;

	FUltraSavedData()
	{
		TimeStamp = -1;
	}

	inline bool operator==(const FUltraSavedData& other) const
	{
		return TimeStamp == other.TimeStamp;
	}
};

UCLASS()
class DOOMFIST_API AUltraCharacter : public ATestCharacter
{
	GENERATED_BODY()
	
public:
	AUltraCharacter();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Moveset) class UUltraAbilityComponent* AbilityComponent;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = CollisionBox, meta = (AllowPrivateAccess = "true"))
	class UCapsuleComponent* UltraCapsule;

protected:
	virtual void TickOnAllComponents(float DeltaTime) override;

private:
	UFUNCTION(Client, unreliable)
	void SendUltraClientCorrection(const FUltraCorrectionData correctiondata);

	virtual void MakeCorrectionData() override;
	virtual void EndAllMovementAbilities() override;

public:
	UPROPERTY(BlueprintReadWrite)
	int Mouse1StartTime;

	UPROPERTY(BlueprintReadWrite)
	int Mouse2StartTime;
	UPROPERTY(BlueprintReadWrite)
	int Mouse2StartChargeTime;
	UPROPERTY(BlueprintReadWrite)
	int Mouse2StartMovementTime;
	UPROPERTY(BlueprintReadWrite)
	bool Mouse2AbilityIsPlaying;
	UPROPERTY(BlueprintReadWrite)
	bool Mouse2MovementIsPlaying;
	UPROPERTY(BlueprintReadWrite)
	bool Mouse2MovementStart;
	UPROPERTY(BlueprintReadWrite)
	int Mouse2Charge;
	UPROPERTY(BlueprintReadWrite)
	FVector Mouse2StartForwardVector;

	UPROPERTY(BlueprintReadWrite)
	int EStartTime;
	UPROPERTY(BlueprintReadWrite)
	bool EAbilityIsPlaying;
	UPROPERTY(BlueprintReadWrite)
	FVector EStartForwardVector;

	UPROPERTY(BlueprintReadWrite)
	int QStartTime;
	UPROPERTY(BlueprintReadWrite)
	bool QAbilityIsPlaying;
	UPROPERTY(BlueprintReadWrite)
	int QStartMovementTime;
	UPROPERTY(BlueprintReadWrite)
	bool QMovementIsPlaying;
	UPROPERTY(BlueprintReadWrite)
	bool QMovementStart;
	UPROPERTY(BlueprintReadWrite)
	FVector QStartForwardVector;

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int Mouse1Cooldown;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int Mouse2Cooldown;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float Mouse2Velocity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int Mouse2Duration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int Mouse2MaxCharge;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int Mouse2HitboxRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int ECooldown;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float EVelocity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int EDuration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int EHitboxRadius;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int QCooldown;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) float QVelocity;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int QDuration;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite) int QHitboxRadius;

	UFUNCTION(BlueprintImplementableEvent)
	void BPMouse2AbilityStart();
	UFUNCTION(BlueprintImplementableEvent)
	void BPMouse2AbilityTick(const bool mouse2);
	UFUNCTION(BlueprintImplementableEvent)
	void BPMouse2AbilityEnd();

	UFUNCTION(BlueprintImplementableEvent)
	void BPEAbilityStart();
	UFUNCTION(BlueprintImplementableEvent)
	void BPEAbilityTick(const bool eIsDown);
	UFUNCTION(BlueprintImplementableEvent)
	void BPEAbilityEnd();

	UFUNCTION(BlueprintImplementableEvent)
	void BPQAbilityStart();
	UFUNCTION(BlueprintImplementableEvent)
	void BPQAbilityTick(const bool qIsDown);
	UFUNCTION(BlueprintImplementableEvent)
	void BPQAbilityEnd();

	/*
	UFUNCTION(BlueprintCallable)
	void HitDetected(ATestCharacter* hitplayer, const bool mouse2, const bool mouse1, const bool E, const bool Q);
	UFUNCTION(BlueprintCallable)
	bool CheckHits(ATestCharacter*& hitplayer, const bool mouse1, const bool mouse2, const bool E, const bool Q);
	UFUNCTION(Server, unreliable)
	void SendHits(const FUltraHits hits);

	TMap<int, FUltraHits> UltraHitsMap;
	*/

	virtual void ClearOldData() override;

	virtual void SaveCurrentData() override;
	virtual void ResetToTimeStamp(int ResetTimeStamp) override;
	void UltraResetToData(const FUltraSavedData ultradata);

	TMap<int, FUltraSavedData> UltraSavedDataMap;

public:
	virtual float GetCapsuleHeight() override;
	virtual float GetCapsuleRadius() override;

	FORCEINLINE class UCapsuleComponent* GetCapsuleComponent() const { return UltraCapsule; }
};
