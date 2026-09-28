#include "Targets/SMAGFTarget.h"
#include "Combat/SMAGFHealthComponent.h"
#include "Feedback/SMAGFReactionComponent.h"
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
	return Health->ApplyDamage(Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser));
}

void ASMAGFTarget::OnDied(USMAGFHealthComponent* DeadHealth)
{
	SetCanBeDamaged(false);
	SetActorEnableCollision(false);
	Destroy();
}
