// Fill out your copyright notice in the Description page of Project Settings.


#include "GameAssetManager.h"
#include "AbilitySystemGlobals.h"

void UGameAssetManager::StartInitialLoading()
{
    Super::StartInitialLoading();

    //Required for using TargetData.
    UAbilitySystemGlobals::Get().InitGlobalData();
}