#include "Feedback/SMAGFDeathVisual.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"

ASMAGFDeathVisual::ASMAGFDeathVisual()
{
	PrimaryActorTick.bCanEverTick = false;
	SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
	Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	Visual->SetupAttachment(RootComponent);
	Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Visual->SetGenerateOverlapEvents(false);
	Visual->SetCanEverAffectNavigation(false);
	Reaction = CreateDefaultSubobject<USMAGFReactionComponent>(TEXT("Reaction"));
	SetCanBeDamaged(false);
	SetActorEnableCollision(false);
}

void ASMAGFDeathVisual::CopyVisual(const UStaticMeshComponent* Source, const FTransform& AuthoredTransform, const FVector& IncomingDirection)
{
	DeathDirection = IncomingDirection;
	Visual->SetStaticMesh(Source->GetStaticMesh());
	for (int32 Index = 0; Index < Source->GetNumMaterials(); ++Index) { Visual->SetMaterial(Index, Source->GetMaterial(Index)); }
	Visual->SetRelativeTransform(AuthoredTransform);
}

void ASMAGFDeathVisual::BeginPlay()
{
	Super::BeginPlay();
	Reaction->SetVisualComponent(Visual);
	Reaction->BeginDeathPresentation(DeathDirection);
}
