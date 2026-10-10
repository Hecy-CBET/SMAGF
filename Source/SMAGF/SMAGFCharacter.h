#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SMAGFCharacter.generated.h"

class USceneComponent;
class UCameraComponent;
class USpringArmComponent;
class USMAGFWeaponComponent;
class USMAGFReactionComponent;

UCLASS()
class SMAGF_API ASMAGFCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	ASMAGFCharacter();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	/** Called once controller startup/possession has established the perspective. */
	void InitializeHorrorFlashlight();
	void UpdateFlashlightTransform(const FRotator& ViewRotation);
	AActor* GetHorrorFlashlight() const { return HorrorFlashlight.Get(); }
	void MoveInCameraPlane(const FVector2D& Intent);
	void ApplyAimDirection(const FVector& Direction);
	UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent.Get(); }
	USpringArmComponent* GetCameraBoom() const { return CameraBoom.Get(); }
	USMAGFWeaponComponent* GetWeapon() const { return Weapon.Get(); }
private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USceneComponent> FlashlightAnchor;
	UPROPERTY(Transient)
	TObjectPtr<AActor> HorrorFlashlight;
	bool bFlashlightInitializationAttempted = false;
	UPROPERTY(VisibleAnywhere, Category="Components")
	TObjectPtr<USMAGFReactionComponent> Reaction;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<UCameraComponent> TopDownCameraComponent;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USpringArmComponent> CameraBoom;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta=(AllowPrivateAccess="true"))
	TObjectPtr<USMAGFWeaponComponent> Weapon;
};
