// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GrabbableComponent.h"
#include "InteractableComponent.generated.h"

UCLASS( Abstract, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class BURGLAR_API UInteractableComponent : public UGrabbableComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	virtual void Interact();
};
