// Fill out your copyright notice in the Description page of Project Settings.


#include "UltraAbilityComponent.h"
#include "UltraCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/KismetMathLibrary.h"

void UUltraAbilityComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Owner = Cast<AUltraCharacter>(GetOwner());
}

void UUltraAbilityComponent::AbilityTick(float DeltaTime, bool IsSimulating)
{
	AbilityDeltaTime = DeltaTime;

	if (Owner->Mouse1True && !IsSimulating)
	{
		if (Owner->TimeStamp - Owner->Mouse1StartTime > Owner->Mouse1Cooldown)
		{

		}
	}

	if (Owner->Mouse2True)
	{
		if (Owner->TimeStamp - Owner->Mouse2StartTime > Owner->Mouse2Cooldown)
		{
			Owner->BPMouse2AbilityStart();
		}
	}

	if (Owner->Mouse2AbilityIsPlaying)
	{
		Owner->BPMouse2AbilityTick(Owner->Mouse2True);
	}

	if (Owner->ETrue)
	{
		if (Owner->TimeStamp - Owner->EStartTime > Owner->ECooldown)
		{
			Owner->BPEAbilityStart();
		}
	}

	if (Owner->EAbilityIsPlaying)
	{
		Owner->BPEAbilityTick(Owner->ETrue);
	}

	if (Owner->QTrue)
	{
		if (Owner->TimeStamp - Owner->QStartTime > Owner->QCooldown)
		{
			Owner->BPQAbilityStart();
		}
	}

	if (Owner->QAbilityIsPlaying)
	{
		Owner->BPQAbilityTick(Owner->QTrue);
	}
}

void UUltraAbilityComponent::Mouse2AbilityTick(float DeltaTime)
{
	if (!Owner->Mouse2True)
	{
		if (!Owner->Mouse2MovementIsPlaying)
		{
			Mouse2MovementStart();
		}
	}

	if (Owner->Mouse2MovementIsPlaying)
	{
		Mouse2MovementTick(DeltaTime);
		Mouse2HitboxTick();
	}

	else
	{
		Owner->Mouse2StartTime = Owner->TimeStamp;
	}
}

void UUltraAbilityComponent::Mouse2MovementTick(float DeltaTime)
{
	float Multiplier = 1 + static_cast<float>(Owner->Mouse2Charge) / static_cast<float>(Owner->Mouse2MaxCharge);

	if (Multiplier < 1)
	{
		Multiplier = 1;
	}

	if (Multiplier > 2)
	{
		Multiplier = 2;
	}

	float magnitude = Owner->Mouse2Velocity * Multiplier * DeltaTime;
	FVector direction = Owner->Mouse2StartForwardVector;

	MovePlayer(magnitude, direction);

	if (Owner->TimeStamp - Owner->Mouse2StartMovementTime >= Owner->Mouse2Duration)
	{
		Mouse2AbilityEnd();
	}
}

void UUltraAbilityComponent::Mouse2HitboxTick()
{
	FHitResult Hit;
	const FVector PlayerLocation = Owner->GetActorLocation();
	
	bool HitboxCollided = SphereTrace(PlayerLocation, Owner->Mouse2HitboxRadius, Hit);

	if (HitboxCollided)
	{
		ATestCharacter* HitPlayer = Cast<ATestCharacter>(Hit.GetActor());

		if (IsValid(HitPlayer))
		{
			Mouse2AbilityEnd();
		}
	}
}

void UUltraAbilityComponent::Mouse2AbilityStart()
{
	Owner->Mouse2AbilityIsPlaying = true;
	Owner->Mouse2StartChargeTime = Owner->TimeStamp;

	Owner->MoveSpeed = Owner->BaseMoveSpeed * 0.5;
}

void UUltraAbilityComponent::Mouse2MovementStart()
{
	Owner->NoMovement = true;
	Owner->bUseControllerRotationYaw = false;
	Owner->Mouse2MovementIsPlaying = true;
	Owner->Mouse2StartMovementTime = Owner->TimeStamp;

	Owner->Mouse2StartForwardVector = UKismetMathLibrary::GetForwardVector(Owner->Rotation);
	Owner->Mouse2Charge = Owner->TimeStamp - Owner->Mouse2StartChargeTime;
}

void UUltraAbilityComponent::Mouse2AbilityEnd()
{
	Owner->NoMovement = false;
	Owner->bUseControllerRotationYaw = true;
	Owner->Mouse2AbilityIsPlaying = false;
	Owner->Mouse2MovementIsPlaying = false;
	Owner->MoveSpeed = Owner->BaseMoveSpeed;
}

void UUltraAbilityComponent::MovePlayer(
	const float magnitude,
	const FVector direction)
{
	FVector Delta = direction * magnitude * AbilityDeltaTime;
	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		SlideAlongSurface(Delta, (1.f - Hit.Time), Hit.Normal, Hit, false);
	}
}

bool UUltraAbilityComponent::SphereTrace(
	const FVector& Start,
	const float Radius,
	FHitResult& HitOut,
	ECollisionChannel TraceChannel)
{
	const FCollisionQueryParams TraceParams = Owner->GetIgnoreCharacterParams();
	HitOut = FHitResult(ForceInit);
	TArray <FOverlapResult> Overlaps;

	bool bMovableActorsInRange = GetWorld()->OverlapMultiByChannel(
		Overlaps,
		Start,
		FQuat(),
		TraceChannel,
		FCollisionShape::MakeSphere(Radius),
		TraceParams
	);

	return bMovableActorsInRange;
}

void UUltraAbilityComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}

