#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SMAGFShowcaseDirector.generated.h"

class ASMAGFTarget;
class ASMAGFPlayerCameraManager;
class APlayerController;

/** Mouse-selected attention state. PlayerCameraManager remains the camera composer. */
UCLASS(NotPlaceable)
class SMAGF_API ASMAGFShowcaseDirector : public AActor
{
 GENERATED_BODY()
public:
 ASMAGFShowcaseDirector();
 static ASMAGFShowcaseDirector* Get(UWorld* World);
 void RegisterTarget(ASMAGFTarget* Target);
 void UnregisterTarget(ASMAGFTarget* Target);
 // Called after this frame's logical POV is captured, before presentation offsets.
 void UpdateMouseFocus(ASMAGFPlayerCameraManager* Camera, APlayerController* Controller, float DeltaSeconds);
 UFUNCTION(BlueprintPure) bool IsFocusActive() const { return FocusedTarget.IsValid(); }
 UFUNCTION(BlueprintPure) float GetFocusStrength() const { return Strength; }
 UFUNCTION(BlueprintPure) FVector GetFocusPoint() const { return FocusPoint; }
 UFUNCTION(BlueprintPure) ASMAGFTarget* GetFocusSubject() const;
 UFUNCTION(BlueprintPure) float GetFocusRemaining() const { return ReleaseRemaining; }
 UFUNCTION(BlueprintPure) bool IsReleasePending() const { return bReleasePending; }
 UFUNCTION(BlueprintPure) int32 GetSelectionChangeCount() const { return SelectionChanges; }
 UFUNCTION(BlueprintPure) float GetFocusScreenDistance() const { return CurrentDistance; }
 /** Editor/unattended validation seam only; normal gameplay always reads PlayerController mouse input. */
 UFUNCTION(BlueprintCallable, Category="SMAGF|Focus|Validation", meta=(DevelopmentOnly))
 bool SetValidationMouseInput(FVector2D Screen, FIntPoint ViewportSize, bool bAvailable);
 UFUNCTION(BlueprintCallable, Category="SMAGF|Focus|Validation", meta=(DevelopmentOnly))
 void ClearValidationMouseInput();
 virtual void BeginPlay() override;
 virtual void EndPlay(const EEndPlayReason::Type Reason) override;
private:
 UPROPERTY(VisibleAnywhere) TObjectPtr<class UPointLightComponent> FocusLight;
 TArray<TWeakObjectPtr<ASMAGFTarget>> Targets;
 TWeakObjectPtr<ASMAGFTarget> FocusedTarget;
 TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
 FDelegateHandle SettingsHandle;
 FVector FocusPoint=FVector::ZeroVector;
 float Strength=0.f, ReleaseRemaining=0.f, Age=0.f, CurrentDistance=-1.f;
 int32 SelectionChanges=0;
 bool bReleasePending=false, bInvalidRelease=false;
 bool bValidationInput=false, bValidationAvailable=false;
 FVector2D ValidationScreen=FVector2D::ZeroVector;
 FIntPoint ValidationSize=FIntPoint::ZeroValue;
 bool Enabled() const;
 void Select(ASMAGFTarget* Target, float Distance);
 void Release(const TCHAR* Reason, bool bInvalid=false);
 void SettingsChanged();
 void UpdateLight();
};
