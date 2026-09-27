#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SMAGFPlayerController.generated.h"

class UInputMappingContext;
class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;
struct FInputActionValue;

UCLASS()
class SMAGF_API ASMAGFPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ASMAGFPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	virtual void FlushPressedKeys() override;
	virtual bool ShouldFlushKeysWhenViewportFocusChanges() const override { return true; }
protected:
	virtual void SetupInputComponent() override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	void CreateInputObjects();
	void AddOwnedMapping();
	void RemoveOwnedMapping();
	void OnMove(const FInputActionValue& Value);
	void OnMoveReleased();
	void OnFireStarted();
	void OnFireReleased();
	void CancelHeldInput();
	void UpdateAim();

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
