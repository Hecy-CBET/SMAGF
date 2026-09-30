#include "Feedback/SMAGFDeathVisual.h"
#include "Presentation/SMAGFPermanenceSubsystem.h"
#include "Presentation/SMAGFPresentationVisual.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "UObject/ConstructorHelpers.h"

ASMAGFDeathVisual::ASMAGFDeathVisual()
{
 PrimaryActorTick.bCanEverTick = true;
 PrimaryActorTick.bStartWithTickEnabled = false;
 SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
 Visual = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
 Visual->SetupAttachment(RootComponent);
 Visual->SetCollisionEnabled(ECollisionEnabled::NoCollision);
 Visual->SetReceivesDecals(true);
 Visual->SetGenerateOverlapEvents(false);
 Visual->SetCanEverAffectNavigation(false);
 Reaction = CreateDefaultSubobject<USMAGFReactionComponent>(TEXT("Reaction"));
 static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/SMAGF/Materials/MI_SMAGF_CorpseDissolve.MI_SMAGF_CorpseDissolve"));
 DissolveTemplate = Material.Object;
 SetCanBeDamaged(false);
 SetActorEnableCollision(false);
}
void ASMAGFDeathVisual::CopyVisual(const UStaticMeshComponent* Source, const FTransform& AuthoredTransform, const FVector& IncomingDirection)
{
 DeathDirection = IncomingDirection;
 Visual->SetStaticMesh(Source->GetStaticMesh());
 for (int32 I=0; I<Source->GetNumMaterials(); ++I) { Visual->SetMaterial(I,Source->GetMaterial(I)); }
 Visual->SetRelativeTransform(AuthoredTransform);
}
void ASMAGFDeathVisual::BeginPlay()
{
 Super::BeginPlay();
 Showcase=GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
 const auto S=Showcase.IsValid()?Showcase->GetSettings():FSMAGFShowcaseSettings();
 if (auto* P=GetWorld()->GetSubsystem<USMAGFPermanenceSubsystem>())
 { bRetained=P->Retain(this,ESMAGFShowcaseFeature::KeepEnemyBodies); }
 const float Lifetime=bRetained?S.Permanence.CorpseLifetime:S.Animation.ReactionDuration+S.Animation.RecoveryDuration;
 RetirementDeadline=GetWorld()->GetTimeSeconds()+Lifetime;
 // Own the deadline so terminal MID values and attached evidence retire together.
 SetLifeSpan(0.f);
 if (Showcase.IsValid()) { SettingsHandle=Showcase->OnSettingsChangedNative.AddUObject(this,&ThisClass::SettingsChanged); }
 if (bRetained)
 {
  Reaction->SetComponentTickEnabled(false);
  PrepareFall(S.Animation.ReactionDuration);
  DissolveDuration=FMath::Clamp(S.Permanence.CorpseDissolveDuration,0.f,Lifetime);
  if (DissolveDuration>0.f)
  {
   const float Hold=FMath::Max(0.f,Lifetime-DissolveDuration);
   if (Hold>0.f) { GetWorldTimerManager().SetTimer(DissolveTimer,this,&ThisClass::BeginDissolve,Hold,false); }
   else { BeginDissolve(); }
  }
 }
 else
 {
  Reaction->SetVisualComponent(Visual);
  Reaction->BeginDeathPresentation(DeathDirection);
 }
 if (Lifetime>0.f) { GetWorldTimerManager().SetTimer(RetirementTimer,this,&ThisClass::Retire,Lifetime,false); }
 else { Retire(); }
 SettingsChanged();
}
void ASMAGFDeathVisual::PrepareFall(float Duration)
{
 FallStart=Visual->GetComponentTransform(); FallEnd=FallStart;
 const FVector Direction=SMAGFMotion::PlanarDirection(DeathDirection);
 // A retained body has a complete terminal pose independent of transient reaction gain.
 FallEnd.SetRotation(FQuat(SMAGFMotion::FallAxis(Direction),PI*.5f)*FallStart.GetRotation());
 const FBoxSphereBounds Bounds=Visual->CalcBounds(FallEnd);
 FHitResult Hit;
 FCollisionQueryParams Query(SCENE_QUERY_STAT(SMAGFCorpseSupport),false,this);
 const FVector Start=Bounds.Origin+FVector(0,0,Bounds.BoxExtent.Z+50.f);
 if (GetWorld()->LineTraceSingleByObjectType(Hit,Start,Start-FVector(0,0,10000.f),FCollisionObjectQueryParams(ECC_WorldStatic),Query))
 {
  FVector Location=FallEnd.GetLocation();
  Location.Z+=Hit.ImpactPoint.Z-(Bounds.Origin.Z-Bounds.BoxExtent.Z);
  FallEnd.SetLocation(Location);
 }
 FallDuration=FMath::Max(0.f,Duration); bFalling=FallDuration>0.f;
 if (bFalling) { SetActorTickEnabled(true); }
 else { Visual->SetWorldTransform(FallEnd); }
}
void ASMAGFDeathVisual::BeginDissolve()
{
 if (!bRetained || DissolveDuration<=0.f || IsActorBeingDestroyed()) { return; }
 // Very short configured lifetimes still finish the terminal pose before dissolving.
 bFalling=false; Visual->SetWorldTransform(FallEnd);
 for (int32 I=0; I<Visual->GetNumMaterials(); ++I)
 {
  if (auto* MID=UMaterialInstanceDynamic::Create(DissolveTemplate,this))
  { MID->SetScalarParameterValue(TEXT("Dissolve"),0.f); DissolveMaterials.Add(MID); Visual->SetMaterial(I,MID); }
 }
 DissolveStartedAt=GetWorld()->GetTimeSeconds(); bDissolving=true; SetActorTickEnabled(true);
}
void ASMAGFDeathVisual::SettingsChanged()
{
 if (!Showcase.IsValid()) { return; }
 const auto Feature=bRetained?ESMAGFShowcaseFeature::KeepEnemyBodies:ESMAGFShowcaseFeature::EnemyDeathReaction;
 if (!Showcase->IsFeatureEffectivelyEnabled(Feature)) { Retire(); }
}
void ASMAGFDeathVisual::Tick(float Dt)
{
 Super::Tick(Dt);
 if (bFalling)
 {
  FallElapsed+=Dt;
  const float Alpha=FMath::Clamp(FallElapsed/FallDuration,0.f,1.f);
  FTransform Pose; Pose.Blend(FallStart,FallEnd,Alpha); Visual->SetWorldTransform(Pose);
  if (Alpha>=1.f) { bFalling=false; Visual->SetWorldTransform(FallEnd); }
 }
 if (bDissolving)
 {
  const float Alpha=FMath::Clamp((GetWorld()->GetTimeSeconds()-DissolveStartedAt)/DissolveDuration,0.f,1.f);
  for (UMaterialInstanceDynamic* MID : DissolveMaterials) { MID->SetScalarParameterValue(TEXT("Dissolve"),Alpha); }
  if (Alpha>=1.f) { Retire(); return; }
 }
 SetActorTickEnabled(bFalling || bDissolving);
}
void ASMAGFDeathVisual::Retire()
{
 for (UMaterialInstanceDynamic* MID : DissolveMaterials) { MID->SetScalarParameterValue(TEXT("Dissolve"),1.f); }
 SetActorHiddenInGame(true); Destroy();
}
void ASMAGFDeathVisual::EndPlay(const EEndPlayReason::Type Reason)
{
 GetWorldTimerManager().ClearTimer(DissolveTimer); GetWorldTimerManager().ClearTimer(RetirementTimer);
 SetActorTickEnabled(false);
 if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(SettingsHandle); }
 // Decal actors are world-owned; their components are attached to this visual.
 const auto AttachedComponents=Visual->GetAttachChildren();
 for (USceneComponent* Child : AttachedComponents)
 {
  if (Child && Child->GetOwner()!=this)
  { if (auto* Evidence=Cast<ASMAGFPresentationVisual>(Child->GetOwner())) { Evidence->Destroy(); } }
 }
 DissolveMaterials.Empty();
 Super::EndPlay(Reason);
}
