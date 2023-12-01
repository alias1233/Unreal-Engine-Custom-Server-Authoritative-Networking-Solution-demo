// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "UltraCharacter.h"
#include "UltraAbilityComponent.generated.h"

/**
 * 
 */
UCLASS()
class DOOMFIST_API UUltraAbilityComponent : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	virtual void InitializeComponent() override;

	void AbilityTick(float DeltaTime, bool IsSimulating);

	UPROPERTY(Transient)
	AUltraCharacter* Owner;

	void Mouse2AbilityEnd();

	UPROPERTY(BlueprintReadOnly)
	float AbilityDeltaTime;

protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void Mouse2AbilityTick(float DeltaTime);
	void Mouse2MovementTick(float DeltaTime);
	void Mouse2HitboxTick();

	void Mouse2AbilityStart();
	void Mouse2MovementStart();

protected:
	UFUNCTION(BlueprintCallable)
	void MovePlayer(
		const float magnitude,
		const FVector direction
	);

	bool SphereTrace(
		const FVector& Start,
		const float Radius,
		FHitResult& HitOut,
		ECollisionChannel TraceChannel = ECC_Pawn
	);
};
