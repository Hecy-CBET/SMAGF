#pragma once

#include "CoreMinimal.h"
#include "AudioDeviceHandle.h"
#include "GameFramework/PlayerController.h"
#include "Horror/SMAGFFlashlightMotion.h"
#include "SMAGFPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;
class SSMAGFShowcaseDebugPanel;
struct FInputActionValue;

UENUM(BlueprintType)
enum class ESMAGFPlayerPerspective : uint8 { TopDown, FirstPerson };

UCLASS()
class SMAGF_API ASMAGFPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
 UFUNCTION(BlueprintPure, Category="SMAGF|Debug")
 bool IsGameAudioEnabled() const { return bGameAudioEnabled; }
 UFUNCTION(BlueprintCallable, Category="SMAGF|Debug")
 void SetGameAudioEnabled(bool bEnabled);
 UFUNCTION(BlueprintCallable, Category="SMAGF|Debug")
 void ToggleGameAudio() { SetGameAudioEnabled(!bGameAudioEnabled); }
	UFUNCTION(BlueprintPure, Category="SMAGF|Perspective")
	ESMAGFPlayerPerspective GetPlayerPerspective() const { return Perspective; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Perspective")
	bool IsFirstPersonPerspective() const { return Perspective == ESMAGFPlayerPerspective::FirstPerson; }
	UFUNCTION(BlueprintCallable, Category="SMAGF|Perspective")
	void SetPlayerPerspective(ESMAGFPlayerPerspective NewPerspective);
	UFUNCTION(BlueprintCallable, Category="SMAGF|Perspective")
	void TogglePlayerPerspective();
	/** Stable eye/control-view trace, shared by aiming and the existing fire path. */
	UFUNCTION(BlueprintPure, Category="SMAGF|Perspective")
	FVector GetFirstPersonAimPoint() const;
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	void ToggleShowcasePanel();
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	void CloseShowcasePanel();
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	bool IsShowcasePanelOpen() const { return ShowcasePanel.IsValid(); }
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	FText GetShowcaseLayerLabel() const;
	bool HandleShowcaseKey(FKey Key);
	bool HandleShowcaseToggleKey(FKey Key, bool bControlDown, bool bIsRepeat);
	ASMAGFPlayerController();
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual void PlayerTick(float DeltaTime) override;
	virtual void FlushPressedKeys() override;
	virtual bool ShouldFlushKeysWhenViewportFocusChanges() const override { return true; }
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
 friend struct FSMAGFAudioToggleFixture;
 bool HandleGameAudioKey(FKey Key, EInputEvent Event, bool bControl, bool bExtraModifier);
 UPROPERTY(Transient) bool bGameAudioEnabled = true;
 // Previous multiplier is read/written only by ordered audio-thread commands.
 struct FAudioMuteState { float PreviousVolume = 1.f; };
 TSharedPtr<FAudioMuteState, ESPMode::ThreadSafe> AudioMuteState;
 FAudioDeviceHandle MutedAudioDevice;
	friend struct FSMAGFFlashlightFixture;
	UPROPERTY(Transient) ESMAGFPlayerPerspective Perspective = ESMAGFPlayerPerspective::TopDown;
	void ApplyPerspectiveInput();
	void LookYaw(float Value);
	void LookPitch(float Value);
	bool bSavedOwnerNoSee = false;
	static constexpr float FirstPersonAimDistance = 10000.f;
	TSharedPtr<class SSMAGFShowcaseLayerIndicator> LayerIndicator;
	void RemoveShowcasePanel(bool bRestoreInput);
	void PreviousShowcaseLayer();
	void NextShowcaseLayer();
	void BaselineShowcaseLayer();
	void FullShowcaseLayer();
	TSharedPtr<SSMAGFShowcaseDebugPanel> ShowcasePanel;
	bool bRequireFireRelease = false;
	bool bRequireMoveRelease = false;
	void CreateInputObjects();
	void AddOwnedMapping();
	void RemoveOwnedMapping();
	void OnMove(const FInputActionValue& Value);
	void OnMoveReleased();
	void OnFireStarted();
	void OnFireReleased();
	void CancelHeldInput();
	void UpdateAim(float DeltaTime = 0.f);
	void UpdateFlashlight(float DeltaTime = 0.f);
	FSMAGFFlashlightMotion FlashlightMotion;
	FVector LastTopDownAimPoint = FVector::ZeroVector;
	bool bHasTopDownAimPoint = false;

	UPROPERTY(Transient)
	TObjectPtr<UInputMappingContext> BaselineMapping;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> MoveAction;
	UPROPERTY(Transient)
	TObjectPtr<UInputAction> FireAction;
	UPROPERTY(Transient)
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> MappingSubsystem;
	FVector2D MoveIntent = FVector2D::ZeroVector;
	FVector LastAimDirection = FVector::ForwardVector;
};
