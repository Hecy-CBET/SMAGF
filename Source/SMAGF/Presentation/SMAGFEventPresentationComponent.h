#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SMAGFEventPresentationComponent.generated.h"

/** Local shot/target presentation, driven exclusively by accepted gameplay events. */
UCLASS(ClassGroup=(SMAGF))
class SMAGF_API USMAGFEventPresentationComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USMAGFEventPresentationComponent();
	void Shot(FVector Muzzle, FVector Direction);
	void Hit(FVector Position, FVector Direction);
	void Death(FVector Position, FVector Direction);
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function) override;
	// Verified native defaults; individual asset overrides remain possible without a new audio system.
	UPROPERTY(EditDefaultsOnly, Category="SMAGF|Audio Assets") TObjectPtr<class USoundBase> FireSound;
	UPROPERTY(EditDefaultsOnly, Category="SMAGF|Audio Assets") TObjectPtr<class USoundBase> FireLayer;
	UPROPERTY(EditDefaultsOnly, Category="SMAGF|Audio Assets") TObjectPtr<class USoundBase> ImpactSound;
private:
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> Visual;
	UPROPERTY() TArray<TObjectPtr<class UMaterialInterface>> OriginalMaterials;
	UPROPERTY() TObjectPtr<class UMaterialInterface> Glow;
	UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> Flash;
	TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle Handle;
	float FlashRemaining = 0.f;
	uint32 ShotSequence = 0;
	void RestoreFlash();
	void SettingsChanged();
};
