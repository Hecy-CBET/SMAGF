#include "Feedback/SMAGFReactionComponent.h"
#include "SMAGFGameMode.h"
#include "Feedback/SMAGFDeathVisual.h"
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
	AnimationTuning = Settings.Animation;
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
	if (!Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EnemyThreat))
	{ HostileHit=FPulse(); HostileRemaining=0.f; HostileVelocity=FVector::ZeroVector; }
	ClearDisabledMotion();
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
	if (AnimationEnabled(AnimationTuning.bFireAnticipationAnimationEnabled) && Duration > 0.f)
	{
		StartMotion(Preparation, FVector(0.f, 0.f, -3.f), FRotator(-6.f, 0.f, 0.f));
		Motion[Preparation].Attack = Duration;
		Motion[Preparation].Recovery = 0.f;
	}
}
void USMAGFReactionComponent::CancelAnticipation()
{
	PreparationTime = 0.f;
	Motion[Preparation] = SMAGFMotion::FChannel();
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
	if (AnimationEnabled(AnimationTuning.bWeaponRecoilAnimationEnabled))
	{
		// The baseline has no separate weapon mesh: show weapon force on its existing body visual.
		StartMotion(WeaponMotion, ToVisualParent(Back) * 4.f, FRotator(6.f, 0.f, 0.f));
	}
	if (AnimationEnabled(AnimationTuning.bCharacterFireReactionEnabled))
	{
		StartMotion(FireMotion, FVector(0.f, 0.f, 3.f), FRotator(0.f, 0.f, -4.f));
	}
	StartSecondaryMotion(false, Back);
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
	if (AnimationEnabled(AnimationTuning.bEnemyHitReactionEnabled))
	{
		StartMotion(HitMotion, ToVisualParent(Forward) * 8.f, FRotator(0.f, 0.f, 8.f));
	}
	StartSecondaryMotion(true, Forward);
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
	FVector Offset = Recoil.Offset * R + Impact.Offset * I + HitReaction.Offset * H + HostileHit.Offset * HostileHit.Weight();
	if (PreparationTime > 0.f)
	{
		Offset += ToVisualParent(-GetOwner()->GetActorForwardVector()) * (10.f * Tuning.AnticipationScale
			* FMath::Clamp(PreparationElapsed / PreparationTime, 0.f, 1.f));
	}
	FRotator Rotation = Recoil.Rotation * R + Impact.Rotation * I + HitReaction.Rotation * H + HostileHit.Rotation * HostileHit.Weight();
	FVector Scale = FVector::OneVector;
	FQuat AxisRotation = FQuat::Identity;
	AddAnimation(Offset, Rotation, Scale, AxisRotation);
	if (Offset.IsNearlyZero() && Rotation.IsNearlyZero() && Scale.Equals(FVector::OneVector) && AxisRotation.Equals(FQuat::Identity)) { Visual->SetRelativeTransform(BaseVisual); return; }
	FTransform Result = BaseVisual;
	Result.SetLocation(BaseVisual.GetLocation() + Offset);
	Result.SetRotation(AxisRotation * Rotation.Quaternion() * BaseVisual.GetRotation());
	Result.SetScale3D(BaseVisual.GetScale3D() * Scale);
	Visual->SetRelativeTransform(Result);
}
void USMAGFReactionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* TickFunction)
{
	Super::TickComponent(DeltaTime, TickType, TickFunction);
	if (GetWorld()->IsPaused()) { return; }
	const auto* Corpse = Cast<ASMAGFDeathVisual>(GetOwner());
	for (int32 I = 0; I < MotionCount; ++I)
	{
		if (I == DeathMotion && bDeathPresentation && Corpse && Corpse->RetainCorpse())
		{ Motion[I].Elapsed = FMath::Min(Motion[I].Elapsed + DeltaTime, Motion[I].Attack); }
		else { Motion[I].Advance(DeltaTime); }
	}
	// The sole DeathVisual owns the shared body/disappearance retirement deadline.
	if (bDeathPresentation && !IsAnimationActive() && !Corpse) { GetOwner()->Destroy(); return; }
	HostileHit.Advance(DeltaTime);
	if (HostileRemaining>0.f && CharacterMovement.IsValid())
	{
		const float Step=FMath::Min(DeltaTime,HostileRemaining);
		const FVector Delta=HostileVelocity*Step*((HostileRemaining-Step*.5f)/.25f);
		FHitResult Hit;CharacterMovement->SafeMoveUpdatedComponent(Delta,GetOwner()->GetActorQuat(),true,Hit);
		HostileRemaining=Hit.IsValidBlockingHit()?0.f:HostileRemaining-Step;
	}
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

void USMAGFReactionComponent::StartMotion(EMotion Channel, const FVector& Offset, const FRotator& Rotation, float Strength)
{
	SMAGFMotion::FChannel& State = Motion[Channel];
	State = SMAGFMotion::FChannel();
	const float Gain = Intensity * AnimationTuning.AnimationIntensity * Strength;
	State.Offset = Offset * Gain;
	State.Rotation = Rotation * Gain;
	State.Attack = AnimationTuning.ReactionDuration;
	State.Recovery = AnimationTuning.RecoveryDuration;
	State.Easing = AnimationTuning.Easing;
}

void USMAGFReactionComponent::StartSecondaryMotion(bool bImpact, const FVector& Direction)
{
	if (AnimationEnabled(AnimationTuning.bSquashAndStretchEnabled))
	{
		StartMotion(Deformation, FVector::ZeroVector, FRotator::ZeroRotator);
		const float Amount = bImpact ? -AnimationTuning.SquashAmount : AnimationTuning.StretchAmount;
		Motion[Deformation].ScaleZ = FMath::Clamp(1.f + Amount * Intensity * AnimationTuning.AnimationIntensity, 0.1f, 3.f);
	}
	if (AnimationEnabled(AnimationTuning.bOvershootRecoveryEnabled))
	{
		StartMotion(Settle, ToVisualParent(Direction) * (5.f * AnimationTuning.OvershootAmount), FRotator::ZeroRotator);
	}
	if (AnimationEnabled(AnimationTuning.bProceduralWobbleEnabled))
	{
		StartMotion(Wobble, FVector::ZeroVector, FRotator(0.f, 0.f, 15.f * AnimationTuning.WobbleAmount), bImpact ? 1.f : 0.5f);
		Motion[Wobble].Frequency = AnimationTuning.WobbleFrequency;
	}
}

void USMAGFReactionComponent::ClearDisabledMotion()
{
	const bool Selected[MotionCount] = {
		AnimationTuning.bFireAnticipationAnimationEnabled, AnimationTuning.bWeaponRecoilAnimationEnabled,
		AnimationTuning.bCharacterFireReactionEnabled, AnimationTuning.bEnemyHitReactionEnabled,
		AnimationTuning.bEnemyDeathReactionEnabled, AnimationTuning.bSquashAndStretchEnabled,
		AnimationTuning.bOvershootRecoveryEnabled, AnimationTuning.bProceduralWobbleEnabled };
	for (int32 Index = 0; Index < MotionCount; ++Index)
	{
		if (!AnimationEnabled(Selected[Index])) { Motion[Index] = SMAGFMotion::FChannel(); }
	}
	// Settings can change while paused: remove the dead presentation immediately,
	// then let its next game tick retire the actor without destroying inside a delegate.
	const auto* Corpse = Cast<ASMAGFDeathVisual>(GetOwner());
	if (bDeathPresentation && !AnimationEnabled(AnimationTuning.bEnemyDeathReactionEnabled) && !(Corpse && Corpse->RetainCorpse()))
	{
		GetOwner()->SetActorHiddenInGame(true);
	}
}

bool USMAGFReactionComponent::IsAnimationActive() const
{
	for (const SMAGFMotion::FChannel& Channel : Motion) { if (Channel.Active()) { return true; } }
	return false;
}

void USMAGFReactionComponent::BeginDeathPresentation(const FVector& IncomingDirection)
{
	bDeathPresentation = true;
	if (AnimationEnabled(AnimationTuning.bEnemyDeathReactionEnabled))
	{
		const FVector Fall = SMAGFMotion::PlanarDirection(IncomingDirection);
		DeathAxis = ToVisualParent(SMAGFMotion::FallAxis(Fall)).GetSafeNormal();
		DeathAngle = FMath::Clamp(65.f * Intensity * AnimationTuning.AnimationIntensity, 0.f, 85.f);
		StartMotion(DeathMotion, ToVisualParent(Fall * 4.f - FVector::UpVector * 10.f), FRotator::ZeroRotator);
	}
}

void USMAGFReactionComponent::AddAnimation(FVector& Offset, FRotator& Rotation, FVector& Scale, FQuat& AxisRotation) const
{
	FVector AnimationOffset = FVector::ZeroVector;
	FRotator AnimationRotation = FRotator::ZeroRotator;
	for (int32 Index = 0; Index < MotionCount; ++Index)
	{
		const SMAGFMotion::FChannel& State = Motion[Index];
		float Weight = State.Weight();
		if (Index == DeathMotion)
		{
			const auto* Corpse = Cast<ASMAGFDeathVisual>(GetOwner());
			if (bDeathPresentation && Corpse && Corpse->RetainCorpse())
			{ Weight = State.Attack > 0.f ? SMAGFMotion::Ease(FMath::Clamp(State.Elapsed / State.Attack, 0.f, 1.f), State.Easing) : 1.f; }
			AxisRotation = FQuat(DeathAxis, FMath::DegreesToRadians(FMath::Clamp(DeathAngle * Weight, -85.f, 85.f)));
		}
		if (Index == Settle && State.Active())
		{
			Weight *= FMath::Sin(2.f * PI * State.Elapsed / (State.Attack + State.Recovery));
		}
		if (Index == Wobble) { Weight *= FMath::Sin(2.f * PI * State.Frequency * State.Elapsed); }
		AnimationOffset += State.Offset * Weight;
		AnimationRotation += State.Rotation * Weight;
		if (Index == Deformation)
		{
			const float Z = FMath::Clamp(1.f + (State.ScaleZ - 1.f) * Weight, 0.1f, 3.f);
			const float XY = FMath::InvSqrt(Z);
			Scale = FVector(XY, XY, Z); // Unit volume multiplier; collision is never scaled.
		}
	}
	// Limit only this layer; accepted Phase 4 offsets are composed unchanged.
	Offset += AnimationOffset.GetClampedToMaxSize(60.f);
	Rotation += FRotator(FMath::Clamp(AnimationRotation.Pitch, -85.f, 85.f),
		FMath::Clamp(AnimationRotation.Yaw, -85.f, 85.f), FMath::Clamp(AnimationRotation.Roll, -85.f, 85.f));
}

void USMAGFReactionComponent::ReceiveShowcaseHostileHit(const FVector& Direction)
{
 if(!CharacterMovement.IsValid() || !Showcase.IsValid() || !Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EnemyThreat) || Direction.ContainsNaN()) { return; }
 ++HostileHitCount;
 if (auto* Mode=GetWorld()->GetAuthGameMode<ASMAGFGameMode>())
 { Mode->TriggerHorrorFlashlightFlicker(TEXT("HostileHit")); }
 const FVector Push=Direction.GetSafeNormal2D();
 // Refresh a bounded impulse rather than stacking velocities or disabling input.
 HostileVelocity=Push*800.f;HostileRemaining=.25f;
 HostileHit=FPulse();HostileHit.Offset=ToVisualParent(Push)*5.f*Showcase->GetMasterIntensity();
 HostileHit.Rotation=FRotator(4.f*Showcase->GetMasterIntensity(),0,0);HostileHit.Recovery=.2f;
 ComposeVisual();
}
