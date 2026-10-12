#include "Targets/SMAGFTarget.h"
#include "SMAGFGameMode.h"
#include "Showcase/SMAGFShowcaseDirector.h"
#include "AIController.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Enemy/SMAGFEnemyShowcaseComponent.h"
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
	CreateDefaultSubobject<USMAGFEnemyShowcaseComponent>(TEXT("EnemyShowcase"));
	CreateDefaultSubobject<USMAGFEventPresentationComponent>(TEXT("Presentation"));
	PrimaryActorTick.bCanEverTick = false;
	AIControllerClass=AAIController::StaticClass();
	AutoPossessAI=EAutoPossessAI::PlacedInWorldOrSpawned;
	bUseControllerRotationYaw=false; bUseControllerRotationPitch=false; bUseControllerRotationRoll=false;
	Collision = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	SetRootComponent(Collision);
	Collision->InitBoxExtent(GetCollisionExtent());
	Collision->SetCollisionProfileName(TEXT("BlockAllDynamic"));
	Collision->SetGenerateOverlapEvents(false);
	Collision->SetCanEverAffectNavigation(false);
	NavigationMovement=CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("NavigationMovement"));
	NavigationMovement->SetUpdatedComponent(Collision);
	NavigationMovement->NavAgentProps.AgentRadius=FMath::Max(Collision->GetScaledBoxExtent().X,Collision->GetScaledBoxExtent().Y);
	NavigationMovement->NavAgentProps.AgentHeight=2.f*Collision->GetScaledBoxExtent().Z;
	NavigationMovement->Acceleration=600.f; NavigationMovement->Deceleration=700.f;
	NavigationMovement->GetNavMovementProperties()->bUseAccelerationForPaths=true;
	// Attack ranges are planar, but locomotion must retain navigation-path elevation.
	NavigationMovement->SetPlaneConstraintEnabled(false);
	NavigationMovement->PrimaryComponentTick.bStartWithTickEnabled=false;
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(Collision);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetReceivesDecals(true);
	Visual->SetRelativeScale3D(GetCollisionExtent() / 50.f);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	Visual->SetStaticMesh(Mesh.Object);
	EnemyTelegraphMesh = Mesh.Object;
	Health = CreateDefaultSubobject<USMAGFHealthComponent>(TEXT("Health"));
	Reaction = CreateDefaultSubobject<USMAGFReactionComponent>(TEXT("Reaction"));
	Health->OnDeath.AddDynamic(this, &ASMAGFTarget::OnDied);
}

UPawnMovementComponent* ASMAGFTarget::GetMovementComponent() const { return NavigationMovement; }

FVector ASMAGFTarget::GetNavAgentLocation() const
{
	// Match UNavMovementComponent::GetActorFeetLocation for the actual collision bounds.
	return Collision ? Collision->GetComponentLocation()-FVector(0,0,Collision->Bounds.BoxExtent.Z)
		: Super::GetNavAgentLocation();
}

void ASMAGFTarget::BeginPlay()
{
	Super::BeginPlay();
	Reaction->SetVisualComponent(Visual);
	if (auto* Director=ASMAGFShowcaseDirector::Get(GetWorld())) { Director->RegisterTarget(this); }
}

bool ASMAGFTarget::IsAliveForFocus() const
{ return !IsActorBeingDestroyed() && Health && Health->CurrentHealth>0.f; }

FVector ASMAGFTarget::GetFocusAnchor() const
{ return Collision ? Collision->Bounds.Origin : GetActorLocation(); }

void ASMAGFTarget::EndPlay(const EEndPlayReason::Type Reason)
{
	if (auto* Director=ASMAGFShowcaseDirector::Get(GetWorld())) { Director->UnregisterTarget(this); }
	Super::EndPlay(Reason);
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
	// Health commits its one-shot Alive -> Dead transition before this delegate.
	if (auto* Mode = GetWorld()->GetAuthGameMode<ASMAGFGameMode>())
	{ Mode->PlayHorrorMonsterDeathSoundAtLocation(GetActorLocation()); }
	if (auto* Director=ASMAGFShowcaseDirector::Get(GetWorld())) { Director->UnregisterTarget(this); }
	if (auto* Enemy=FindComponentByClass<USMAGFEnemyShowcaseComponent>()) { Enemy->StopForDeath(); }
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
