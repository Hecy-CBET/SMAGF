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
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
protected:
	virtual void UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime) override;
private:
	FMinimalViewInfo LogicalPOV;
	FVector Follow = FVector::ZeroVector, Look = FVector::ZeroVector, ShotDirection = FVector::ForwardVector, KillPosition = FVector::ZeroVector;
	FSMAGFCameraSettings ShotSettings, KillSettings;
	float ShotGain = 0.f, KillGain = 0.f, ShotAge = 100.f, KillAge = 100.f;
	bool bHaveView = false;
	TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle SettingsHandle;
	void SettingsChanged();
	bool DeprojectLogicalRect(FVector2D Screen, const FIntRect& Rect, TEnumAsByte<EAspectRatioAxisConstraint> Constraint, FVector& Origin, FVector& Direction) const;
};
