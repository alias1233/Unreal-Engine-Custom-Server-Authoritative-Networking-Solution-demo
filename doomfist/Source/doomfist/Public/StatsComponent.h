// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TestCharacter.h"
#include "StatsComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class DOOMFIST_API UStatsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UStatsComponent();

	UPROPERTY(Transient)
	ATestCharacter* Owner;

	virtual void InitializeComponent() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated, EditDefaultsOnly)
	float MaxHealth;
	UPROPERTY(Replicated, EditDefaultsOnly)
	float Health;

protected:

public:	
	UFUNCTION(BluePrintCallable)
	void Damage(const float damage);
	UFUNCTION(BluePrintCallable)
	float GetHealth();
	UFUNCTION(BluePrintCallable)
	float GetMaxHealth();
};
