// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerBoat.h"

// Sets default values
APlayerBoat::APlayerBoat()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APlayerBoat::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void APlayerBoat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void APlayerBoat::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

