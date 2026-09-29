#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMAGFDeathVisual.generated.h"

class UStaticMeshComponent;
class USMAGFReactionComponent;

/** Short lived presentation copy. No health, collision, gameplay target or persistent debris. */
UCLASS(NotBlueprintable, Transient)
class SMAGF_API ASMAGFDeathVisual : public AActor
{
	GENERATED_BODY()
public:
	ASMAGFDeathVisual();
	void CopyVisual(const UStaticMeshComponent* Source, const FTransform& AuthoredTransform, const FVector& IncomingDirection);
	virtual void BeginPlay() override;
private:
	FVector DeathDirection = FVector::ForwardVector;
	UPROPERTY() TObjectPtr<UStaticMeshComponent> Visual;
	UPROPERTY() TObjectPtr<USMAGFReactionComponent> Reaction;
};
