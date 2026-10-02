#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SMAGFTarget.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USMAGFHealthComponent;
class USMAGFReactionComponent;

UCLASS()
class SMAGF_API ASMAGFTarget : public APawn
{
	GENERATED_BODY()
public:
	ASMAGFTarget();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UFUNCTION(BlueprintPure, Category="SMAGF|Focus") bool IsAliveForFocus() const;
	UFUNCTION(BlueprintPure, Category="SMAGF|Focus") FVector GetFocusAnchor() const;
	virtual UPawnMovementComponent* GetMovementComponent() const override;
	virtual FVector GetNavAgentLocation() const override;
	virtual float TakeDamage(float DamageAmount, const FDamageEvent& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	static FVector GetCollisionExtent() { return FVector(35.f, 35.f, 70.f); }
protected:
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<class UFloatingPawnMovement> NavigationMovement;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USMAGFReactionComponent> Reaction;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UBoxComponent> Collision;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<UStaticMeshComponent> Visual;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components")
	TObjectPtr<USMAGFHealthComponent> Health;
private:
	FVector AcceptedDamageDirection = FVector::ForwardVector;
	UFUNCTION()
	void OnDied(USMAGFHealthComponent* DeadHealth);
};
