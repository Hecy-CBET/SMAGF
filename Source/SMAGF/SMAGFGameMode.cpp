#include "SMAGFGameMode.h"
#include "SMAGF.h"
#include "SMAGFCharacter.h"
#include "SMAGFPlayerController.h"
#include "Targets/SMAGFTarget.h"
#include "Engine/World.h"

ASMAGFGameMode::ASMAGFGameMode()
{
	DefaultPawnClass = ASMAGFCharacter::StaticClass();
	PlayerControllerClass = ASMAGFPlayerController::StaticClass();
}

void ASMAGFGameMode::BeginPlay()
{
	Super::BeginPlay();
	// Flat platforms: the previous 650 cm offsets landed on the sloped ramps.
	const FVector2D Positions[] = { {425.f, 0.f}, {-425.f, 0.f}, {0.f, 425.f}, {0.f, -425.f} };
	const FVector Extent = ASMAGFTarget::GetCollisionExtent();
	FCollisionQueryParams Query(SCENE_QUERY_STAT(SMAGFTargetGround), false);
	const FCollisionObjectQueryParams GroundObjects(ECC_WorldStatic);
	for (const FVector2D& XY : Positions)
	{
		FHitResult Ground;
		if (!GetWorld()->LineTraceSingleByObjectType(Ground, FVector(XY.X, XY.Y, 2000.f),
			FVector(XY.X, XY.Y, -2000.f), GroundObjects, Query) || Ground.ImpactNormal.Z < 0.9f)
		{
			UE_LOG(LogSMAGF, Warning, TEXT("Skipping target at %s: no suitable ground."), *XY.ToString());
			continue;
		}
		bool bSupported = true;
		for (float X : {-Extent.X, Extent.X})
		{
			for (float Y : {-Extent.Y, Extent.Y})
			{
				FHitResult Corner;
				const FVector Point(XY.X + X, XY.Y + Y, Ground.ImpactPoint.Z);
				if (!GetWorld()->LineTraceSingleByObjectType(Corner, Point + FVector(0.f, 0.f, 10.f),
					Point - FVector(0.f, 0.f, 10.f), GroundObjects, Query)
					|| Corner.ImpactNormal.Z < 0.9f || FMath::Abs(Corner.ImpactPoint.Z - Point.Z) > 2.f)
				{
					bSupported = false;
				}
			}
		}
		const FVector Location(XY.X, XY.Y, Ground.ImpactPoint.Z + Extent.Z + 2.f);
		if (!bSupported || GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity,
			ECC_WorldDynamic, FCollisionShape::MakeBox(Extent), Query))
		{
			UE_LOG(LogSMAGF, Warning, TEXT("Skipping target at %s: unsupported or obstructed."), *XY.ToString());
			continue;
		}
		FActorSpawnParameters Params;
		Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding;
		if (ASMAGFTarget* Target = GetWorld()->SpawnActor<ASMAGFTarget>(Location, FRotator::ZeroRotator, Params))
		{
			UE_LOG(LogSMAGF, Log, TEXT("Baseline target %s at %s (ground Z %.1f)."),
				*Target->GetName(), *Location.ToString(), Ground.ImpactPoint.Z);
		}
	}
}
