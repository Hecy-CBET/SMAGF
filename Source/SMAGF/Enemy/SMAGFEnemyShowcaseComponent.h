#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Enemy/SMAGFEnemyDebugMode.h"
#include "SMAGFEnemyShowcaseComponent.generated.h"
UENUM(BlueprintType)
enum class ESMAGFEnemyState : uint8 { Idle, Telegraph, Cooldown, Pursuit, AttackDelay, Hold };
UCLASS(ClassGroup=(SMAGF))
class SMAGF_API USMAGFEnemyShowcaseComponent : public UActorComponent
{
 GENERATED_BODY()
public:
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
 bool bFrontMarkerEnabled = true;
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
 FLinearColor EnemyProjectileEmissiveColor = FLinearColor(0.f, .7f, 1.f, 1.f);
 UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation", meta=(ClampMin="0", ClampMax="100"))
 float EnemyProjectileEmissiveStrength = 4.f;
 USMAGFEnemyShowcaseComponent();
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function) override;
 void StopForDeath();
 void ApplyDebugBehaviorMode(ESMAGFEnemyDebugMode NewMode);
 UFUNCTION(BlueprintPure) ESMAGFEnemyDebugMode GetDebugBehaviorMode() const { return DebugMode; }
 void SetSpawnIndex(int32 Value) { SpawnIndex=FMath::Max(0,Value); }
 UFUNCTION(BlueprintPure) ESMAGFEnemyState GetThreatState() const { return State; }
 UFUNCTION(BlueprintPure) int32 GetShotsFired() const { return ShotsFired; }
 UFUNCTION(BlueprintPure) float GetEnemyMoveSpeed() const { return StoredSpeed; }
 UFUNCTION(BlueprintPure) float GetSampledSpeedDeviation() const { return SpeedDeviation; }
 UFUNCTION(BlueprintPure) float GetPlayerSpeedReference() const { return PlayerSpeedReference; }
 UFUNCTION(BlueprintPure) int32 GetSpawnIndex() const { return SpawnIndex; }
 UFUNCTION(BlueprintPure) int32 GetMoveRequestCount() const { return MoveRequestCount; }
 UFUNCTION(BlueprintPure) float GetRecheckRemaining() const { return RecheckRemaining; }
 UFUNCTION(BlueprintPure) TArray<float> GetRecheckDelays() const { return RecheckDelays; }
private:
 void SettingsChanged();
 void ResetCycle();
 bool Enabled() const;
 bool MovementEnabled() const;
 bool OffenseEnabled() const;
 bool InitializeMovement();
 void StopNavigation(bool bImmediate = true);
 void BeginBraking();
 void ApplyMovementTuning();
 void AdjustPursuitInput(class UPathFollowingComponent* Following, FVector& Input);
 void EnterPursuit();
 void EnterAttack();
 void BeginTelegraph();
 void ScheduleRecheck();
 void UpdatePursuit(class APawn* Player,float Dt);
 bool bBraking=false;
 ESMAGFEnemyDebugMode DebugMode=ESMAGFEnemyDebugMode::Aggressive;
 bool bSpeedInitialized=false,bMovementWasEnabled=false,bDead=false;
 bool bNavSurfaceOffsetInitialized=false;
 bool bRecoveringNavProjection=false;
 float NavSurfaceOffset=0.f;
 float StoredSpeed=0.f,SpeedDeviation=0.f,PlayerSpeedReference=0.f;
 float RecheckRemaining=0.f,RetryRemaining=0.f;
 int32 MoveRequestCount=0;
 FRandomStream RecheckRandom;
 TArray<float> RecheckDelays;
 TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> Marker;
 UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> MarkerMaterial;
 FDelegateHandle Handle;
 FQuat BaseRotation;
 ESMAGFEnemyState State=ESMAGFEnemyState::Idle;
 int32 SpawnIndex=0,ShotsFired=0;
 float Elapsed=0.f,FrontOffset=50.f;
};
