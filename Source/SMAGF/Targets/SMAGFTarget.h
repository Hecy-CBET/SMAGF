#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "SMAGFTarget.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class USMAGFHealthComponent;
class USMAGFReactionComponent;

UENUM(BlueprintType)
enum class ESMAGFEnemyTelegraphPresentationMode : uint8 { Legacy, ChargeOnly };

UCLASS()
class SMAGF_API ASMAGFTarget : public APawn
{
	GENERATED_BODY()
public:

	// Author these on the target Blueprint Class Defaults, not its component template.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
	bool bFrontMarkerEnabled = true;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
	FLinearColor EnemyProjectileEmissiveColor = FLinearColor(0.f, .7f, 1.f, 1.f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation", meta=(ClampMin="0", ClampMax="100"))
	float EnemyProjectileEmissiveStrength = 4.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
	ESMAGFEnemyTelegraphPresentationMode EnemyTelegraphPresentationMode = ESMAGFEnemyTelegraphPresentationMode::Legacy;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
	TObjectPtr<class UStaticMesh> EnemyTelegraphMesh;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
	FRotator EnemyTelegraphRelativeRotation = FRotator::ZeroRotator;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
	FVector EnemyTelegraphTargetScale = FVector(.08f, .45f, .3f);
	// Local position used only by ChargeOnly. Legacy retains its collision-derived offset.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation")
	FVector EnemyTelegraphLocalOffset = FVector(43.f, 0.f, 12.f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Enemy|Presentation", meta=(ClampMin="0", ClampMax="10", Units="s"))
	float EnemyTelegraphRetractDuration = .15f;
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
