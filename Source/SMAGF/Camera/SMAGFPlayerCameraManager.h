#pragma once
#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "SMAGFPlayerCameraManager.generated.h"

/** Final presentation authority. Logical deprojection always uses the unmodified pawn POV. */
UCLASS()
class SMAGF_API ASMAGFPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
public:
	FMinimalViewInfo GetFirstPersonBaseView() const;
	void PerspectiveChanged();
	virtual void ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot) override;
	UPROPERTY(EditDefaultsOnly, Category="SMAGF|Perspective", meta=(ClampMin="5", ClampMax="170"))
	float FirstPersonFOV = 90.f;
	static constexpr float FirstPersonPitchLimit = 85.f;
	UFUNCTION(BlueprintCallable, Category="SMAGF|Camera")
	void NotifyShot(FVector Direction);
	UFUNCTION(BlueprintCallable, Category="SMAGF|Camera")
	void NotifyKill(FVector Position);
	UFUNCTION(BlueprintPure, Category="SMAGF|Camera")
	bool DeprojectLogicalScreen(FVector2D Screen, FVector& Origin, FVector& Direction) const;
	/** Explicit viewport size for deterministic headless validation and offscreen tools. */
	UFUNCTION(BlueprintPure, Category="SMAGF|Camera")
	bool DeprojectLogicalViewport(FVector2D Screen, FIntPoint ViewportSize, FVector& Origin, FVector& Direction) const;
	UFUNCTION(BlueprintPure, Category="SMAGF|Camera")
	FMinimalViewInfo GetLogicalView() const { return LogicalPOV; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Camera")
	bool ProjectWorldToLogicalScreen(FVector WorldPosition, FVector2D& Screen) const;
	UFUNCTION(BlueprintPure, Category="SMAGF|Camera")
	bool ProjectWorldToLogicalViewport(FVector WorldPosition, FIntPoint ViewportSize, FVector2D& Screen) const;
	/** Read-only diagnostics for the current Focus-only post-process contribution. */
	UFUNCTION(BlueprintPure, Category="SMAGF|Camera|Diagnostics")
	FPostProcessSettings GetFocusPostProcess() const { return FocusPostProcess; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Camera|Diagnostics")
	float GetFocusPostProcessWeight() const { return FocusPostProcessWeight; }
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
protected:
	virtual void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV) override;
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;
private:
	FMinimalViewInfo LogicalPOV;
	TWeakObjectPtr<class ASMAGFShowcaseDirector> FocusDirector;
	FVector FocusOffset=FVector::ZeroVector;
	float FocusZoom=0.f;
	FPostProcessSettings FocusPostProcess;
	float FocusPostProcessWeight=0.f;
	FVector Follow = FVector::ZeroVector, Look = FVector::ZeroVector, ShotDirection = FVector::ForwardVector, KillPosition = FVector::ZeroVector;
	FSMAGFCameraSettings ShotSettings, KillSettings;
	float ShotGain = 0.f, KillGain = 0.f, ShotAge = 100.f, KillAge = 100.f;
	bool bHaveView = false;
	TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle SettingsHandle;
	void SettingsChanged();
	bool ProjectLogicalRect(FVector WorldPosition, const FIntRect& Rect, TEnumAsByte<EAspectRatioAxisConstraint> Constraint, FVector2D& Screen) const;
	bool DeprojectLogicalRect(FVector2D Screen, const FIntRect& Rect, TEnumAsByte<EAspectRatioAxisConstraint> Constraint, FVector& Origin, FVector& Direction) const;
};
