#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TimerManager.h"
#include "SMAGFFlickeringComponent.generated.h"

class ULightComponent;

/** Owner-local light modulation. Does not own lights or change their visibility. */
UCLASS(ClassGroup=(SMAGF), meta=(BlueprintSpawnableComponent))
class SMAGF_API USMAGFFlickeringComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USMAGFFlickeringComponent();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, BlueprintSetter=SetFlickeringEnabled, Category="SMAGF|Flickering")
	bool bFlickeringEnabled = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0.01", Units="s"))
	float FlickerBurstDuration = 3.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0", Units="s"))
	float FlickerBurstDurationJitter = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0.01", Units="s"))
	float FlickerRestDuration = 12.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0", Units="s"))
	float FlickerRestDurationJitter = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0"))
	float FlickerMinIntensityMultiplier = .15f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0"))
	float FlickerMaxIntensityMultiplier = 1.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0.01", Units="s"))
	float FlickerStepMin = .04f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SMAGF|Flickering", meta=(ClampMin="0.01", Units="s"))
	float FlickerStepMax = .12f;

	/** Restores existing baselines before collecting; safe during a burst. */
	UFUNCTION(BlueprintCallable, Category="SMAGF|Flickering")
	void RefreshControlledLights();
	/** Restarts a burst using authored duration/jitter, even with automatic looping off. */
	UFUNCTION(BlueprintCallable, Category="SMAGF|Flickering")
	void TriggerFlickerBurst();
	UFUNCTION(BlueprintSetter, Category="SMAGF|Flickering")
	void SetFlickeringEnabled(bool bEnabled);
	/** Cancels automatic and manual flicker and restores all captured baselines. */
	UFUNCTION(BlueprintCallable, Category="SMAGF|Flickering")
	void StopFlickering();
	virtual void Activate(bool bReset = false) override;
	virtual void Deactivate() override;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void OnUnregister() override;

private:
	friend class FSMAGFHorrorFoundationTest;
	struct FControlledLight
	{
		TWeakObjectPtr<ULightComponent> Light;
		float Baseline = 0.f;
	};
	TArray<FControlledLight> Lights;
	FTimerHandle SequenceTimer;
	FRandomStream Random;
	double BurstEndTime = 0.;
	bool bBurstActive = false;
	bool bRuntimeReady = false;
	void CancelSequence();
	void RestoreLights();
	void ScheduleRest(bool bInitialPhase = false);
	void StepBurst();
	float RandomDuration(float Duration, float Jitter);
};
