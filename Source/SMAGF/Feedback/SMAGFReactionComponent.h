#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Components/ActorComponent.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "Feedback/SMAGFMotion.h"
#include "SMAGFReactionComponent.generated.h"

class USceneComponent;
class USMAGFShowcaseSubsystem;
class UCharacterMovementComponent;

/** One composer for local presentation, plus a bounded swept planar impulse. */
UCLASS(ClassGroup=(SMAGF), meta=(BlueprintSpawnableComponent))
class SMAGF_API USMAGFReactionComponent : public UActorComponent
{
	GENERATED_BODY()
public:
	USMAGFReactionComponent();
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction) override;
	void SetVisualComponent(USceneComponent* Component);
	USceneComponent* GetVisualComponent() const { return Visual.Get(); }
	void BeginAnticipation(float Duration);
	void CancelAnticipation();
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	void OnShot(const FVector& Direction);
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	void OnDamagingHit(const FVector& Direction);
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	FTransform GetBaseVisualTransform() const { return BaseVisual; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	bool IsKnockbackActive() const { return KnockRemaining > 0.f; }
	UFUNCTION(BlueprintCallable, Category="SMAGF|Feedback")
	void ReceiveShowcaseHostileHit(const FVector& Direction);
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	int32 GetHostileHitCount() const { return HostileHitCount; }
	void BeginDeathPresentation(const FVector& IncomingDirection);
	UFUNCTION(BlueprintPure, Category="SMAGF|Feedback")
	bool IsAnimationActive() const;
private:
	enum EMotion { Preparation, WeaponMotion, FireMotion, HitMotion, DeathMotion, Deformation, Settle, Wobble, MotionCount };
	SMAGFMotion::FChannel Motion[MotionCount];
	FSMAGFAnimationSettings AnimationTuning;
	bool bDeathPresentation = false;
	FVector DeathAxis = FVector::RightVector;
	float DeathAngle = 0.f;
	bool AnimationEnabled(bool Selected) const { return bMasterEnabled && AnimationTuning.bEnabled && Selected; }
	void StartMotion(EMotion Channel, const FVector& Offset, const FRotator& Rotation, float Strength = 1.f);
	void StartSecondaryMotion(bool bImpact, const FVector& Direction);
	void ClearDisabledMotion();
	void AddAnimation(FVector& Offset, FRotator& Rotation, FVector& Scale, FQuat& AxisRotation) const;
	struct FPulse
	{
		FVector Offset = FVector::ZeroVector;
		FRotator Rotation = FRotator::ZeroRotator;
		float Elapsed = 0.f;
		float Attack = 0.f;
		float Recovery = 0.f;
		float Weight() const;
		void Advance(float Delta);
	};
	TWeakObjectPtr<USceneComponent> Visual;
	TWeakObjectPtr<UCharacterMovementComponent> CharacterMovement;
	TWeakObjectPtr<USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle SettingsHandle;
	FTransform BaseVisual = FTransform::Identity;
	FSMAGFFeelingSettings Tuning;
	bool bMasterEnabled = false;
	float Intensity = 1.f;
	float BaselineAcceleration = 0.f;
	float BaselineDeceleration = 0.f;
	FPulse Recoil, Impact, HitReaction, HostileHit;
	FVector HostileVelocity=FVector::ZeroVector;
	float HostileRemaining=0.f;
	int32 HostileHitCount=0;
	float PreparationTime = 0.f;
	float PreparationElapsed = 0.f;
	FVector KnockVelocity = FVector::ZeroVector;
	float KnockRemaining = 0.f;
	float KnockDuration = 0.f;
	bool Enabled(bool Selected) const { return bMasterEnabled && Tuning.bEnabled && Selected; }
	void SettingsChanged();
	void ComposeVisual();
	FVector ToVisualParent(const FVector& Direction) const;
	void StartKnockback(const FVector& Direction, float Strength, float Duration);
};
