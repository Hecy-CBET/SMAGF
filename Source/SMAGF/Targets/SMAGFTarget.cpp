#include "Targets/SMAGFTarget.h"
#include "Presentation/SMAGFEventPresentationComponent.h"
#include "Presentation/SMAGFPresentationVisual.h"
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
	CreateDefaultSubobject<USMAGFEventPresentationComponent>(TEXT("Presentation"));
	PrimaryActorTick.bCanEverTick = false;
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitBoxExtent(GetCollisionExtent());
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetGenerateOverlapEvents(false);
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetReceivesDecals(true);
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
	const FVector Position = GetActorLocation();
	const float Accepted = Health->ApplyDamage(Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser));
	if (Accepted > 0.f)
	{
		if (auto* Presentation = FindComponentByClass<USMAGFEventPresentationComponent>()) { Presentation->Hit(Position, AcceptedDamageDirection); }
	}
	return Accepted;
}

void ASMAGFTarget::OnDied(USMAGFHealthComponent* DeadHealth)
{
	SetCanBeDamaged(false);
	SetActorEnableCollision(false);
	if (auto* Presentation = FindComponentByClass<USMAGFEventPresentationComponent>()) { Presentation->Death(GetActorLocation(), AcceptedDamageDirection); }
	const USMAGFShowcaseSubsystem* Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (Showcase && (Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EnemyDeathReaction)
		|| Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::KeepEnemyBodies)))
	{
		const FTransform Transform = GetActorTransform();
		if (ASMAGFDeathVisual* Copy = GetWorld()->SpawnActorDeferred<ASMAGFDeathVisual>(
			ASMAGFDeathVisual::StaticClass(), Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
		{
			Copy->CopyVisual(Visual, Reaction->GetBaseVisualTransform(), AcceptedDamageDirection);
			Copy->FinishSpawning(Transform);
			ASMAGFPresentationVisual::TransferSurfaceEvidence(this, Copy->GetVisualComponent());
		}
	}
	Destroy();
}
