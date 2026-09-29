#include "Combat/SMAGFProjectile.h"
#include "Combat/SMAGFHealthComponent.h"
#include "Feedback/SMAGFFeedbackSubsystem.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/World.h"
#include "UObject/StrongObjectPtr.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ASMAGFProjectile::ASMAGFProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = TG_PostPhysics;
	InitialLifeSpan = 3.f;
	Collision = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitSphereRadius(6.f);
	Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Collision->SetCollisionObjectType(ECC_WorldDynamic);
	Collision->SetCollisionResponseToAllChannels(ECR_Block);
	Collision->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	Collision->SetGenerateOverlapEvents(false);
	Collision->OnComponentHit.AddDynamic(this, &ASMAGFProjectile::OnImpact);
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetRelativeScale3D(FVector(0.12f));
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	Visual->SetStaticMesh(Mesh.Object);
	// Inspected existing material: emissive = "Emissive Color" * "Emissive Strength".
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/Variant_TwinStick/M_Glow.M_Glow"));
	GlowMaterial = Glow.Object;
	Movement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));
	Movement->SetUpdatedComponent(Collision);
	Movement->InitialSpeed = 2000.f;
	Movement->MaxSpeed = 2000.f;
	Movement->ProjectileGravityScale = 0.f;
	Movement->bInitialVelocityInLocalSpace = false;
	Movement->bShouldBounce = false;
	Movement->bSweepCollision = true;
}

void ASMAGFProjectile::InitializeShot(const FVector& Direction, float Speed, float InDamage)
{
	ShotDirection = Direction.GetSafeNormal2D();
	FlightDirection = SMAGFMotion::PlanarDirection(ShotDirection);
	Damage = FMath::IsFinite(InDamage) ? FMath::Max(0.f, InDamage) : 0.f;
	const float SafeSpeed = FMath::IsFinite(Speed) ? FMath::Max(1.f, Speed) : 2000.f;
	Movement->InitialSpeed = SafeSpeed;
	Movement->MaxSpeed = SafeSpeed;
	Movement->Velocity = ShotDirection * SafeSpeed;
}

void ASMAGFProjectile::BeginPlay()
{
	Super::BeginPlay();
	Collision->IgnoreActorWhenMoving(GetOwner(), true);
	Collision->IgnoreActorWhenMoving(GetInstigator(), true);
	BaseVisual = Visual->GetRelativeTransform();
	BaseMaterial = Visual->GetMaterial(0);
	Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (Showcase.IsValid())
	{
		SettingsHandle = Showcase->OnSettingsChangedNative.AddUObject(this, &ASMAGFProjectile::SettingsChanged);
		SettingsChanged();
	}
}

void ASMAGFProjectile::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(SettingsHandle); }
	Super::EndPlay(Reason);
}

void ASMAGFProjectile::SettingsChanged()
{
	const FSMAGFShowcaseSettings Settings = Showcase->GetSettings();
	Feeling = Settings.Feeling;
	Animation = Settings.Animation;
	MasterIntensity = Settings.Master.MasterIntensity;
	bReadability = Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::ProjectileReadability);
	bAnimation = Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::ProjectileAnimation);
	if (bConsumed && !bAnimation)
	{
		ImpactRemaining = 0.f;
		SetActorHiddenInGame(true); // Cleanup even while paused, retire outside the delegate.
	}
	UpdateVisual();
}

void ASMAGFProjectile::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	if (bConsumed)
	{
		ImpactRemaining -= DeltaSeconds;
		if (ImpactRemaining <= 0.f) { Destroy(); return; }
	}
	else
	{
		VisualAge += DeltaSeconds;
		if (!Movement->Velocity.IsNearlyZero()) { FlightDirection = Movement->Velocity.GetSafeNormal(); }
	}
	UpdateVisual();
}

void ASMAGFProjectile::UpdateVisual(bool bCaptureFlight)
{
	FTransform Result = BaseVisual;
	FVector Scale = BaseVisual.GetScale3D() * (bReadability ? Feeling.ProjectileVisualScale : 1.f);
	if (bAnimation && (!bConsumed || bCaptureFlight))
	{
		// The authored primitive is a sphere: choose visible local +X as its longitudinal axis.
		// Only its relative rotation follows velocity; the collision root is untouched.
		const FQuat WorldRotation = FRotationMatrix::MakeFromX(FlightDirection).ToQuat();
		Result.SetRotation(Collision->GetComponentQuat().Inverse() * WorldRotation * BaseVisual.GetRotation());
		const float AgeAlpha = FMath::Clamp(VisualAge / 3.f, 0.f, 1.f); // Original gameplay lifetime.
		const float Stretch = 1.f + (Animation.ProjectileFlightStretch - 1.f) * MasterIntensity * AgeAlpha;
		const float Growth = 1.f + (Animation.ProjectileFlightGrowth - 1.f) * MasterIntensity * AgeAlpha;
		Scale *= FVector(Stretch, 1.f, 1.f) * Growth;
	}
	if (bConsumed && bAnimation && !bCaptureFlight)
	{
		Scale = ImpactScale;
		Result.SetRotation(ImpactRotation);
	}
	Result.SetScale3D(Scale);
	Visual->SetRelativeTransform(Result); // Sole writer of projectile presentation.
	if (bReadability && GlowMaterial)
	{
		if (!GlowInstance) { GlowInstance = UMaterialInstanceDynamic::Create(GlowMaterial, this); }
		if (GlowInstance)
		{
			Visual->SetMaterial(0, GlowInstance);
			const float Alpha = Feeling.ProjectileGlowFadeDuration > 0.f
				? FMath::Clamp(VisualAge / Feeling.ProjectileGlowFadeDuration, 0.f, 1.f) : 1.f;
			GlowInstance->SetVectorParameterValue(TEXT("Emissive Color"), FMath::Lerp(FLinearColor(1.f, 0.f, 0.f), FLinearColor(1.f, 0.65f, 0.02f), Alpha));
			GlowInstance->SetScalarParameterValue(TEXT("Emissive Strength"), FMath::Lerp(25.f, 3.f, Alpha));
		}
	}
	else { Visual->SetMaterial(0, BaseMaterial); }
}

void ASMAGFProjectile::OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bConsumed || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}
	bConsumed = true;
	// Hit traces retain travel direction even if the movement component already stopped on contact.
	const FVector Travel = !Movement->Velocity.IsNearlyZero() ? Movement->Velocity
		: (!((Hit.TraceEnd - Hit.TraceStart).IsNearlyZero()) ? Hit.TraceEnd - Hit.TraceStart : FlightDirection);
	const FVector AcceptedDirection = SMAGFMotion::PlanarDirection(Travel);
	FlightDirection = AcceptedDirection;
	// Capture the pre-impact visible shape before entering the consumed presentation state.
	UpdateVisual(true);
	ImpactScale = Visual->GetRelativeScale3D();
	ImpactRotation = Visual->GetRelativeRotation().Quaternion();
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Movement->StopMovementImmediately();
	if (IsValid(OtherActor))
	{
		// Retain health through synchronous death/destruction callbacks; no concrete target cast.
		TStrongObjectPtr<USMAGFHealthComponent> Health(OtherActor->FindComponentByClass<USMAGFHealthComponent>());
		const float Before = Health.IsValid() ? Health->CurrentHealth : 0.f;
		const float ReportedDamage = UGameplayStatics::ApplyPointDamage(OtherActor, Damage, AcceptedDirection, Hit,
			GetInstigatorController(), this, UDamageType::StaticClass());
		const float AppliedDamage = Health.IsValid() ? Before - Health->CurrentHealth : ReportedDamage;
		const bool bKilled = Health.IsValid() && Before > 0.f && Health->CurrentHealth <= 0.f;
		USMAGFReactionComponent* Reaction = IsValid(OtherActor) ? OtherActor->FindComponentByClass<USMAGFReactionComponent>() : nullptr;
		// A wall without a health/feedback receiver is not a damaging hit.
		if (AppliedDamage > 0.f && (Health.IsValid() || Reaction))
		{
			if (Reaction) { Reaction->OnDamagingHit(AcceptedDirection); }
			if (USMAGFFeedbackSubsystem* Feedback = GetWorld()->GetSubsystem<USMAGFFeedbackSubsystem>()) { Feedback->NotifyDamagingHit(bKilled); }
		}
	}
	if (bAnimation && Animation.ProjectileImpactAnimationDuration > 0.f)
	{
		// Scale deviation is neutral at intensity zero; timing never changes with intensity.
		const float Squash = FMath::Clamp(1.f + (Animation.ProjectileImpactSquash - 1.f) * MasterIntensity, 0.1f, 1.f);
		ImpactScale *= FVector(Squash, FMath::InvSqrt(Squash), FMath::InvSqrt(Squash));
		ImpactRemaining = Animation.ProjectileImpactAnimationDuration;
		SetLifeSpan(0.f); // Consumed, collision-free tail owns the remaining visual lifetime.
		UpdateVisual();
	}
	else { Destroy(); }
}
