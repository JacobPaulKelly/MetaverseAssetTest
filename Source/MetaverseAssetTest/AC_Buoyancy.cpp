
#include "AC_Buoyancy.h"
#include "GameFramework/GameSession.h"

// Sets default values for this component's properties
UAC_Buoyancy::UAC_Buoyancy()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAC_Buoyancy::BeginPlay()
{
	Super::BeginPlay();

	//getting the owning actor of this component.
	AActor* Parent = GetOwner();

	//setting up paramenters to recieve owning actors origins and box extents
	//dont care about the origins currently as all we need is the extents to calculate the volume
	FVector OwningActorOrigin;
	FVector OwningActorBoxExtent;

	//calling function to apply values to origin and extents
	Parent->GetActorBounds(false, OwningActorOrigin, OwningActorBoxExtent);

	//calculating the voolume
	ObjectsVolume = OwningActorBoxExtent.X * OwningActorBoxExtent.Y * OwningActorBoxExtent.Z;
	
	//calculating the mass of our actor for our next check
	ObjectsMass = ObjectsDensity * ObjectsVolume;

	//checking to see if the object should float and early return if it cant in preperation for buoyancy calculations.
	if (ObjectsDensity >= WaterDensity)
	{
		CanObjectFloat = false;

		return;
	}

	CanObjectFloat = true;
}


// Called every frame
void UAC_Buoyancy::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//getting and calculating the upthrust/buoyancy value
	Upthrust = BuoyancyForce(WaterDensity, GravitationalForce, ObjectsMass, ObjectsVolume, CanObjectFloat);

	//applying calculated force to object
	GetOwner()->addvelocity(Upthrust * DeltaTime);
	
}

const float BuoyancyForce(float fluidDensity, float gravitationalForce, float objectMass, float objectVolume, bool objectDoesFloat)
{
	float Buoyancy;
	float CombinedForces;
	float DisplacedVolume;

	CombinedForces = fluidDensity * gravitationalForce;

	if (objectDoesFloat)
	{
		DisplacedVolume = objectMass / fluidDensity;

		if (DisplacedVolume > fluidDensity)
		{
			DisplacedVolume = fluidDensity;
		}

		Buoyancy = CombinedForces * DisplacedVolume;

		return Buoyancy;
	}

	DisplacedVolume = objectVolume;

	Buoyancy = CombinedForces * DisplacedVolume;

	return Buoyancy;
}