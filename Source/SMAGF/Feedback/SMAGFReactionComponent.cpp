#include "Feedback/SMAGFReactionComponent.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

USMAGFReactionComponent::USMAGFReactionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}
void USMAGFReactionComponent::BeginPlay()
{
	Super::BeginPlay();
	if (ACharacter* Character = Cast<ACharacter>(GetOwner()))
	{
		CharacterMovement = Character->GetCharacterMovement();
		BaselineAcceleration = CharacterMovement->MaxAcceleration;
		BaselineDeceleration = CharacterMovement->BrakingDecelerationWalking;
	}
	Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (Showcase.IsValid())
	{
		SettingsHandle = Showcase->OnSettingsChangedNative.AddUObject(this, &USMAGFReactionComponent::SettingsChanged);
		SettingsChanged();
	}
}
void USMAGFReactionComponent::SetVisualComponent(USceneComponent* Component)
{
	Visual = Component;
	if (Visual.IsValid()) { BaseVisual = Visual->GetRelativeTransform(); }
}
void USMAGFReactionComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(SettingsHandle); }
	if (Visual.IsValid()) { Visual->SetRelativeTransform(BaseVisual); }
	if (CharacterMovement.IsValid())
	{
		CharacterMovement->MaxAcceleration = BaselineAcceleration;
		CharacterMovement->BrakingDecelerationWalking = BaselineDeceleration;
	}
	Super::EndPlay(Reason);
}
void USMAGFReactionComponent::SettingsChanged()
{
	const FSMAGFShowcaseSettings Settings = Showcase->GetSettings();
	Tuning = Settings.Feeling;
	bMasterEnabled = Settings.Master.bShowcaseEnabled;
	Intensity = Settings.Master.MasterIntensity;
	if (CharacterMovement.IsValid())
	{
		CharacterMovement->MaxAcceleration = Enabled(Tuning.bMovementAccelerationEnabled) ? Tuning.MovementAcceleration : BaselineAcceleration;
		CharacterMovement->BrakingDecelerationWalking = Enabled(Tuning.bMovementAccelerationEnabled) ? Tuning.MovementDeceleration : BaselineDeceleration;
	}
	if (!Enabled(Tuning.bFireAnticipationEnabled)) { PreparationTime = 0.f; }
	if (!Enabled(Tuning.bRecoilEnabled)) { Recoil = FPulse(); }
	if (!Enabled(Tuning.bImpactResponseEnabled)) { Impact = FPulse(); }
	if (!Enabled(Tuning.bHitReactionEnabled)) { HitReaction = FPulse(); }
	if (!Enabled(CharacterMovement.IsValid() ? Tuning.bPlayerKnockbackEnabled : Tuning.bEnemyKnockbackEnabled))
	{
		KnockRemaining = 0.f; KnockVelocity = FVector::ZeroVector;
	}
	ComposeVisual();
}
float USMAGFReactionComponent::FPulse::Weight() const
{
	if (Elapsed >= Attack + Recovery) { return 0.f; }
	if (Attack > 0.f && Elapsed < Attack) { return Elapsed / Attack; }
	return Recovery > 0.f ? FMath::Clamp(1.f - (Elapsed - Attack) / Recovery, 0.f, 1.f) : 0.f;
}
void USMAGFReactionComponent::FPulse::Advance(float Delta)
{
	Elapsed += Delta;
	if (Elapsed >= Attack + Recovery) { *this = FPulse(); }
}
FVector USMAGFReactionComponent::ToVisualParent(const FVector& Direction) const
{
	if (Visual.IsValid() && Visual->GetAttachParent())
	{
		return Visual->GetAttachParent()->GetComponentTransform().InverseTransformVectorNoScale(Direction);
	}
	return Direction;
}
void USMAGFReactionComponent::BeginAnticipation(float Duration)
{
	PreparationTime = Enabled(Tuning.bFireAnticipationEnabled) ? Duration : 0.f;
	PreparationElapsed = 0.f;
}
void USMAGFReactionComponent::CancelAnticipation()
{
	PreparationTime = 0.f;
	ComposeVisual();
}
void USMAGFReactionComponent::OnShot(const FVector& Direction)
{
	CancelAnticipation();
	if (Direction.ContainsNaN()) { return; }
	const FVector Back = -Direction.GetSafeNormal2D();
	if (Enabled(Tuning.bRecoilEnabled))
	{
		// Refresh one bounded pulse; never stack independent timelines or accumulated transforms.
		Recoil = FPulse();
		Recoil.Offset = ToVisualParent(Back) * Tuning.RecoilDistance * Intensity;
		Recoil.Rotation = FRotator(Tuning.RecoilRotation * Intensity, 0.f, 0.f);
		Recoil.Attack = Tuning.RecoilDuration;
		Recoil.Recovery = Tuning.RecoilRecoveryDuration;
	}
	if (Enabled(Tuning.bPlayerKnockbackEnabled)) { StartKnockback(Back, Tuning.PlayerKnockbackStrength, Tuning.PlayerKnockbackRecovery); }
	ComposeVisual();
}
void USMAGFReactionComponent::OnDamagingHit(const FVector& Direction)
{
	if (Direction.ContainsNaN()) { return; }
	const FVector Forward = Direction.GetSafeNormal2D();
	if (Enabled(Tuning.bImpactResponseEnabled))
	{
		Impact = FPulse();
		Impact.Offset = ToVisualParent(Forward) * Tuning.ImpactResponseDistance * Intensity;
		Impact.Recovery = Tuning.ImpactResponseDuration;
	}
	if (Enabled(Tuning.bHitReactionEnabled))
	{
		HitReaction = FPulse();
		HitReaction.Offset = ToVisualParent(Forward) * (6.f * Tuning.HitReactionIntensity * Intensity);
		HitReaction.Rotation = FRotator(3.f * Tuning.HitReactionIntensity * Intensity, 0.f, 0.f);
		HitReaction.Recovery = Tuning.HitReactionDuration;
	}
	if (Enabled(Tuning.bEnemyKnockbackEnabled)) { StartKnockback(Forward, Tuning.EnemyKnockbackStrength, Tuning.EnemyKnockbackDuration); }
	ComposeVisual();
}
void USMAGFReactionComponent::StartKnockback(const FVector& Direction, float Strength, float Duration)
{
	KnockVelocity = Direction.GetSafeNormal2D() * Strength * Intensity;
	KnockDuration = Duration;
	KnockRemaining = Duration;
}
void USMAGFReactionComponent::ComposeVisual()
{
	if (!Visual.IsValid()) { return; }
	const float R = Recoil.Weight(), I = Impact.Weight(), H = HitReaction.Weight();
	FVector Offset = Recoil.Offset * R + Impact.Offset * I + HitReaction.Offset * H;
	if (PreparationTime > 0.f)
	{
		Offset += ToVisualParent(-GetOwner()->GetActorForwardVector()) * (10.f * Tuning.AnticipationScale
			* FMath::Clamp(PreparationElapsed / PreparationTime, 0.f, 1.f));
	}
	const FRotator Rotation = Recoil.Rotation * R + Impact.Rotation * I + HitReaction.Rotation * H;
	if (Offset.IsNearlyZero() && Rotation.IsNearlyZero()) { Visual->SetRelativeTransform(BaseVisual); return; }
	FTransform Result = BaseVisual;
	Result.SetLocation(BaseVisual.GetLocation() + Offset);
	Result.SetRotation(Rotation.Quaternion() * BaseVisual.GetRotation());
	Visual->SetRelativeTransform(Result);
}
void USMAGFReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	Super::TickComponent(DeltaTime, TickType, TickFunction);
	if (GetWorld()->IsPaused()) { return; }
	Recoil.Advance(DeltaTime); Impact.Advance(DeltaTime); HitReaction.Advance(DeltaTime);
	PreparationElapsed += DeltaTime;
	if (KnockRemaining > 0.f && KnockDuration > 0.f)
	{
		const float Step = FMath::Min(DeltaTime, KnockRemaining);
		const float Weight = (KnockRemaining - Step * 0.5f) / KnockDuration;
		const FVector Delta = KnockVelocity * Step * Weight;
		FHitResult Hit;
		if (CharacterMovement.IsValid())
		{
			CharacterMovement->SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorQuat(), true, Hit);
		}
		else { GetOwner()->AddActorWorldOffset(Delta, true, &Hit); }
		KnockRemaining = Hit.IsValidBlockingHit() ? 0.f : KnockRemaining - Step;
	}
	ComposeVisual();
}
