// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbableComponent.h"
#include "StealableComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BURGLAR_API UStealableComponent : public UGrabbableComponent
{
	GENERATED_BODY()

public:
	UStealableComponent();

	bool IsNeeded() { return bIsNeeded; }

	float MoneyValue() { return MoneyWorth; }

protected:
	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Stealable" )
	float MoneyWorth;

	UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Stealable" )
	bool bIsNeeded;
};
