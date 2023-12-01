// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsComponent.h"
#include "TestCharacter.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/Pawn.h"

// Sets default values for this component's properties
void UStatsComponent::InitializeComponent()
{
	Super::InitializeComponent();

	Owner = Cast<ATestCharacter>(GetOwner());
}

UStatsComponent::UStatsComponent()
{
	MaxHealth = 100;
	Health = 100;
}

float UStatsComponent::GetHealth()
{
	return Health;
}

float UStatsComponent::GetMaxHealth()
{
	return MaxHealth;
}

void UStatsComponent::Damage(const float damage)
{
	Health -= damage;
}

void UStatsComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UStatsComponent, Health);
	DOREPLIFETIME(UStatsComponent, MaxHealth);
}

