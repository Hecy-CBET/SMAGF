#include "Combat/SMAGFWeaponComponent.h"
#include "Combat/SMAGFProjectile.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "SMAGF.h"

USMAGFWeaponComponent::USMAGFWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	ProjectileClass = ASMAGFProjectile::StaticClass();
}

FVector USMAGFWeaponComponent::GetMuzzleLocation() const
{
	return GetOwner()->GetActorLocation() + GetOwner()->GetActorRotation().RotateVector(MuzzleOffset);
}

void USMAGFWeaponComponent::StartFire()
{
	bHeld = true;
	TryFire();
}

void USMAGFWeaponComponent::StopFire()
{
	bHeld = false;
}

void USMAGFWeaponComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopFire();
	Super::EndPlay(EndPlayReason);
}

void USMAGFWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	if (bHeld)
	{
		TryFire();
	}
}

void USMAGFWeaponComponent::TryFire()
{
	UWorld* World = GetWorld();
	if (!World || World->IsPaused() || !IsValid(GetOwner()) || !GetOwner()->HasActorBegunPlay())
	{
		return;
	}
	const double Now = World->GetTimeSeconds();
	if (Now < NextShotTime || LastShotFrame == GFrameCounter)
	{
		return;
	}
	// Schedule from now: missed intervals are discarded, never replayed as a burst.
	NextShotTime = Now + (FMath::IsFinite(FireInterval) ? FMath::Max(0.01f, FireInterval) : 0.25f);
	LastShotFrame = GFrameCounter;
	if (!ProjectileClass)
	{
		UE_LOG(LogSMAGF, Error, TEXT("%s has no projectile class."), *GetNameSafe(GetOwner()));
		return;
	}
	const FVector Muzzle = GetMuzzleLocation();
	if (Muzzle.ContainsNaN())
	{
		return;
	}
	// Prevent the muzzle spawning a shot through a nearby wall or target.
	FCollisionQueryParams Query(SCENE_QUERY_STAT(SMAGFMuzzle), false, GetOwner());
	FHitResult Obstruction;
	const FVector Start = GetOwner()->GetActorLocation() + FVector(0.f, 0.f, MuzzleOffset.Z);
	FVector SpawnLocation = Muzzle;
	if (World->SweepSingleByChannel(Obstruction, Start, Muzzle, FQuat::Identity, ECC_Visibility,
		FCollisionShape::MakeSphere(6.f), Query))
	{
		SpawnLocation = Start;
	}
	const FTransform Transform(GetOwner()->GetActorRotation(), SpawnLocation);
	ASMAGFProjectile* Projectile = World->SpawnActorDeferred<ASMAGFProjectile>(ProjectileClass, Transform,
		GetOwner(), Cast<APawn>(GetOwner()), ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile)
	{
		Projectile->InitializeShot(GetOwner()->GetActorForwardVector(), ProjectileSpeed, ProjectileDamage);
		Projectile->FinishSpawning(Transform);
	}
}
