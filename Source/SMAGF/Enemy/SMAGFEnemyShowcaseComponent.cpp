#include "Enemy/SMAGFEnemyShowcaseComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SMAGF.h"
#include "Enemy/SMAGFEnemyProjectile.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "Kismet/GameplayStatics.h"
USMAGFEnemyShowcaseComponent::USMAGFEnemyShowcaseComponent()
{ PrimaryComponentTick.bCanEverTick=true;PrimaryComponentTick.bStartWithTickEnabled=false; }
bool USMAGFEnemyShowcaseComponent::Enabled() const
{ return !bDead && Showcase.IsValid() && Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EnemyThreat); }
void USMAGFEnemyShowcaseComponent::BeginPlay()
{
 Super::BeginPlay();BaseRotation=GetOwner()->GetActorQuat();
 if(auto* Root=Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent())) { FrontOffset=Root->Bounds.BoxExtent.X+8.f; }
 Marker=NewObject<UStaticMeshComponent>(GetOwner(),TEXT("EnemyFrontMarker"));GetOwner()->AddInstanceComponent(Marker);
 Marker->SetupAttachment(GetOwner()->GetRootComponent());
 Marker->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube.Cube")));
 Marker->SetCollisionEnabled(ECollisionEnabled::NoCollision);Marker->SetCanEverAffectNavigation(false);Marker->SetReceivesDecals(false);
 Marker->SetRelativeLocation(FVector(FrontOffset,0,12));Marker->SetRelativeScale3D(FVector(.08f,.45f,.3f));Marker->RegisterComponent();
 MarkerMaterial=UMaterialInstanceDynamic::Create(LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/Variant_TwinStick/M_Glow.M_Glow")),this);
 Marker->SetMaterial(0,MarkerMaterial);
 Showcase=GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
 if(Showcase.IsValid()) { Handle=Showcase->OnSettingsChangedNative.AddUObject(this,&ThisClass::SettingsChanged); }
 SettingsChanged();
}
bool USMAGFEnemyShowcaseComponent::MovementEnabled() const
{ return Enabled() && Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EnemyMovement); }
void USMAGFEnemyShowcaseComponent::StopNavigation(bool bImmediate)
{
 bBraking=false;
 if(auto* Pawn=Cast<APawn>(GetOwner()))
 {
  if(auto* Controller=Cast<AAIController>(Pawn->GetController()))
  {
   if(bImmediate) { Controller->StopMovement(); }
   else if(auto* Following=Controller->GetPathFollowingComponent())
   { Following->AbortMove(*this,FPathFollowingResultFlags::OwnerFinished,FAIRequestID::CurrentRequest,EPathFollowingVelocityMode::Keep); }
  }
  if(auto* Movement=Cast<UFloatingPawnMovement>(Pawn->GetMovementComponent()))
  { Movement->ConsumeInputVector();if(bImmediate) { Movement->StopMovementImmediately(); } Movement->SetComponentTickEnabled(!bImmediate); }
 }
 RetryRemaining=0.f;
}
void USMAGFEnemyShowcaseComponent::BeginBraking()
{
 StopNavigation(false);bBraking=true;Elapsed=0.f;RecheckRemaining=0.f;
}
void USMAGFEnemyShowcaseComponent::ApplyMovementTuning()
{
 auto* Pawn=Cast<APawn>(GetOwner());
 auto* Movement=Pawn?Cast<UFloatingPawnMovement>(Pawn->GetMovementComponent()):nullptr;
 if(!Movement || !Showcase.IsValid()) { return; }
 const auto T=Showcase->GetSettings().SmokeAndMirrors;
 Movement->Acceleration=T.EnemyMoveAcceleration;Movement->Deceleration=T.EnemyMoveDeceleration;
 Movement->GetNavMovementProperties()->bUseAccelerationForPaths=true;
 // FloatingPawnMovement has no deceleration-derived automatic braking distance.
 Movement->SetFixedBrakingDistance(FMath::Max(125.f,FMath::Square(Movement->MaxSpeed)/(2.f*Movement->Deceleration)));
}
void USMAGFEnemyShowcaseComponent::AdjustPursuitInput(UPathFollowingComponent*,FVector& Input)
{
 if(!MovementEnabled() || bBraking) { Input=FVector::ZeroVector;return; }
 auto* Player=UGameplayStatics::GetPlayerPawn(this,0);
 auto* Pawn=Cast<APawn>(GetOwner());
 auto* Movement=Pawn?Cast<UFloatingPawnMovement>(Pawn->GetMovementComponent()):nullptr;
 if(!Player || !Movement) { return; }
 // The engine brakes relative to the player endpoint. Our stationary boundary is
 // 500 cm earlier. Limit analog path input there; the engine still integrates
 // acceleration/deceleration and steers along the full XYZ navigation path.
 const float Remaining=FVector::Dist2D(Player->GetActorLocation(),GetOwner()->GetActorLocation())
  -Showcase->GetSettings().SmokeAndMirrors.EnemyAttackStartRange;
 const float BrakingDistance=FMath::Max(125.f,FMath::Square(Movement->MaxSpeed)/(2.f*Movement->Deceleration));
 Input=Input.GetClampedToMaxSize(FMath::Clamp(Remaining/BrakingDistance,0.f,1.f));
}
void USMAGFEnemyShowcaseComponent::ResetCycle()
{ StopNavigation();State=ESMAGFEnemyState::Idle;Elapsed=0.f;RecheckRemaining=0.f; }
bool USMAGFEnemyShowcaseComponent::InitializeMovement()
{
 if(bSpeedInitialized) { return true; }
 auto* Player=Cast<ACharacter>(UGameplayStatics::GetPlayerPawn(this,0));
 auto* Pawn=Cast<APawn>(GetOwner());
 auto* Movement=Pawn?Cast<UFloatingPawnMovement>(Pawn->GetMovementComponent()):nullptr;
 if(!Player || !Player->GetCharacterMovement() || !Movement) { return false; }
 const auto T=Showcase->GetSettings().SmokeAndMirrors;
 const uint32 Seed=Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::DeterministicRandomSeed)
  ?uint32(T.RandomSeed):FGuid::NewGuid().A;
 const uint32 PerEnemy=Seed^(uint32(SpawnIndex)*0x9e3779b9u);
 FRandomStream SpeedRandom(int32(PerEnemy^0x53a9b4d1u));
 RecheckRandom.Initialize(int32(PerEnemy^0x17c3e529u));
 PlayerSpeedReference=Player->GetCharacterMovement()->MaxWalkSpeed;
 SpeedDeviation=SpeedRandom.FRandRange(-T.EnemyMoveSpeedDeviation,T.EnemyMoveSpeedDeviation);
 StoredSpeed=PlayerSpeedReference*T.EnemyMoveSpeedRatio*(1.f+SpeedDeviation);
 Movement->MaxSpeed=StoredSpeed;ApplyMovementTuning();bSpeedInitialized=true;return true;
}
void USMAGFEnemyShowcaseComponent::ScheduleRecheck()
{
 const auto T=Showcase->GetSettings().SmokeAndMirrors;
 RecheckRemaining=FMath::Max(.05f,T.EnemyRangeRecheckInterval+RecheckRandom.FRandRange(-T.EnemyRangeRecheckJitter,T.EnemyRangeRecheckJitter));
 if(RecheckDelays.Num()==16) { RecheckDelays.RemoveAt(0); }
 RecheckDelays.Add(RecheckRemaining);
}
void USMAGFEnemyShowcaseComponent::EnterAttack()
{
 StopNavigation();State=ESMAGFEnemyState::AttackDelay;Elapsed=0.f;
 if(MovementEnabled()) { ScheduleRecheck(); }
}
void USMAGFEnemyShowcaseComponent::EnterPursuit()
{
 StopNavigation();State=ESMAGFEnemyState::Pursuit;Elapsed=0.f;RecheckRemaining=0.f;
}
void USMAGFEnemyShowcaseComponent::UpdatePursuit(APawn* Player,float Dt)
{
 auto* Pawn=Cast<APawn>(GetOwner());auto* Controller=Pawn?Cast<AAIController>(Pawn->GetController()):nullptr;
 if(Controller && Controller->GetMoveStatus()!=EPathFollowingStatus::Idle) { return; }
 RetryRemaining-=Dt;if(RetryRemaining>0.f) { return; }RetryRemaining=.5f;
 auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
 if(!Controller || !Nav || !Nav->GetDefaultNavDataInstance(FNavigationSystem::DontCreate))
 { UE_LOG(LogSMAGF,Verbose,TEXT("Enemy navigation unavailable; bounded retry"));return; }
 auto* Movement=Cast<UFloatingPawnMovement>(Pawn->GetMovementComponent());
 if(!Movement) { return; }
 Controller->GetPathFollowingComponent()->PostProcessMove.BindUObject(this,&ThisClass::AdjustPursuitInput);
 Controller->GetPathFollowingComponent()->SetStopMovementOnFinish(true);
 Movement->SetComponentTickEnabled(true);
 // Disable reach-radius expansion so the gameplay 500 cm threshold is actually reached.
 FAIMoveRequest Request(Player);Request.SetUsePathfinding(true);Request.SetAllowPartialPath(false);
 Request.SetAcceptanceRadius(FMath::Max(0.f,Showcase->GetSettings().SmokeAndMirrors.EnemyAttackStartRange-2.f));
 Request.SetReachTestIncludesAgentRadius(false);Request.SetReachTestIncludesGoalRadius(false);Request.SetCanStrafe(true);
 ++MoveRequestCount;
 if(Controller->MoveTo(Request).Code==EPathFollowingRequestResult::Failed)
 { Movement->StopMovementImmediately();Movement->SetComponentTickEnabled(false);UE_LOG(LogSMAGF,Verbose,TEXT("Enemy MoveTo failed; retry in 0.5 seconds")); }
}
void USMAGFEnemyShowcaseComponent::SettingsChanged()
{
 ApplyMovementTuning();
 const bool Active=Enabled(),Moving=MovementEnabled();SetComponentTickEnabled(Active);
 if(Marker) { Marker->SetVisibility(Active); }
 if(Active && !Moving && bMovementWasEnabled) { BeginBraking();State=ESMAGFEnemyState::Idle; }
 else if(!Active || Moving!=bMovementWasEnabled) { ResetCycle(); }
 if(!Active) { GetOwner()->SetActorRotation(BaseRotation); }
 bMovementWasEnabled=Moving;
}
void USMAGFEnemyShowcaseComponent::StopForDeath()
{
 bDead=true;ResetCycle();SetComponentTickEnabled(false);if(Marker) { Marker->SetVisibility(false); }
 if(auto* Pawn=Cast<APawn>(GetOwner()))
 { if(auto* Controller=Cast<AAIController>(Pawn->GetController())) { Controller->UnPossess();Controller->Destroy(); } }
}
void USMAGFEnemyShowcaseComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function)
{
 Super::TickComponent(Dt,Type,Function);
 if(!Enabled() || GetWorld()->IsPaused()) { return; }
 APawn* Player=UGameplayStatics::GetPlayerPawn(this,0);const auto S=Showcase->GetSettings();const auto T=S.SmokeAndMirrors;
 const FVector Delta=Player?Player->GetActorLocation()-GetOwner()->GetActorLocation():FVector::ZeroVector;
 const float Distance=Delta.Size2D();
 const bool Moving=MovementEnabled();
 if(!Player) { ResetCycle();return; }
 if(Moving && !InitializeMovement()) { ResetCycle();return; }
 auto* Pawn=Cast<APawn>(GetOwner());
 auto* Movement=Pawn?Cast<UFloatingPawnMovement>(Pawn->GetMovementComponent()):nullptr;
 const float Speed=Movement?Movement->Velocity.Size():0.f;
 if(bBraking)
 {
  if(Speed>5.f) { return; }
  StopNavigation();State=ESMAGFEnemyState::Idle;
 }
 if(!Moving && Distance>T.EnemyAttackStartRange) { ResetCycle(); }
 else
 {
  if(!Delta.IsNearlyZero(0.01f))
  { const FRotator Aim(0.f,Delta.Rotation().Yaw,0.f);GetOwner()->SetActorRotation(FMath::RInterpConstantTo(GetOwner()->GetActorRotation(),Aim,Dt,180.f)); }
  if(State==ESMAGFEnemyState::Idle)
  { if(Moving && Distance>T.EnemyAttackStartRange) { EnterPursuit(); } else { EnterAttack(); } }
  if(State==ESMAGFEnemyState::Pursuit)
  {
   if(Distance<=T.EnemyAttackStartRange+2.f && Speed<=5.f) { EnterAttack(); }
   else if(Distance<=T.EnemyAttackStartRange) { BeginBraking(); }
   else { UpdatePursuit(Player,Dt); }
  }
  else if(Moving)
  {
   RecheckRemaining-=Dt;
   if(RecheckRemaining<=0.f)
   { if(Distance>T.EnemyEngagementRange) { EnterPursuit();UpdatePursuit(Player,Dt); } else { ScheduleRecheck(); } }
  }
  Elapsed+=Dt;
  if(State==ESMAGFEnemyState::AttackDelay && Elapsed>=.6f+SpawnIndex*.3f) { State=ESMAGFEnemyState::Telegraph;Elapsed=0.f; }
  else if(State==ESMAGFEnemyState::Telegraph && Elapsed>=T.EnemyTelegraphDuration)
  {
   const FVector Origin=GetOwner()->GetActorLocation()+GetOwner()->GetActorForwardVector()*FrontOffset;
   FActorSpawnParameters Params;Params.Owner=GetOwner();Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
   if(auto* Projectile=GetWorld()->SpawnActor<ASMAGFEnemyProjectile>(Origin,FRotator::ZeroRotator,Params))
   { Projectile->InitializeFlight(Player->GetActorLocation()-Origin,T.EnemyProjectileSpeed);++ShotsFired; }
   State=ESMAGFEnemyState::Cooldown;Elapsed=0.f;
  }
  else if(State==ESMAGFEnemyState::Cooldown && Elapsed>=T.EnemyAttackCooldown) { State=ESMAGFEnemyState::Telegraph;Elapsed=0.f; }
 }
 const float Pulse=State==ESMAGFEnemyState::Telegraph?FMath::Clamp(Elapsed/FMath::Max(T.EnemyTelegraphDuration,.001f),0.f,1.f):0.f;
 Marker->SetRelativeScale3D(FVector(.08f,.45f,.3f)*(1.f+.3f*Pulse*S.Master.MasterIntensity));
 if(MarkerMaterial) { MarkerMaterial->SetVectorParameterValue(TEXT("Emissive Color"),FLinearColor(0.f,.5f,1.f));MarkerMaterial->SetScalarParameterValue(TEXT("Emissive Strength"),(1.f+5.f*Pulse)*S.Master.MasterIntensity); }
}
void USMAGFEnemyShowcaseComponent::EndPlay(const EEndPlayReason::Type Reason)
{
 StopNavigation();
 if(auto* Pawn=Cast<APawn>(GetOwner()))
 { if(auto* Controller=Cast<AAIController>(Pawn->GetController()))
   { if(auto* Following=Controller->GetPathFollowingComponent())
     { if(Following->PostProcessMove.IsBoundToObject(this)) { Following->PostProcessMove.Unbind(); } } } }
 if(Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(Handle); }
 SetComponentTickEnabled(false);if(Marker) { Marker->DestroyComponent(); }Super::EndPlay(Reason);
}
