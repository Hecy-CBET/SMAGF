#include "Enemy/SMAGFEnemyProjectile.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "SMAGFCharacter.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"
ASMAGFEnemyProjectile::ASMAGFEnemyProjectile()
{
 InitialLifeSpan=4.f;
 Collision=CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));SetRootComponent(Collision);
 Collision->InitSphereRadius(6.f);Collision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
 Collision->SetCollisionObjectType(ECC_WorldDynamic);Collision->SetCollisionResponseToAllChannels(ECR_Ignore);
 Collision->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);Collision->SetCollisionResponseToChannel(ECC_Pawn,ECR_Block);
 Collision->SetGenerateOverlapEvents(false);Collision->OnComponentHit.AddDynamic(this,&ThisClass::OnImpact);
 Visual=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));Visual->SetupAttachment(Collision);
 Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);Visual->SetCanEverAffectNavigation(false);Visual->SetReceivesDecals(false);
 Visual->SetRelativeScale3D(FVector(.2f));
 static ConstructorHelpers::FObjectFinder<UStaticMesh> Mesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));Visual->SetStaticMesh(Mesh.Object);
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> M(TEXT("/Game/Variant_TwinStick/M_Glow.M_Glow"));Glow=M.Object;
 Movement=CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Movement"));Movement->SetUpdatedComponent(Collision);
 Movement->ProjectileGravityScale=0.f;Movement->bInitialVelocityInLocalSpace=false;Movement->bSweepCollision=true;
}
void ASMAGFEnemyProjectile::InitializeFlight(const FVector& Direction,float Speed)
{
 FlightDirection=Direction.GetSafeNormal2D();
 Movement->InitialSpeed=Speed;Movement->MaxSpeed=Speed;Movement->Velocity=Direction.GetSafeNormal2D()*Speed;
}
void ASMAGFEnemyProjectile::BeginPlay()
{
 Super::BeginPlay();Collision->IgnoreActorWhenMoving(GetOwner(),true);
 Material=UMaterialInstanceDynamic::Create(Glow,this);Visual->SetMaterial(0,Material);
 if(Material) { Material->SetVectorParameterValue(TEXT("Emissive Color"),FLinearColor(0.f,.7f,1.f));Material->SetScalarParameterValue(TEXT("Emissive Strength"),4.f); }
 Showcase=GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
 if(Showcase.IsValid()) { Handle=Showcase->OnSettingsChangedNative.AddUObject(this,&ThisClass::SettingsChanged); }
 SettingsChanged();
}
void ASMAGFEnemyProjectile::SettingsChanged()
{
 if(!Showcase.IsValid() || !Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EnemyThreat)) { Destroy(); }
}
void ASMAGFEnemyProjectile::OnImpact(UPrimitiveComponent*,AActor* Other,UPrimitiveComponent*,FVector,const FHitResult& Hit)
{
 if(bConsumed) { return; } bConsumed=true;
 const FVector Direction=FlightDirection;
 Movement->StopMovementImmediately();Collision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 if(auto* Player=Cast<ASMAGFCharacter>(Other))
 { if(auto* Reaction=Player->FindComponentByClass<USMAGFReactionComponent>()) { Reaction->ReceiveShowcaseHostileHit(Direction); } }
 Destroy();
}
void ASMAGFEnemyProjectile::EndPlay(const EEndPlayReason::Type Reason)
{
 if(Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(Handle); }
 Super::EndPlay(Reason);
}
