#include "Combat/SMAGFProjectile.h"
#include "Combat/SMAGFHealthComponent.h"
#include "Feedback/SMAGFFeedbackSubsystem.h"
#include "Feedback/SMAGFReactionComponent.h"
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
	PrimaryActorTick.bCanEverTick = false;
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
}

void ASMAGFProjectile::OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComponent, FVector NormalImpulse, const FHitResult& Hit)
{
	if (bConsumed || OtherActor == GetOwner() || OtherActor == GetInstigator())
	{
		return;
	}
	bConsumed = true;
	Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Movement->StopMovementImmediately();
	if (IsValid(OtherActor))
	{
		// Retain health through synchronous death/destruction callbacks; no concrete target cast.
		TStrongObjectPtr<USMAGFHealthComponent> Health(OtherActor->FindComponentByClass<USMAGFHealthComponent>());
		const float Before = Health.IsValid() ? Health->CurrentHealth : 0.f;
		const float ReportedDamage = UGameplayStatics::ApplyPointDamage(OtherActor, Damage, ShotDirection, Hit,
			GetInstigatorController(), this, UDamageType::StaticClass());
		const float AppliedDamage = Health.IsValid() ? Before - Health->CurrentHealth : ReportedDamage;
		const bool bKilled = Health.IsValid() && Before > 0.f && Health->CurrentHealth <= 0.f;
		USMAGFReactionComponent* Reaction = IsValid(OtherActor) ? OtherActor->FindComponentByClass<USMAGFReactionComponent>() : nullptr;
		// A wall without a health/feedback receiver is not a damaging hit.
		if (AppliedDamage > 0.f && (Health.IsValid() || Reaction))
		{
			if (Reaction) { Reaction->OnDamagingHit(ShotDirection); }
			if (USMAGFFeedbackSubsystem* Feedback = GetWorld()->GetSubsystem<USMAGFFeedbackSubsystem>()) { Feedback->NotifyDamagingHit(bKilled); }
		}
	}
	Destroy();
}
