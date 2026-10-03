#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Containers/Ticker.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "SMAGFFeedbackSubsystem.generated.h"

class USMAGFShowcaseSubsystem;

/** Owns global hit stop and narrow presentation pressure; their clocks remain separate. */
UCLASS()
class SMAGF_API USMAGFFeedbackSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(USMAGFFeedbackSubsystem, STATGROUP_Tickables); }
	virtual bool IsTickableWhenPaused() const override { return false; }
	/** One call at the consumed player-projectile contact result boundary. */
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	void NotifyContact(float AcceptedDamage, bool bKilled);
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	void NotifyPlayerHit();
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	float GetPresentationPressure() const { return PresentationPressure; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	float GetLastPresentationScale() const { return LastScale; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	ESMAGFEventImportance GetLastImportance() const { return LastImportance; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	int32 GetPresentationEventCount() const { return EventCount; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	float GetLastAuthoredHitStop() const { return LastAuthoredDuration; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	float GetLastResolvedHitStop() const { return LastResolvedDuration; }
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
	float PresentationPressure = 0.f;
	float LastScale = 1.f;
	float LastAuthoredDuration = 0.f;
	float LastResolvedDuration = 0.f;
	ESMAGFEventImportance LastImportance = ESMAGFEventImportance::Minor;
	int32 EventCount = 0;
	bool IsPressureEnabled() const;
	void ProcessPresentationEvent(ESMAGFEventImportance Importance, const TCHAR* Kind, bool bDamage, bool bKilled);
	bool TickRealTime(float DeltaTime);
	void SettingsChanged();
	void ApplyStrongest();
};
