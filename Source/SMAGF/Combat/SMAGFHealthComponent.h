#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SMAGFHealthComponent.generated.h"

class USMAGFHealthComponent;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FSMAGFHealthChanged, USMAGFHealthComponent*, Health, float, NewHealth, float, AppliedDamage);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSMAGFDeath, USMAGFHealthComponent*, Health);

UCLASS(ClassGroup=(SMAGF), meta=(BlueprintSpawnableComponent))
class SMAGF_API USMAGFHealthComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USMAGFHealthComponent();
	virtual void BeginPlay() override;
	float ApplyDamage(float Damage);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Health", meta=(ClampMin="1.0"))
	float MaxHealth = 100.f;
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Health")
	float CurrentHealth = 100.f;
	UPROPERTY(BlueprintAssignable, Category="Health")
	FSMAGFHealthChanged OnHealthChanged;
	UPROPERTY(BlueprintAssignable, Category="Health")
	FSMAGFDeath OnDeath;
private:
	bool bDead = false;
};
