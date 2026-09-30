#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMAGFDeathVisual.generated.h"

class UStaticMeshComponent;
class USMAGFReactionComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/** Single directional corpse; owns game-time fall, hold and dissolve. */
UCLASS(NotBlueprintable, Transient)
class SMAGF_API ASMAGFDeathVisual : public AActor
{
 GENERATED_BODY()
public:
 ASMAGFDeathVisual();
 void CopyVisual(const UStaticMeshComponent* Source, const FTransform& AuthoredTransform, const FVector& IncomingDirection);
 virtual void BeginPlay() override;
 virtual void Tick(float Dt) override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 UFUNCTION(BlueprintPure, Category="SMAGF|Presentation")
 float GetRetirementDeadline() const { return RetirementDeadline; }
 bool RetainCorpse() const { return bRetained; }
 UStaticMeshComponent* GetVisualComponent() const { return Visual; }
private:
 void SettingsChanged();
 void BeginDissolve();
 void Retire();
 void PrepareFall(float Duration);
 float RetirementDeadline = 0.f;
 float FallDuration = 0.f, FallElapsed = 0.f;
 float DissolveDuration = 0.f, DissolveStartedAt = 0.f;
 bool bRetained = false, bFalling = false, bDissolving = false;
 FTransform FallStart, FallEnd;
 FTimerHandle DissolveTimer, RetirementTimer;
 FVector DeathDirection = FVector::ForwardVector;
 TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
 FDelegateHandle SettingsHandle;
 UPROPERTY() TObjectPtr<UStaticMeshComponent> Visual;
 UPROPERTY() TObjectPtr<USMAGFReactionComponent> Reaction;
 UPROPERTY() TObjectPtr<UMaterialInterface> DissolveTemplate;
 UPROPERTY() TArray<TObjectPtr<UMaterialInstanceDynamic>> DissolveMaterials;
};
