
#include "AC_Buoyancy.h"
#include "GameFramework/GameSession.h"
#include "DrawDebugHelpers.h"

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
	Parent = GetOwner();

	//should add a check here to see if the root is a valid static mesh.
	MeshRootComp = Cast<UStaticMeshComponent>(Parent->GetRootComponent());

	//setting up paramenters to recieve owning actors origins and box extents
	//dont care about the origins currently as all we need is the extents to calculate the volume
	FVector OwningActorOrigin;
	FVector OwningActorBoxExtent;

	//calling function to apply values to origin and extents
	//MeshRootComp->GetLocalBounds(OwningActorOrigin, OwningActorBoxExtent); //didnt work
	Parent->GetActorBounds(false, OwningActorOrigin, OwningActorBoxExtent);

	//grabbing the height of the object to calc how much of the object is submerged.
	ObjectsHeight = OwningActorBoxExtent.Z;

	//must divide by 100 to convert from cm to m.
	ObjectsVolume = 
		((OwningActorBoxExtent.X/100)*2) *
		((OwningActorBoxExtent.Y/100)*2) *
		((OwningActorBoxExtent.Z/100)*2);


	//calculating the mass of our actor for later as this shouldnt change
	ObjectsMass = ObjectsDensity * ObjectsVolume;
}


// Called every frame
void UAC_Buoyancy::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//getting and calculating the upthrust/buoyancy value
	Upthrust = BuoyancyForce(WaterDensity, GravitationalForce, ObjectsMass, ObjectsVolume);

	//applying calculated force to object
	ApplyForces();
	
}

const FVector UAC_Buoyancy::BuoyancyForce(float fluidDensity, float gravitationalForce, float objectMass, float objectVolume)
{
	float Buoyancy;
	float DisplacedVolume;

	//getting the objects location to see how submerged it is so we can adjust buoyancy accordingly.
	ObjectsLocation = GetOwner()->GetRootComponent()->GetComponentLocation();

	//Calculating and clamping the objects submersion and its factor to multiply with
	float SubmersionDepth = FMath::Max(0, SeaLevel - ObjectsLocation.Z);
	float SubmersionFactor = FMath::Clamp(SubmersionDepth / (ObjectsHeight/4), 0, 1);

	DisplacedVolume = objectVolume * SubmersionFactor;

	//clamping to ensure that displaced volume cant exceed the objects volume, this means all fluid is displaced
	if (DisplacedVolume > objectVolume)
	{
		DisplacedVolume = objectVolume;
	}

	//Buoyancy formula
	Buoyancy = fluidDensity * gravitationalForce * DisplacedVolume;

	// do this to make sure gravity is being applied correctly to buoyancy
	float GravityForce = objectMass * GravitationalForce;

	FVector TotalForce = FVector(0, 0, Buoyancy - GravityForce);

	return TotalForce;
}

const void UAC_Buoyancy::ApplyForces()
{
	//getting the current velocity vector to know what direction we are going
	//to apply drag
	FVector ObjectLinearVelocity = MeshRootComp->GetPhysicsLinearVelocity();

	//Some Value for different drag types; fast drag, and slow drag
	float Drag = 0.8;
	float QuadraticDrag = 0.2f;

	//For slow speed drag
	FVector LinearDragForce = -ObjectLinearVelocity * Drag;
	//for fast speed drag
	FVector QuadraricDragForce = -QuadraticDrag * ObjectLinearVelocity * ObjectLinearVelocity.Size();

	//for total drag force being applied
	FVector DragForce = LinearDragForce + QuadraricDragForce;

	//checking to see if we are above sea level to drastically reduce the drag from water so that it feels more realistic
	if (ObjectsLocation.Z>SeaLevel)
	{
		DragForce = DragForce / 100;
	}

	// here we are going to use the force calculated and apply it to several points on the ship for realism
	BuoyancyPointsToAddForce(DragForce);

	return void();
}

void UAC_Buoyancy::BuoyancyPointsToAddForce(FVector drag)
{
	//using parent transform so we can get the actual position of the point as the boat will
	//rotate from different buoyancies in different points on the object	
	FTransform ParentTransform = Parent->GetActorTransform();

	FVector Bounds;
	FVector Origins;
	Parent->GetActorBounds(false, Origins, Bounds);

	//four corners (and a centre point to help with stability), also found that puting the Z bounds helped make the floating look better
	TArray<FVector>BuoyancyDistributionPoints = {
		FVector(Bounds.X,Bounds.Y,-Bounds.Z / 4),
		FVector(-Bounds.X,Bounds.Y,-Bounds.Z /4),
		FVector(Bounds.X,-Bounds.Y,-Bounds.Z/4),
		FVector(-Bounds.X,-Bounds.Y,-Bounds.Z/4),
		FVector(0,0,-Bounds.Z/2)
	};

	//looping over each vector in the array and applying forces based on current location
	for (FVector Points : BuoyancyDistributionPoints)
	{
		//using built in fuction to get location of points with rotation taken into consideration
		FVector WorldPoint = ParentTransform.TransformPosition(Points);

		DrawDebugSphere(GetWorld(), WorldPoint, 10,10, FColor::Red, false, 0.1);

		//Calculating how much of a point is submerged
		float SubmersionDepth = FMath::Max(0, SeaLevel - WorldPoint.Z);
		float SubmersionFactor = FMath::Clamp(SubmersionDepth / Bounds.Z, 0, 1);

		//checking if we are in pie mode as getting different results for physics in game
		if (GetWorld()->WorldType == EWorldType::PIE)
		{
		if (SubmersionDepth > 0)
		{
			FVector BuoyancyForce = (Upthrust * SubmersionFactor) / BuoyancyDistributionPoints.Num();
			MeshRootComp->AddForceAtLocationLocal(BuoyancyForce + drag, Points);
			
		}
		continue;
		}

		//made it feel better when no forces were applied when out of water
		if (SubmersionDepth > 0)
		{
			FVector BuoyancyForce = (Upthrust * SubmersionFactor);
			MeshRootComp->AddForceAtLocationLocal(BuoyancyForce + drag, Points);

			//UE_LOG(LogTemp, Warning, TEXT("Buoyancy Force at %s: %s"), *WorldPoint.ToString(), *BuoyancyForce.ToString());
		}
	}
}
