#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMAGFProjectile.generated.h"

class USphereComponent;
class UStaticMeshComponent;
class UProjectileMovementComponent;

UCLASS()
class SMAGF_API ASMAGFProjectile : public AActor
{
	GENERATED_BODY()
public:
	ASMAGFProjectile();
	void InitializeShot(const FVector& Direction, float Speed, float InDamage);
protected:
	virtual void BeginPlay() override;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USphereComponent> Collision;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Visual;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UProjectileMovementComponent> Movement;
private:
	UFUNCTION()
	void OnImpact(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComponent,
		FVector NormalImpulse, const FHitResult& Hit);
	float Damage = 25.f;
	FVector ShotDirection = FVector::ForwardVector;
	bool bConsumed = false;
};
