#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "Containers/Ticker.h"
#include "SMAGFProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;
class USMAGFShowcaseSubsystem;
class UMaterialInterface;
class UMaterialInstanceDynamic;

UCLASS()
class SMAGF_API ASMAGFProjectile : public AActor
{
	GENERATED_BODY()
public:
	ASMAGFProjectile();
	void InitializeShot(const FVector& Direction, float Speed, float InDamage);
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UFUNCTION(BlueprintPure, Category="SMAGF|Projectile")
	bool IsImpactPresentationActive() const { return bConsumed && ImpactRemaining > 0.f; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Projectile")
	float GetVisualAge() const { return VisualAge; }
protected:
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USphereComponent> Collision;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Visual;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UProjectileMovementComponent> Movement;
private:
	TWeakObjectPtr<USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle SettingsHandle;
	FSMAGFFeelingSettings Feeling;
	FSMAGFAnimationSettings Animation;
	FSMAGFAudiovisualSettings Audiovisual;
	UPROPERTY() TObjectPtr<class UInstancedStaticMeshComponent> Trail;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> TrailMaterial;
	struct FTrailPoint { FVector Position; float Age; };
	TArray<FTrailPoint> TrailPoints;
	bool bTrail = false, bAudiovisualScale = false;
	float TrailClock = 0.f;
	float AudiovisualScale() const;
	void UpdateTrail(float Dt);
	bool bReadability = false, bAnimation = false;
	float MasterIntensity = 1.f;
	FTransform BaseVisual = FTransform::Identity;
	UPROPERTY() TObjectPtr<UMaterialInterface> BaseMaterial;
	UPROPERTY() TObjectPtr<UMaterialInterface> GlowMaterial;
	UPROPERTY() TObjectPtr<UMaterialInstanceDynamic> GlowInstance;
	float VisualAge = 0.f;
	float ImpactRemaining = 0.f;
	double ImpactDeadline = 0.;
	FTSTicker::FDelegateHandle ImpactTicker;
	FVector SurfacePoint = FVector::ZeroVector, SurfaceNormal = FVector::UpVector;
	FVector FlightDirection = FVector::ForwardVector;
	FVector ImpactScale = FVector::OneVector;
	FQuat ImpactRotation = FQuat::Identity;
	void SettingsChanged();
	void UpdateVisual(bool bCaptureFlight = false);
	UFUNCTION()
	void OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);
	float Damage = 25.f;
	FVector ShotDirection = FVector::ForwardVector;
	bool bConsumed = false;
};
