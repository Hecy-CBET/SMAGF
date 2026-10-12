#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMAGFEnemyProjectile.generated.h"
UCLASS(NotBlueprintable,Transient)
class SMAGF_API ASMAGFEnemyProjectile : public AActor
{
 GENERATED_BODY()
public:
 ASMAGFEnemyProjectile();
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
 void SetPresentation(const FLinearColor& EmissiveColor,float EmissiveStrength);
 void InitializeFlight(const FVector& Direction,float Speed);
private:
 void SettingsChanged();
 UFUNCTION() void OnImpact(UPrimitiveComponent* HitComponent,AActor* Other,UPrimitiveComponent* OtherComponent,FVector Impulse,const FHitResult& Hit);
 UPROPERTY() TObjectPtr<class USphereComponent> Collision;
 UPROPERTY() TObjectPtr<class UStaticMeshComponent> Visual;
 UPROPERTY() TObjectPtr<class UProjectileMovementComponent> Movement;
 UPROPERTY() TObjectPtr<class UMaterialInterface> Glow;
 UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> Material;
 TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
 FDelegateHandle Handle;
 FLinearColor PresentationEmissiveColor=FLinearColor(0.f,.7f,1.f,1.f);
 float PresentationEmissiveStrength=4.f;
 FVector FlightDirection=FVector::ForwardVector;
 bool bConsumed=false;
};
