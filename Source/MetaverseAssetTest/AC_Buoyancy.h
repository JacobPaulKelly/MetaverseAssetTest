// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AC_Buoyancy.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class METAVERSEASSETTEST_API UAC_Buoyancy : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAC_Buoyancy();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	AActor* Parent;

	UStaticMeshComponent* MeshRootComp;

	const float WaterDensity = 997;

	const float GravitationalForce = 9.807;

	const float SeaLevel = 0;

	bool CanObjectFloat;
	
	float ObjectsMass;

	float ObjectsVolume;

	FVector ObjectsLocation;

	float ObjectsHeight;

	FVector Upthrust;

	const FVector BuoyancyForce(float fluidDensity, float gravitationalForce, float objectMass, float objectVolume);

	const void ApplyForces();

	void BuoyancyPointsToAddForce(FVector drag);

	//Editable Variables
	UPROPERTY(EditAnywhere)
	float ObjectsDensity;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
