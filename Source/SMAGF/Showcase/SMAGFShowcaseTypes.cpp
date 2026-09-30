#include "Showcase/SMAGFShowcaseTypes.h"

namespace
{
	float ValidFloat(float Value, float Min, float Max, float Fallback)
	{
		return FMath::IsFinite(Value) ? FMath::Clamp(Value, Min, Max) : Fallback;
	}
}

bool FSMAGFMasterSettings::operator==(const FSMAGFMasterSettings& Other) const
{
	return bShowcaseEnabled == Other.bShowcaseEnabled
		&& MasterIntensity == Other.MasterIntensity;
}

bool FSMAGFFeelingSettings::operator==(const FSMAGFFeelingSettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bFireAnticipationEnabled == Other.bFireAnticipationEnabled
		&& bMovementAccelerationEnabled == Other.bMovementAccelerationEnabled
		&& bRecoilEnabled == Other.bRecoilEnabled
		&& bImpactResponseEnabled == Other.bImpactResponseEnabled
		&& bHitStopEnabled == Other.bHitStopEnabled
		&& bEnemyKnockbackEnabled == Other.bEnemyKnockbackEnabled
		&& bPlayerKnockbackEnabled == Other.bPlayerKnockbackEnabled
		&& bHitReactionEnabled == Other.bHitReactionEnabled
		&& FireAnticipationTime == Other.FireAnticipationTime
		&& AnticipationScale == Other.AnticipationScale
		&& MovementAcceleration == Other.MovementAcceleration
		&& MovementDeceleration == Other.MovementDeceleration
		&& RecoilDistance == Other.RecoilDistance
		&& ImpactResponseDistance == Other.ImpactResponseDistance
		&& ImpactResponseDuration == Other.ImpactResponseDuration
		&& RecoilRotation == Other.RecoilRotation
		&& RecoilDuration == Other.RecoilDuration
		&& RecoilRecoveryDuration == Other.RecoilRecoveryDuration
		&& HitStopDuration == Other.HitStopDuration
		&& KillHitStopDuration == Other.KillHitStopDuration
		&& HitStopTimeScale == Other.HitStopTimeScale
		&& EnemyKnockbackStrength == Other.EnemyKnockbackStrength
		&& EnemyKnockbackDuration == Other.EnemyKnockbackDuration
		&& PlayerKnockbackStrength == Other.PlayerKnockbackStrength
		&& PlayerKnockbackRecovery == Other.PlayerKnockbackRecovery
		&& HitReactionDuration == Other.HitReactionDuration
		&& HitReactionIntensity == Other.HitReactionIntensity
		&& bProjectileReadabilityEnabled == Other.bProjectileReadabilityEnabled
		&& ProjectileVisualScale == Other.ProjectileVisualScale
		&& ProjectileGlowFadeDuration == Other.ProjectileGlowFadeDuration;
}

bool FSMAGFAnimationSettings::operator==(const FSMAGFAnimationSettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bFireAnticipationAnimationEnabled == Other.bFireAnticipationAnimationEnabled
		&& bWeaponRecoilAnimationEnabled == Other.bWeaponRecoilAnimationEnabled
		&& bCharacterFireReactionEnabled == Other.bCharacterFireReactionEnabled
		&& bEnemyHitReactionEnabled == Other.bEnemyHitReactionEnabled
		&& bEnemyDeathReactionEnabled == Other.bEnemyDeathReactionEnabled
		&& bSquashAndStretchEnabled == Other.bSquashAndStretchEnabled
		&& bOvershootRecoveryEnabled == Other.bOvershootRecoveryEnabled
		&& bProceduralWobbleEnabled == Other.bProceduralWobbleEnabled
		&& AnimationIntensity == Other.AnimationIntensity
		&& SquashAmount == Other.SquashAmount
		&& StretchAmount == Other.StretchAmount
		&& OvershootAmount == Other.OvershootAmount
		&& WobbleAmount == Other.WobbleAmount
		&& WobbleFrequency == Other.WobbleFrequency
		&& ReactionDuration == Other.ReactionDuration
		&& RecoveryDuration == Other.RecoveryDuration
		&& Easing == Other.Easing
		&& bProjectileAnimationEnabled == Other.bProjectileAnimationEnabled
		&& ProjectileFlightStretch == Other.ProjectileFlightStretch
		&& ProjectileFlightGrowth == Other.ProjectileFlightGrowth
		&& ProjectileImpactSquash == Other.ProjectileImpactSquash
		&& ProjectileImpactAnimationDuration == Other.ProjectileImpactAnimationDuration;
}

bool FSMAGFAudiovisualSettings::operator==(const FSMAGFAudiovisualSettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bMuzzleFlashEnabled == Other.bMuzzleFlashEnabled
		&& bShotLightFlashEnabled == Other.bShotLightFlashEnabled
		&& bProjectileVisualScaleEnabled == Other.bProjectileVisualScaleEnabled
		&& bProjectileTrailEnabled == Other.bProjectileTrailEnabled
		&& bShellEjectionEnabled == Other.bShellEjectionEnabled
		&& bFireSoundEnabled == Other.bFireSoundEnabled
		&& bLayeredFireSoundEnabled == Other.bLayeredFireSoundEnabled
		&& bImpactFlashEnabled == Other.bImpactFlashEnabled
		&& bImpactEffectEnabled == Other.bImpactEffectEnabled
		&& bImpactSoundEnabled == Other.bImpactSoundEnabled
		&& bEnemyFlashEnabled == Other.bEnemyFlashEnabled
		&& bDeathEffectEnabled == Other.bDeathEffectEnabled
		&& bExplosionEffectEnabled == Other.bExplosionEffectEnabled
		&& MuzzleFlashScale == Other.MuzzleFlashScale
		&& MuzzleFlashDuration == Other.MuzzleFlashDuration
		&& ShotLightIntensity == Other.ShotLightIntensity
		&& ShotLightRadius == Other.ShotLightRadius
		&& ProjectileVisualScale == Other.ProjectileVisualScale
		&& TrailDuration == Other.TrailDuration
		&& ShellLifetime == Other.ShellLifetime
		&& FireSoundVolume == Other.FireSoundVolume
		&& FireSoundPitchVariation == Other.FireSoundPitchVariation
		&& ImpactScale == Other.ImpactScale
		&& ImpactDuration == Other.ImpactDuration
		&& ImpactLightIntensity == Other.ImpactLightIntensity
		&& EnemyFlashDuration == Other.EnemyFlashDuration
		&& DeathEffectScale == Other.DeathEffectScale
		&& ExplosionScale == Other.ExplosionScale;
}

bool FSMAGFCameraSettings::operator==(const FSMAGFCameraSettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bCameraFollowSmoothingEnabled == Other.bCameraFollowSmoothingEnabled
		&& bAimLookAheadEnabled == Other.bAimLookAheadEnabled
		&& bScreenShakeEnabled == Other.bScreenShakeEnabled
		&& bDirectionalCameraKickEnabled == Other.bDirectionalCameraKickEnabled
		&& bKillCameraEmphasisEnabled == Other.bKillCameraEmphasisEnabled
		&& CameraFollowSpeed == Other.CameraFollowSpeed
		&& AimLookAheadDistance == Other.AimLookAheadDistance
		&& AimLookAheadInterpolation == Other.AimLookAheadInterpolation
		&& ShakeTranslationAmplitude == Other.ShakeTranslationAmplitude
		&& ShakeRotationAmplitude == Other.ShakeRotationAmplitude
		&& ShakeFrequency == Other.ShakeFrequency
		&& ShakeDuration == Other.ShakeDuration
		&& ShakeFalloff == Other.ShakeFalloff
		&& CameraKickDistance == Other.CameraKickDistance
		&& CameraKickRotation == Other.CameraKickRotation
		&& CameraKickDuration == Other.CameraKickDuration
		&& CameraKickRecovery == Other.CameraKickRecovery
		&& KillCameraDuration == Other.KillCameraDuration;
}

bool FSMAGFPermanenceSettings::operator==(const FSMAGFPermanenceSettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bKeepEnemyBodiesEnabled == Other.bKeepEnemyBodiesEnabled
		&& bKeepShellCasingsEnabled == Other.bKeepShellCasingsEnabled
		&& bKeepImpactEvidenceEnabled == Other.bKeepImpactEvidenceEnabled
		&& bKeepDebrisEnabled == Other.bKeepDebrisEnabled
		&& bKeepAftermathSmokeEnabled == Other.bKeepAftermathSmokeEnabled
		&& CorpseLifetime == Other.CorpseLifetime
		&& CorpseDissolveDuration == Other.CorpseDissolveDuration
		&& ShellLifetime == Other.ShellLifetime
		&& DebrisLifetime == Other.DebrisLifetime
		&& ImpactEvidenceLifetime == Other.ImpactEvidenceLifetime
		&& MaxPersistentCorpses == Other.MaxPersistentCorpses
		&& MaxPersistentShells == Other.MaxPersistentShells
		&& MaxPersistentDebris == Other.MaxPersistentDebris;
}

bool FSMAGFSmokeAndMirrorsSettings::operator==(const FSMAGFSmokeAndMirrorsSettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bScriptedEnemyWaveEnabled == Other.bScriptedEnemyWaveEnabled
		&& bForcedDramaticKillEnabled == Other.bForcedDramaticKillEnabled
		&& bForcedExplosionEnabled == Other.bForcedExplosionEnabled
		&& bShowcaseSlowMotionEnabled == Other.bShowcaseSlowMotionEnabled
		&& bPredefinedShowcaseSpawnsEnabled == Other.bPredefinedShowcaseSpawnsEnabled
		&& bAutomaticEncounterResetEnabled == Other.bAutomaticEncounterResetEnabled
		&& bDeterministicRandomSeedEnabled == Other.bDeterministicRandomSeedEnabled
		&& bFreezeEnemiesEnabled == Other.bFreezeEnemiesEnabled
		&& bScriptedShowcaseSequenceEnabled == Other.bScriptedShowcaseSequenceEnabled
		&& WaveSize == Other.WaveSize
		&& SlowMotionTimeScale == Other.SlowMotionTimeScale
		&& SlowMotionDuration == Other.SlowMotionDuration
		&& EncounterResetDelay == Other.EncounterResetDelay
		&& RandomSeed == Other.RandomSeed
		&& SequenceDuration == Other.SequenceDuration;
}

bool FSMAGFFocusSettings::operator==(const FSMAGFFocusSettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bAimDirectionFramingEnabled == Other.bAimDirectionFramingEnabled
		&& bTargetEmphasisEnabled == Other.bTargetEmphasisEnabled
		&& bContextualCameraEmphasisEnabled == Other.bContextualCameraEmphasisEnabled
		&& bContextualLightingPulseEnabled == Other.bContextualLightingPulseEnabled
		&& bContextualPostProcessEnabled == Other.bContextualPostProcessEnabled
		&& bBackgroundDeemphasisEnabled == Other.bBackgroundDeemphasisEnabled
		&& FocusIntensity == Other.FocusIntensity
		&& FocusDuration == Other.FocusDuration
		&& FocusCameraBias == Other.FocusCameraBias
		&& FocusVignetteAmount == Other.FocusVignetteAmount
		&& FocusExposureBias == Other.FocusExposureBias;
}

bool FSMAGFJuiceResponsiblySettings::operator==(const FSMAGFJuiceResponsiblySettings& Other) const
{
	return bEnabled == Other.bEnabled
		&& bEventImportanceScalingEnabled == Other.bEventImportanceScalingEnabled
		&& MinorMultiplier == Other.MinorMultiplier
		&& NormalMultiplier == Other.NormalMultiplier
		&& MajorMultiplier == Other.MajorMultiplier
		&& CriticalMultiplier == Other.CriticalMultiplier;
}

bool FSMAGFShowcaseSettings::operator==(const FSMAGFShowcaseSettings& Other) const
{
	return Master == Other.Master
		&& Feeling == Other.Feeling
		&& Animation == Other.Animation
		&& Audiovisual == Other.Audiovisual
		&& Camera == Other.Camera
		&& Permanence == Other.Permanence
		&& SmokeAndMirrors == Other.SmokeAndMirrors
		&& Focus == Other.Focus
		&& JuiceResponsibly == Other.JuiceResponsibly;
}

void FSMAGFShowcaseSettings::Validate()
{
	const FSMAGFShowcaseSettings Defaults;
	Master.MasterIntensity = ValidFloat(Master.MasterIntensity, 0.f, 2.f, Defaults.Master.MasterIntensity);
	Feeling.ProjectileVisualScale = ValidFloat(Feeling.ProjectileVisualScale, 0.5f, 3.0f, Defaults.Feeling.ProjectileVisualScale);
	Feeling.ProjectileGlowFadeDuration = ValidFloat(Feeling.ProjectileGlowFadeDuration, 0.0f, 5.0f, Defaults.Feeling.ProjectileGlowFadeDuration);
	Feeling.FireAnticipationTime = ValidFloat(Feeling.FireAnticipationTime, 0.f, 2.f, Defaults.Feeling.FireAnticipationTime);
	Feeling.AnticipationScale = ValidFloat(Feeling.AnticipationScale, 0.f, 2.f, Defaults.Feeling.AnticipationScale);
	Feeling.MovementAcceleration = ValidFloat(Feeling.MovementAcceleration, 0.f, 10000.f, Defaults.Feeling.MovementAcceleration);
	Feeling.MovementDeceleration = ValidFloat(Feeling.MovementDeceleration, 0.f, 10000.f, Defaults.Feeling.MovementDeceleration);
	Feeling.RecoilDistance = ValidFloat(Feeling.RecoilDistance, 0.f, 100.f, Defaults.Feeling.RecoilDistance);
	Feeling.ImpactResponseDistance = ValidFloat(Feeling.ImpactResponseDistance, 0.f, 100.f, Defaults.Feeling.ImpactResponseDistance);
	Feeling.ImpactResponseDuration = ValidFloat(Feeling.ImpactResponseDuration, 0.f, 1.f, Defaults.Feeling.ImpactResponseDuration);
	Feeling.RecoilRotation = ValidFloat(Feeling.RecoilRotation, 0.f, 45.f, Defaults.Feeling.RecoilRotation);
	Feeling.RecoilDuration = ValidFloat(Feeling.RecoilDuration, 0.f, 5.f, Defaults.Feeling.RecoilDuration);
	Feeling.RecoilRecoveryDuration = ValidFloat(Feeling.RecoilRecoveryDuration, 0.f, 5.f, Defaults.Feeling.RecoilRecoveryDuration);
	Feeling.HitStopDuration = ValidFloat(Feeling.HitStopDuration, 0.f, 1.f, Defaults.Feeling.HitStopDuration);
	Feeling.KillHitStopDuration = ValidFloat(Feeling.KillHitStopDuration, 0.f, 1.f, Defaults.Feeling.KillHitStopDuration);
	Feeling.HitStopTimeScale = ValidFloat(Feeling.HitStopTimeScale, 0.f, 1.f, Defaults.Feeling.HitStopTimeScale);
	Feeling.EnemyKnockbackStrength = ValidFloat(Feeling.EnemyKnockbackStrength, 0.f, 5000.f, Defaults.Feeling.EnemyKnockbackStrength);
	Feeling.EnemyKnockbackDuration = ValidFloat(Feeling.EnemyKnockbackDuration, 0.f, 5.f, Defaults.Feeling.EnemyKnockbackDuration);
	Feeling.PlayerKnockbackStrength = ValidFloat(Feeling.PlayerKnockbackStrength, 0.f, 5000.f, Defaults.Feeling.PlayerKnockbackStrength);
	Feeling.PlayerKnockbackRecovery = ValidFloat(Feeling.PlayerKnockbackRecovery, 0.f, 5.f, Defaults.Feeling.PlayerKnockbackRecovery);
	Feeling.HitReactionDuration = ValidFloat(Feeling.HitReactionDuration, 0.f, 5.f, Defaults.Feeling.HitReactionDuration);
	Feeling.HitReactionIntensity = ValidFloat(Feeling.HitReactionIntensity, 0.f, 2.f, Defaults.Feeling.HitReactionIntensity);
	Animation.ProjectileFlightStretch = ValidFloat(Animation.ProjectileFlightStretch, 1.0f, 4.0f, Defaults.Animation.ProjectileFlightStretch);
	Animation.ProjectileFlightGrowth = ValidFloat(Animation.ProjectileFlightGrowth, 1.0f, 3.0f, Defaults.Animation.ProjectileFlightGrowth);
	Animation.ProjectileImpactSquash = ValidFloat(Animation.ProjectileImpactSquash, 0.1f, 1.0f, Defaults.Animation.ProjectileImpactSquash);
	Animation.ProjectileImpactAnimationDuration = ValidFloat(Animation.ProjectileImpactAnimationDuration, 0.0f, 0.5f, Defaults.Animation.ProjectileImpactAnimationDuration);
	Animation.AnimationIntensity = ValidFloat(Animation.AnimationIntensity, 0.f, 2.f, Defaults.Animation.AnimationIntensity);
	Animation.SquashAmount = ValidFloat(Animation.SquashAmount, 0.f, 0.9f, Defaults.Animation.SquashAmount);
	Animation.StretchAmount = ValidFloat(Animation.StretchAmount, 0.f, 2.f, Defaults.Animation.StretchAmount);
	Animation.OvershootAmount = ValidFloat(Animation.OvershootAmount, 0.f, 2.f, Defaults.Animation.OvershootAmount);
	Animation.WobbleAmount = ValidFloat(Animation.WobbleAmount, 0.f, 2.f, Defaults.Animation.WobbleAmount);
	Animation.WobbleFrequency = ValidFloat(Animation.WobbleFrequency, 0.f, 30.f, Defaults.Animation.WobbleFrequency);
	Animation.ReactionDuration = ValidFloat(Animation.ReactionDuration, 0.f, 5.f, Defaults.Animation.ReactionDuration);
	Animation.RecoveryDuration = ValidFloat(Animation.RecoveryDuration, 0.f, 5.f, Defaults.Animation.RecoveryDuration);
	Audiovisual.MuzzleFlashScale = ValidFloat(Audiovisual.MuzzleFlashScale, 0.f, 10.f, Defaults.Audiovisual.MuzzleFlashScale);
	Audiovisual.MuzzleFlashDuration = ValidFloat(Audiovisual.MuzzleFlashDuration, 0.f, 5.f, Defaults.Audiovisual.MuzzleFlashDuration);
	Audiovisual.ShotLightIntensity = ValidFloat(Audiovisual.ShotLightIntensity, 0.f, 100000.f, Defaults.Audiovisual.ShotLightIntensity);
	Audiovisual.ShotLightRadius = ValidFloat(Audiovisual.ShotLightRadius, 0.f, 5000.f, Defaults.Audiovisual.ShotLightRadius);
	Audiovisual.ProjectileVisualScale = ValidFloat(Audiovisual.ProjectileVisualScale, 0.01f, 10.f, Defaults.Audiovisual.ProjectileVisualScale);
	Audiovisual.TrailDuration = ValidFloat(Audiovisual.TrailDuration, 0.f, 10.f, Defaults.Audiovisual.TrailDuration);
	Audiovisual.ShellLifetime = ValidFloat(Audiovisual.ShellLifetime, 0.f, 120.f, Defaults.Audiovisual.ShellLifetime);
	Audiovisual.FireSoundVolume = ValidFloat(Audiovisual.FireSoundVolume, 0.f, 2.f, Defaults.Audiovisual.FireSoundVolume);
	Audiovisual.FireSoundPitchVariation = ValidFloat(Audiovisual.FireSoundPitchVariation, 0.f, 1.f, Defaults.Audiovisual.FireSoundPitchVariation);
	Audiovisual.ImpactScale = ValidFloat(Audiovisual.ImpactScale, 0.f, 10.f, Defaults.Audiovisual.ImpactScale);
	Audiovisual.ImpactDuration = ValidFloat(Audiovisual.ImpactDuration, 0.f, 10.f, Defaults.Audiovisual.ImpactDuration);
	Audiovisual.ImpactLightIntensity = ValidFloat(Audiovisual.ImpactLightIntensity, 0.f, 100000.f, Defaults.Audiovisual.ImpactLightIntensity);
	Audiovisual.EnemyFlashDuration = ValidFloat(Audiovisual.EnemyFlashDuration, 0.f, 5.f, Defaults.Audiovisual.EnemyFlashDuration);
	Audiovisual.DeathEffectScale = ValidFloat(Audiovisual.DeathEffectScale, 0.f, 10.f, Defaults.Audiovisual.DeathEffectScale);
	Audiovisual.ExplosionScale = ValidFloat(Audiovisual.ExplosionScale, 0.f, 10.f, Defaults.Audiovisual.ExplosionScale);
	Camera.CameraFollowSpeed = ValidFloat(Camera.CameraFollowSpeed, 0.f, 100.f, Defaults.Camera.CameraFollowSpeed);
	Camera.AimLookAheadDistance = ValidFloat(Camera.AimLookAheadDistance, 0.f, 1000.f, Defaults.Camera.AimLookAheadDistance);
	Camera.AimLookAheadInterpolation = ValidFloat(Camera.AimLookAheadInterpolation, 0.f, 100.f, Defaults.Camera.AimLookAheadInterpolation);
	Camera.ShakeTranslationAmplitude = ValidFloat(Camera.ShakeTranslationAmplitude, 0.f, 100.f, Defaults.Camera.ShakeTranslationAmplitude);
	Camera.ShakeRotationAmplitude = ValidFloat(Camera.ShakeRotationAmplitude, 0.f, 30.f, Defaults.Camera.ShakeRotationAmplitude);
	Camera.ShakeFrequency = ValidFloat(Camera.ShakeFrequency, 0.f, 100.f, Defaults.Camera.ShakeFrequency);
	Camera.ShakeDuration = ValidFloat(Camera.ShakeDuration, 0.f, 5.f, Defaults.Camera.ShakeDuration);
	Camera.ShakeFalloff = ValidFloat(Camera.ShakeFalloff, 0.f, 10.f, Defaults.Camera.ShakeFalloff);
	Camera.CameraKickDistance = ValidFloat(Camera.CameraKickDistance, 0.f, 100.f, Defaults.Camera.CameraKickDistance);
	Camera.CameraKickRotation = ValidFloat(Camera.CameraKickRotation, 0.f, 30.f, Defaults.Camera.CameraKickRotation);
	Camera.CameraKickDuration = ValidFloat(Camera.CameraKickDuration, 0.f, 5.f, Defaults.Camera.CameraKickDuration);
	Camera.CameraKickRecovery = ValidFloat(Camera.CameraKickRecovery, 0.f, 5.f, Defaults.Camera.CameraKickRecovery);
	Camera.KillCameraDuration = ValidFloat(Camera.KillCameraDuration, 0.f, 5.f, Defaults.Camera.KillCameraDuration);
	Permanence.CorpseLifetime = ValidFloat(Permanence.CorpseLifetime, 0.f, 300.f, Defaults.Permanence.CorpseLifetime);
	Permanence.CorpseDissolveDuration = FMath::Min(Permanence.CorpseLifetime, ValidFloat(Permanence.CorpseDissolveDuration, 0.f, 30.f, Defaults.Permanence.CorpseDissolveDuration));
	Permanence.ShellLifetime = ValidFloat(Permanence.ShellLifetime, 0.f, 300.f, Defaults.Permanence.ShellLifetime);
	Permanence.DebrisLifetime = ValidFloat(Permanence.DebrisLifetime, 0.f, 300.f, Defaults.Permanence.DebrisLifetime);
	Permanence.ImpactEvidenceLifetime = ValidFloat(Permanence.ImpactEvidenceLifetime, 0.f, 300.f, Defaults.Permanence.ImpactEvidenceLifetime);
	Permanence.MaxPersistentCorpses = FMath::Clamp(Permanence.MaxPersistentCorpses, 0, 1000);
	Permanence.MaxPersistentShells = FMath::Clamp(Permanence.MaxPersistentShells, 0, 1000);
	Permanence.MaxPersistentDebris = FMath::Clamp(Permanence.MaxPersistentDebris, 0, 1000);
	SmokeAndMirrors.WaveSize = FMath::Clamp(SmokeAndMirrors.WaveSize, 1, 100);
	SmokeAndMirrors.SlowMotionTimeScale = ValidFloat(SmokeAndMirrors.SlowMotionTimeScale, 0.01f, 1.f, Defaults.SmokeAndMirrors.SlowMotionTimeScale);
	SmokeAndMirrors.SlowMotionDuration = ValidFloat(SmokeAndMirrors.SlowMotionDuration, 0.f, 10.f, Defaults.SmokeAndMirrors.SlowMotionDuration);
	SmokeAndMirrors.EncounterResetDelay = ValidFloat(SmokeAndMirrors.EncounterResetDelay, 0.f, 120.f, Defaults.SmokeAndMirrors.EncounterResetDelay);
	SmokeAndMirrors.RandomSeed = FMath::Clamp(SmokeAndMirrors.RandomSeed, 0, 2147483647);
	SmokeAndMirrors.SequenceDuration = ValidFloat(SmokeAndMirrors.SequenceDuration, 0.f, 300.f, Defaults.SmokeAndMirrors.SequenceDuration);
	Focus.FocusIntensity = ValidFloat(Focus.FocusIntensity, 0.f, 2.f, Defaults.Focus.FocusIntensity);
	Focus.FocusDuration = ValidFloat(Focus.FocusDuration, 0.f, 10.f, Defaults.Focus.FocusDuration);
	Focus.FocusCameraBias = ValidFloat(Focus.FocusCameraBias, 0.f, 1000.f, Defaults.Focus.FocusCameraBias);
	Focus.FocusVignetteAmount = ValidFloat(Focus.FocusVignetteAmount, 0.f, 1.f, Defaults.Focus.FocusVignetteAmount);
	Focus.FocusExposureBias = ValidFloat(Focus.FocusExposureBias, -5.f, 5.f, Defaults.Focus.FocusExposureBias);
	JuiceResponsibly.MinorMultiplier = ValidFloat(JuiceResponsibly.MinorMultiplier, 0.f, 4.f, Defaults.JuiceResponsibly.MinorMultiplier);
	JuiceResponsibly.NormalMultiplier = ValidFloat(JuiceResponsibly.NormalMultiplier, 0.f, 4.f, Defaults.JuiceResponsibly.NormalMultiplier);
	JuiceResponsibly.MajorMultiplier = ValidFloat(JuiceResponsibly.MajorMultiplier, 0.f, 4.f, Defaults.JuiceResponsibly.MajorMultiplier);
	JuiceResponsibly.CriticalMultiplier = ValidFloat(JuiceResponsibly.CriticalMultiplier, 0.f, 4.f, Defaults.JuiceResponsibly.CriticalMultiplier);
	if (static_cast<uint8>(Animation.Easing) >= static_cast<uint8>(ESMAGFEasingType::Count))
	{
		Animation.Easing = Defaults.Animation.Easing;
	}
}

float FSMAGFJuiceResponsiblySettings::ResolveMultiplier(ESMAGFEventImportance Importance) const
{
	switch (Importance)
	{
	case ESMAGFEventImportance::Minor: return MinorMultiplier;
	case ESMAGFEventImportance::Normal: return NormalMultiplier;
	case ESMAGFEventImportance::Major: return MajorMultiplier;
	case ESMAGFEventImportance::Critical: return CriticalMultiplier;
	default: return 0.f;
	}
}

