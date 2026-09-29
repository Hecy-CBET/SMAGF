#include "Targets/SMAGFTarget.h"
#include "Combat/SMAGFHealthComponent.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "Feedback/SMAGFDeathVisual.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Engine/World.h"
#include "Engine/DamageEvents.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "UObject/ConstructorHelpers.h"

ASMAGFTarget::ASMAGFTarget()
{
	PrimaryActorTick.bCanEverTick = false;
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitBoxExtent(GetCollisionExtent());
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetGenerateOverlapEvents(false);
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetRelativeScale3D(GetCollisionExtent() / 50.f);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	Visual->SetStaticMesh(Mesh.Object);
	Health = CreateDefaultSubobject<USMAGFHealthComponent>(TEXT("Health"));
	Reaction = CreateDefaultSubobject<USMAGFReactionComponent>(TEXT("Reaction"));
	Health->OnDeath.AddDynamic(this, &ASMAGFTarget::OnDied);
}

void ASMAGFTarget::BeginPlay()
{
	Super::BeginPlay();
	Reaction->SetVisualComponent(Visual);
}

float ASMAGFTarget::TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (!CanBeDamaged() || !FMath::IsFinite(DamageAmount) || DamageAmount <= 0.f)
	{
		return 0.f;
	}
	// Point damage carries the actual shot direction before synchronous OnDeath is broadcast.
	AcceptedDamageDirection = DamageEvent.IsOfType(FPointDamageEvent::ClassID)
		? SMAGFMotion::PlanarDirection(static_cast<const FPointDamageEvent&>(DamageEvent).ShotDirection)
		: FVector::ForwardVector;
	return Health->ApplyDamage(Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser));
}

void ASMAGFTarget::OnDied(USMAGFHealthComponent* DeadHealth)
{
	SetCanBeDamaged(false);
	SetActorEnableCollision(false);
	const USMAGFShowcaseSubsystem* Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (Showcase && Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EnemyDeathReaction))
	{
		const FTransform Transform = GetActorTransform();
		if (ASMAGFDeathVisual* Copy = GetWorld()->SpawnActorDeferred<ASMAGFDeathVisual>(
			ASMAGFDeathVisual::StaticClass(), Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Copy->CopyVisual(Visual, Reaction->GetBaseVisualTransform(), AcceptedDamageDirection);
			Copy->FinishSpawning(Transform);
		}
	}
	Destroy();
}
