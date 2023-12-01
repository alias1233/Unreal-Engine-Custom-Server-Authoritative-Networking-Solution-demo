// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PawnMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "TestCharacter.h"
#include "TestMovement.generated.h"

/**
 * 
 */
UCLASS()
class DOOMFIST_API UTestMovement : public UPawnMovementComponent
{
	GENERATED_BODY()

public:
	virtual void InitializeComponent() override;

	void MovementTick(float DeltaTime);
	void SimulatedMovementTick(float DeltaTime, bool updatedthistick);

	bool IsGrounded();

	UPROPERTY(Transient)
	ATestCharacter* Owner;
	
protected:
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
};
