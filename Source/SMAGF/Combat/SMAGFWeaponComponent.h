#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SMAGFWeaponComponent.generated.h"

class ASMAGFProjectile;
class USMAGFShowcaseSubsystem;

UCLASS(ClassGroup=(SMAGF), meta=(BlueprintSpawnableComponent))
class SMAGF_API USMAGFWeaponComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USMAGFWeaponComponent();
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void StartFire();
	UFUNCTION(BlueprintCallable, Category="Weapon")
	void StopFire();
	UFUNCTION(BlueprintPure, Category="Weapon")
	bool IsAnticipating() const { return bAnticipating; }
	FVector GetMuzzleLocation() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	TSubclassOf<ASMAGFProjectile> ProjectileClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.01"))
	float FireInterval = 0.25f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="1.0"))
	float ProjectileSpeed = 2000.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon", meta=(ClampMin="0.0"))
	float ProjectileDamage = 25.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Weapon")
	FVector MuzzleOffset = FVector(65.f, 0.f, 20.f);
private:
	void TryFire();
	void FireShot();
	void CancelAnticipation();
	void SettingsChanged();
	TWeakObjectPtr<USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle SettingsHandle;
	bool bAnticipating = false;
	double AnticipationEnds = 0.0;
	bool bHeld = false;
	double NextShotTime = 0.0;
	uint64 LastShotFrame = MAX_uint64;
};
