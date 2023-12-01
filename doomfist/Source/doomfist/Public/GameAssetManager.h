// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "GameAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class DOOMFIST_API UGameAssetManager : public UAssetManager
{
	GENERATED_BODY()
	
	virtual void StartInitialLoading() override;
};
