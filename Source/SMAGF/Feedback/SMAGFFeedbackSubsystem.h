#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Containers/Ticker.h"
#include "SMAGFFeedbackSubsystem.generated.h"

class USMAGFShowcaseSubsystem;

/** Owns only world-global Feeling hit stop. Expiration never uses gameplay timers. */
UCLASS()
class SMAGF_API USMAGFFeedbackSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	void NotifyDamagingHit(bool bKilled);
	/** Absolute time-scale request; strongest slowdown wins, never multiplied. */
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	int32 RequestHitStop(float Duration, float TimeScale);
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	int32 GetActiveRequestCount() const { return Requests.Num(); }
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	void ClearHitStop();
protected:
	virtual bool DoesSupportWorldType(EWorldType::Type Type) const override;
private:
	struct FRequest { int32 Id; double Expires; float Scale; };
	TArray<FRequest> Requests;
	TWeakObjectPtr<USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle SettingsHandle;
	FTSTicker::FDelegateHandle TickerHandle;
	int32 NextId = 0;
	float CapturedDilation = 1.f;
	bool bCaptured = false;
	bool TickRealTime(float DeltaTime);
	void SettingsChanged();
	void ApplyStrongest();
};
