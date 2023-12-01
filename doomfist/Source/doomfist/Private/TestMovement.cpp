// Fill out your copyright notice in the Description page of Project Settings.

#include "TestMovement.h"
#include "Components/CapsuleComponent.h"
#include "TestCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

void UTestMovement::InitializeComponent()
{
	Super::InitializeComponent();

	Owner = Cast<ATestCharacter>(GetOwner());
}

void UTestMovement::MovementTick(float DeltaTime)
{
	if (Owner->NoMovement)
	{
		Owner->LastVelocity = (Owner->GetActorLocation() - Owner->PreviousLocation) / DeltaTime;
		Owner->PreviousLocation = Owner->GetActorLocation();

		return;
	}

	FVector InputForce;

	if (IsGrounded())
	{
		Owner->bIsGrounded = true;
		FVector JumpForce = FVector::ZeroVector;

		if (Owner->SpaceBarTrue)
		{
			JumpForce = Owner->JumpForce * FVector::UpVector;
		}

		float frictionmultiplier = 1;

		if (!Owner->bNoFriction)
		{
			frictionmultiplier = 1 - Owner->GroundFriction * DeltaTime;
		}

		Velocity = Owner->LastVelocity * frictionmultiplier + Owner->InputVelocity * (1 + Owner->GroundFriction) * DeltaTime + JumpForce;
	}

	else
	{
		Owner->bIsGrounded = false;
		FVector GravityForce = FVector::DownVector * Owner->Gravity * DeltaTime;

		Velocity = Owner->LastVelocity * (1 - Owner->AirFriction * DeltaTime) + Owner->InputVelocity * 0.5 * (1 + Owner->AirFriction) * DeltaTime + GravityForce;
	}

	FVector Delta = Velocity * DeltaTime;
	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		SlideAlongSurface(Delta, (1.f - Hit.Time), Hit.Normal, Hit, false);
	}

	Owner->LastVelocity = (Owner->GetActorLocation() - Owner->PreviousLocation) / DeltaTime;
	Owner->PreviousLocation = Owner->GetActorLocation();
}

void UTestMovement::SimulatedMovementTick(float DeltaTime, bool updatedthistick)
{
	if (Owner->NoMovement)
	{
		return;
	}

	if (IsGrounded())
	{
		Owner->bIsGrounded = true;

		float frictionmultiplier = 1;

		if (!Owner->bNoFriction)
		{
			frictionmultiplier = 1 - Owner->GroundFriction * DeltaTime;
		}

		Velocity = Owner->LastVelocity * frictionmultiplier;
	}

	else
	{
		Owner->bIsGrounded = false;
		FVector GravityForce = FVector::DownVector * Owner->Gravity * DeltaTime;

		Owner->LastVelocity = Owner->LastVelocity * (1 - Owner->AirFriction * DeltaTime) + GravityForce;

		Velocity = Owner->LastVelocity * (1 - Owner->AirFriction * DeltaTime) + GravityForce;
	}

	FVector Delta = Velocity * DeltaTime;
	FHitResult Hit(1.f);

	SafeMoveUpdatedComponent(Delta, UpdatedComponent->GetComponentQuat(), true, Hit);

	if (Hit.Time < 1.f)
	{
		SlideAlongSurface(Delta, (1.f - Hit.Time), Hit.Normal, Hit, false);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Green, FString::Printf(TEXT("Hello %s"), *Owner->LastVelocity.ToString()));
	//GEngine->AddOnScreenDebugMessage(-1, 5, FColor::Blue, FString::Printf(TEXT("Hello %s"), *Owner->PreviousLocation.ToString()));
}

bool UTestMovement::IsGrounded()
{
	float Reach = Owner->GetCapsuleHeight() + 5;
	FVector LineTraceEnd = Owner->GetActorLocation() + FVector::DownVector * Reach;
	FHitResult Hit;

	bool IsGrounded = GetWorld()->LineTraceSingleByObjectType(
		OUT Hit,
		Owner->GetActorLocation(),
		LineTraceEnd,
		FCollisionObjectQueryParams(ECollisionChannel::ECC_WorldStatic),
		Owner->GetIgnoreCharacterParams()
	);

	return IsGrounded;
}

void UTestMovement::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
}
