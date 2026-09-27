#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMAGFTarget.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USMAGFHealthComponent;

UCLASS()
class SMAGF_API ASMAGFTarget : public AActor
{
	GENERATED_BODY()
public:
	ASMAGFTarget();
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	static FVector GetCollisionExtent() { return FVector(35.f, 35.f, 70.f); }
protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> Collision;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Visual;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USMAGFHealthComponent> Health;
private:
	UFUNCTION()
	void OnDied(USMAGFHealthComponent* DeadHealth);
};
