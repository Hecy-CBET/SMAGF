#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Showcase/SMAGFShowcaseDeveloperSettings.h"

bool USMAGFShowcaseSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void USMAGFShowcaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	StartupDefaults = GetDefault<USMAGFShowcaseDeveloperSettings>()->Defaults;
	StartupDefaults.Validate();
	// Phase 3 deliberately starts in Baseline without changing authored defaults.
	Settings = ResolvePreset(ESMAGFShowcasePreset::Baseline);
}

void USMAGFShowcaseSubsystem::Deinitialize()
{
	OnSettingsChanged.Clear();
	OnShowcasePresetChanged.Clear();
	OnSettingsChangedNative.Clear();
	OnPresetChangedNative.Clear();
	Super::Deinitialize();
}

bool USMAGFShowcaseSubsystem::ApplySettings(const FSMAGFShowcaseSettings& NewSettings)
{
	FSMAGFShowcaseSettings Validated = NewSettings;
	Validated.Validate();
	if (Settings == Validated)
	{
		return false;
	}
	return CommitState(Validated, ESMAGFShowcasePreset::Custom, true);
}

bool USMAGFShowcaseSubsystem::ResetToDefaults()
{
	// Like all manual edits, a real reset enters Custom; a no-op preserves the label.
	return ApplySettings(StartupDefaults);
}

bool USMAGFShowcaseSubsystem::SetShowcaseEnabled(bool bEnabled)
{
	FSMAGFShowcaseSettings Copy = Settings;
	Copy.Master.bShowcaseEnabled = bEnabled;
	return ApplySettings(Copy);
}

bool USMAGFShowcaseSubsystem::SetMasterIntensity(float Intensity)
{
	FSMAGFShowcaseSettings Copy = Settings;
	Copy.Master.MasterIntensity = Intensity;
	return ApplySettings(Copy);
}

bool USMAGFShowcaseSubsystem::SetGroupEnabled(ESMAGFShowcaseGroup Group, bool bEnabled)
{
	FSMAGFShowcaseSettings Copy = Settings;
	switch (Group)
	{
	case ESMAGFShowcaseGroup::Feeling: Copy.Feeling.bEnabled = bEnabled; break;
	case ESMAGFShowcaseGroup::Animation: Copy.Animation.bEnabled = bEnabled; break;
	case ESMAGFShowcaseGroup::Audiovisual: Copy.Audiovisual.bEnabled = bEnabled; break;
	case ESMAGFShowcaseGroup::Camera: Copy.Camera.bEnabled = bEnabled; break;
	case ESMAGFShowcaseGroup::Permanence: Copy.Permanence.bEnabled = bEnabled; break;
	case ESMAGFShowcaseGroup::SmokeAndMirrors: Copy.SmokeAndMirrors.bEnabled = bEnabled; break;
	case ESMAGFShowcaseGroup::Focus: Copy.Focus.bEnabled = bEnabled; break;
	case ESMAGFShowcaseGroup::JuiceResponsibly: Copy.JuiceResponsibly.bEnabled = bEnabled; break;
	default: return false;
	}
	return ApplySettings(Copy);
}

bool USMAGFShowcaseSubsystem::IsGroupEnabled(ESMAGFShowcaseGroup Group) const
{
	switch (Group)
	{
	case ESMAGFShowcaseGroup::Feeling: return Settings.Feeling.bEnabled;
	case ESMAGFShowcaseGroup::Animation: return Settings.Animation.bEnabled;
	case ESMAGFShowcaseGroup::Audiovisual: return Settings.Audiovisual.bEnabled;
	case ESMAGFShowcaseGroup::Camera: return Settings.Camera.bEnabled;
	case ESMAGFShowcaseGroup::Permanence: return Settings.Permanence.bEnabled;
	case ESMAGFShowcaseGroup::SmokeAndMirrors: return Settings.SmokeAndMirrors.bEnabled;
	case ESMAGFShowcaseGroup::Focus: return Settings.Focus.bEnabled;
	case ESMAGFShowcaseGroup::JuiceResponsibly: return Settings.JuiceResponsibly.bEnabled;
	default: return false;
	}
}

bool USMAGFShowcaseSubsystem::SetFeatureEnabled(ESMAGFShowcaseFeature Feature, bool bEnabled)
{
	FSMAGFShowcaseSettings Copy = Settings;
	switch (Feature)
	{
	case ESMAGFShowcaseFeature::ProjectileAnimation: Copy.Animation.bProjectileAnimationEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ProjectileReadability: Copy.Feeling.bProjectileReadabilityEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::FireAnticipation: Copy.Feeling.bFireAnticipationEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::MovementAcceleration: Copy.Feeling.bMovementAccelerationEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::Recoil: Copy.Feeling.bRecoilEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ImpactResponse: Copy.Feeling.bImpactResponseEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::HitStop: Copy.Feeling.bHitStopEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::EnemyKnockback: Copy.Feeling.bEnemyKnockbackEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::PlayerKnockback: Copy.Feeling.bPlayerKnockbackEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::HitReaction: Copy.Feeling.bHitReactionEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::FireAnticipationAnimation: Copy.Animation.bFireAnticipationAnimationEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::WeaponRecoilAnimation: Copy.Animation.bWeaponRecoilAnimationEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::CharacterFireReaction: Copy.Animation.bCharacterFireReactionEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::EnemyHitReaction: Copy.Animation.bEnemyHitReactionEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::EnemyDeathReaction: Copy.Animation.bEnemyDeathReactionEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::SquashAndStretch: Copy.Animation.bSquashAndStretchEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::OvershootRecovery: Copy.Animation.bOvershootRecoveryEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ProceduralWobble: Copy.Animation.bProceduralWobbleEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::MuzzleFlash: Copy.Audiovisual.bMuzzleFlashEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ShotLightFlash: Copy.Audiovisual.bShotLightFlashEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ProjectileVisualScale: Copy.Audiovisual.bProjectileVisualScaleEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ProjectileTrail: Copy.Audiovisual.bProjectileTrailEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ShellEjection: Copy.Audiovisual.bShellEjectionEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::FireSound: Copy.Audiovisual.bFireSoundEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::LayeredFireSound: Copy.Audiovisual.bLayeredFireSoundEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ImpactFlash: Copy.Audiovisual.bImpactFlashEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ImpactEffect: Copy.Audiovisual.bImpactEffectEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ImpactSound: Copy.Audiovisual.bImpactSoundEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::EnemyFlash: Copy.Audiovisual.bEnemyFlashEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::DeathEffect: Copy.Audiovisual.bDeathEffectEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ExplosionEffect: Copy.Audiovisual.bExplosionEffectEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::CameraFollowSmoothing: Copy.Camera.bCameraFollowSmoothingEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::AimLookAhead: Copy.Camera.bAimLookAheadEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ScreenShake: Copy.Camera.bScreenShakeEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::DirectionalCameraKick: Copy.Camera.bDirectionalCameraKickEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::KillCameraEmphasis: Copy.Camera.bKillCameraEmphasisEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::KeepEnemyBodies: Copy.Permanence.bKeepEnemyBodiesEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::KeepShellCasings: Copy.Permanence.bKeepShellCasingsEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::KeepImpactEvidence: Copy.Permanence.bKeepImpactEvidenceEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::KeepDebris: Copy.Permanence.bKeepDebrisEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::KeepAftermathSmoke: Copy.Permanence.bKeepAftermathSmokeEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::EnemyMovement: Copy.SmokeAndMirrors.bEnemyMovementEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::EnemyThreat: Copy.SmokeAndMirrors.bEnemyThreatEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ScriptedEnemyWave: Copy.SmokeAndMirrors.bScriptedEnemyWaveEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ForcedDramaticKill: Copy.SmokeAndMirrors.bForcedDramaticKillEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ForcedExplosion: Copy.SmokeAndMirrors.bForcedExplosionEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ShowcaseSlowMotion: Copy.SmokeAndMirrors.bShowcaseSlowMotionEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::PredefinedShowcaseSpawns: Copy.SmokeAndMirrors.bPredefinedShowcaseSpawnsEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::AutomaticEncounterReset: Copy.SmokeAndMirrors.bAutomaticEncounterResetEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::DeterministicRandomSeed: Copy.SmokeAndMirrors.bDeterministicRandomSeedEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::FreezeEnemies: Copy.SmokeAndMirrors.bFreezeEnemiesEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ScriptedShowcaseSequence: Copy.SmokeAndMirrors.bScriptedShowcaseSequenceEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::AimDirectionFraming: Copy.Focus.bAimDirectionFramingEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::TargetEmphasis: Copy.Focus.bTargetEmphasisEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ContextualCameraEmphasis: Copy.Focus.bContextualCameraEmphasisEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ContextualLightingPulse: Copy.Focus.bContextualLightingPulseEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::ContextualPostProcess: Copy.Focus.bContextualPostProcessEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::BackgroundDeemphasis: Copy.Focus.bBackgroundDeemphasisEnabled = bEnabled; break;
	case ESMAGFShowcaseFeature::EventImportanceScaling: Copy.JuiceResponsibly.bEventImportanceScalingEnabled = bEnabled; break;
	default: return false;
	}
	return ApplySettings(Copy);
}

bool USMAGFShowcaseSubsystem::IsFeatureSelectedEnabled(ESMAGFShowcaseFeature Feature) const
{
	switch (Feature)
	{
	case ESMAGFShowcaseFeature::ProjectileAnimation: return Settings.Animation.bProjectileAnimationEnabled;
	case ESMAGFShowcaseFeature::ProjectileReadability: return Settings.Feeling.bProjectileReadabilityEnabled;
	case ESMAGFShowcaseFeature::FireAnticipation: return Settings.Feeling.bFireAnticipationEnabled;
	case ESMAGFShowcaseFeature::MovementAcceleration: return Settings.Feeling.bMovementAccelerationEnabled;
	case ESMAGFShowcaseFeature::Recoil: return Settings.Feeling.bRecoilEnabled;
	case ESMAGFShowcaseFeature::ImpactResponse: return Settings.Feeling.bImpactResponseEnabled;
	case ESMAGFShowcaseFeature::HitStop: return Settings.Feeling.bHitStopEnabled;
	case ESMAGFShowcaseFeature::EnemyKnockback: return Settings.Feeling.bEnemyKnockbackEnabled;
	case ESMAGFShowcaseFeature::PlayerKnockback: return Settings.Feeling.bPlayerKnockbackEnabled;
	case ESMAGFShowcaseFeature::HitReaction: return Settings.Feeling.bHitReactionEnabled;
	case ESMAGFShowcaseFeature::FireAnticipationAnimation: return Settings.Animation.bFireAnticipationAnimationEnabled;
	case ESMAGFShowcaseFeature::WeaponRecoilAnimation: return Settings.Animation.bWeaponRecoilAnimationEnabled;
	case ESMAGFShowcaseFeature::CharacterFireReaction: return Settings.Animation.bCharacterFireReactionEnabled;
	case ESMAGFShowcaseFeature::EnemyHitReaction: return Settings.Animation.bEnemyHitReactionEnabled;
	case ESMAGFShowcaseFeature::EnemyDeathReaction: return Settings.Animation.bEnemyDeathReactionEnabled;
	case ESMAGFShowcaseFeature::SquashAndStretch: return Settings.Animation.bSquashAndStretchEnabled;
	case ESMAGFShowcaseFeature::OvershootRecovery: return Settings.Animation.bOvershootRecoveryEnabled;
	case ESMAGFShowcaseFeature::ProceduralWobble: return Settings.Animation.bProceduralWobbleEnabled;
	case ESMAGFShowcaseFeature::MuzzleFlash: return Settings.Audiovisual.bMuzzleFlashEnabled;
	case ESMAGFShowcaseFeature::ShotLightFlash: return Settings.Audiovisual.bShotLightFlashEnabled;
	case ESMAGFShowcaseFeature::ProjectileVisualScale: return Settings.Audiovisual.bProjectileVisualScaleEnabled;
	case ESMAGFShowcaseFeature::ProjectileTrail: return Settings.Audiovisual.bProjectileTrailEnabled;
	case ESMAGFShowcaseFeature::ShellEjection: return Settings.Audiovisual.bShellEjectionEnabled;
	case ESMAGFShowcaseFeature::FireSound: return Settings.Audiovisual.bFireSoundEnabled;
	case ESMAGFShowcaseFeature::LayeredFireSound: return Settings.Audiovisual.bLayeredFireSoundEnabled;
	case ESMAGFShowcaseFeature::ImpactFlash: return Settings.Audiovisual.bImpactFlashEnabled;
	case ESMAGFShowcaseFeature::ImpactEffect: return Settings.Audiovisual.bImpactEffectEnabled;
	case ESMAGFShowcaseFeature::ImpactSound: return Settings.Audiovisual.bImpactSoundEnabled;
	case ESMAGFShowcaseFeature::EnemyFlash: return Settings.Audiovisual.bEnemyFlashEnabled;
	case ESMAGFShowcaseFeature::DeathEffect: return Settings.Audiovisual.bDeathEffectEnabled;
	case ESMAGFShowcaseFeature::ExplosionEffect: return Settings.Audiovisual.bExplosionEffectEnabled;
	case ESMAGFShowcaseFeature::CameraFollowSmoothing: return Settings.Camera.bCameraFollowSmoothingEnabled;
	case ESMAGFShowcaseFeature::AimLookAhead: return Settings.Camera.bAimLookAheadEnabled;
	case ESMAGFShowcaseFeature::ScreenShake: return Settings.Camera.bScreenShakeEnabled;
	case ESMAGFShowcaseFeature::DirectionalCameraKick: return Settings.Camera.bDirectionalCameraKickEnabled;
	case ESMAGFShowcaseFeature::KillCameraEmphasis: return Settings.Camera.bKillCameraEmphasisEnabled;
	case ESMAGFShowcaseFeature::KeepEnemyBodies: return Settings.Permanence.bKeepEnemyBodiesEnabled;
	case ESMAGFShowcaseFeature::KeepShellCasings: return Settings.Permanence.bKeepShellCasingsEnabled;
	case ESMAGFShowcaseFeature::KeepImpactEvidence: return Settings.Permanence.bKeepImpactEvidenceEnabled;
	case ESMAGFShowcaseFeature::KeepDebris: return Settings.Permanence.bKeepDebrisEnabled;
	case ESMAGFShowcaseFeature::KeepAftermathSmoke: return Settings.Permanence.bKeepAftermathSmokeEnabled;
	case ESMAGFShowcaseFeature::EnemyMovement: return Settings.SmokeAndMirrors.bEnemyMovementEnabled;
	case ESMAGFShowcaseFeature::EnemyThreat: return Settings.SmokeAndMirrors.bEnemyThreatEnabled;
	case ESMAGFShowcaseFeature::ScriptedEnemyWave: return Settings.SmokeAndMirrors.bScriptedEnemyWaveEnabled;
	case ESMAGFShowcaseFeature::ForcedDramaticKill: return Settings.SmokeAndMirrors.bForcedDramaticKillEnabled;
	case ESMAGFShowcaseFeature::ForcedExplosion: return Settings.SmokeAndMirrors.bForcedExplosionEnabled;
	case ESMAGFShowcaseFeature::ShowcaseSlowMotion: return Settings.SmokeAndMirrors.bShowcaseSlowMotionEnabled;
	case ESMAGFShowcaseFeature::PredefinedShowcaseSpawns: return Settings.SmokeAndMirrors.bPredefinedShowcaseSpawnsEnabled;
	case ESMAGFShowcaseFeature::AutomaticEncounterReset: return Settings.SmokeAndMirrors.bAutomaticEncounterResetEnabled;
	case ESMAGFShowcaseFeature::DeterministicRandomSeed: return Settings.SmokeAndMirrors.bDeterministicRandomSeedEnabled;
	case ESMAGFShowcaseFeature::FreezeEnemies: return Settings.SmokeAndMirrors.bFreezeEnemiesEnabled;
	case ESMAGFShowcaseFeature::ScriptedShowcaseSequence: return Settings.SmokeAndMirrors.bScriptedShowcaseSequenceEnabled;
	case ESMAGFShowcaseFeature::AimDirectionFraming: return Settings.Focus.bAimDirectionFramingEnabled;
	case ESMAGFShowcaseFeature::TargetEmphasis: return Settings.Focus.bTargetEmphasisEnabled;
	case ESMAGFShowcaseFeature::ContextualCameraEmphasis: return Settings.Focus.bContextualCameraEmphasisEnabled;
	case ESMAGFShowcaseFeature::ContextualLightingPulse: return Settings.Focus.bContextualLightingPulseEnabled;
	case ESMAGFShowcaseFeature::ContextualPostProcess: return Settings.Focus.bContextualPostProcessEnabled;
	case ESMAGFShowcaseFeature::BackgroundDeemphasis: return Settings.Focus.bBackgroundDeemphasisEnabled;
	case ESMAGFShowcaseFeature::EventImportanceScaling: return Settings.JuiceResponsibly.bEventImportanceScalingEnabled;
	default: return false;
	}
}

ESMAGFShowcaseGroup USMAGFShowcaseSubsystem::GetFeatureGroup(ESMAGFShowcaseFeature Feature) const
{
	switch (Feature)
	{
	case ESMAGFShowcaseFeature::ProjectileAnimation: return ESMAGFShowcaseGroup::Animation;
	case ESMAGFShowcaseFeature::ProjectileReadability: return ESMAGFShowcaseGroup::Feeling;
	case ESMAGFShowcaseFeature::FireAnticipation:
	case ESMAGFShowcaseFeature::MovementAcceleration:
	case ESMAGFShowcaseFeature::Recoil:
	case ESMAGFShowcaseFeature::ImpactResponse:
	case ESMAGFShowcaseFeature::HitStop:
	case ESMAGFShowcaseFeature::EnemyKnockback:
	case ESMAGFShowcaseFeature::PlayerKnockback:
	case ESMAGFShowcaseFeature::HitReaction:
		return ESMAGFShowcaseGroup::Feeling;
	case ESMAGFShowcaseFeature::FireAnticipationAnimation:
	case ESMAGFShowcaseFeature::WeaponRecoilAnimation:
	case ESMAGFShowcaseFeature::CharacterFireReaction:
	case ESMAGFShowcaseFeature::EnemyHitReaction:
	case ESMAGFShowcaseFeature::EnemyDeathReaction:
	case ESMAGFShowcaseFeature::SquashAndStretch:
	case ESMAGFShowcaseFeature::OvershootRecovery:
	case ESMAGFShowcaseFeature::ProceduralWobble:
		return ESMAGFShowcaseGroup::Animation;
	case ESMAGFShowcaseFeature::MuzzleFlash:
	case ESMAGFShowcaseFeature::ShotLightFlash:
	case ESMAGFShowcaseFeature::ProjectileVisualScale:
	case ESMAGFShowcaseFeature::ProjectileTrail:
	case ESMAGFShowcaseFeature::ShellEjection:
	case ESMAGFShowcaseFeature::FireSound:
	case ESMAGFShowcaseFeature::LayeredFireSound:
	case ESMAGFShowcaseFeature::ImpactFlash:
	case ESMAGFShowcaseFeature::ImpactEffect:
	case ESMAGFShowcaseFeature::ImpactSound:
	case ESMAGFShowcaseFeature::EnemyFlash:
	case ESMAGFShowcaseFeature::DeathEffect:
	case ESMAGFShowcaseFeature::ExplosionEffect:
		return ESMAGFShowcaseGroup::Audiovisual;
	case ESMAGFShowcaseFeature::CameraFollowSmoothing:
	case ESMAGFShowcaseFeature::AimLookAhead:
	case ESMAGFShowcaseFeature::ScreenShake:
	case ESMAGFShowcaseFeature::DirectionalCameraKick:
	case ESMAGFShowcaseFeature::KillCameraEmphasis:
		return ESMAGFShowcaseGroup::Camera;
	case ESMAGFShowcaseFeature::KeepEnemyBodies:
	case ESMAGFShowcaseFeature::KeepShellCasings:
	case ESMAGFShowcaseFeature::KeepImpactEvidence:
	case ESMAGFShowcaseFeature::KeepDebris:
	case ESMAGFShowcaseFeature::KeepAftermathSmoke:
		return ESMAGFShowcaseGroup::Permanence;
	case ESMAGFShowcaseFeature::EnemyMovement:
	case ESMAGFShowcaseFeature::EnemyThreat:
	case ESMAGFShowcaseFeature::ScriptedEnemyWave:
	case ESMAGFShowcaseFeature::ForcedDramaticKill:
	case ESMAGFShowcaseFeature::ForcedExplosion:
	case ESMAGFShowcaseFeature::ShowcaseSlowMotion:
	case ESMAGFShowcaseFeature::PredefinedShowcaseSpawns:
	case ESMAGFShowcaseFeature::AutomaticEncounterReset:
	case ESMAGFShowcaseFeature::DeterministicRandomSeed:
	case ESMAGFShowcaseFeature::FreezeEnemies:
	case ESMAGFShowcaseFeature::ScriptedShowcaseSequence:
		return ESMAGFShowcaseGroup::SmokeAndMirrors;
	case ESMAGFShowcaseFeature::AimDirectionFraming:
	case ESMAGFShowcaseFeature::TargetEmphasis:
	case ESMAGFShowcaseFeature::ContextualCameraEmphasis:
	case ESMAGFShowcaseFeature::ContextualLightingPulse:
	case ESMAGFShowcaseFeature::ContextualPostProcess:
	case ESMAGFShowcaseFeature::BackgroundDeemphasis:
		return ESMAGFShowcaseGroup::Focus;
	case ESMAGFShowcaseFeature::EventImportanceScaling:
		return ESMAGFShowcaseGroup::JuiceResponsibly;
	default: return ESMAGFShowcaseGroup::Count;
	}
}

bool USMAGFShowcaseSubsystem::IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature Feature) const
{
	return IsShowcaseEnabled() && IsGroupEnabled(GetFeatureGroup(Feature)) && IsFeatureSelectedEnabled(Feature);
}

float USMAGFShowcaseSubsystem::GetEventImportanceMultiplier(ESMAGFEventImportance Importance) const
{
	return Settings.JuiceResponsibly.ResolveMultiplier(Importance);
}


bool USMAGFShowcaseSubsystem::CommitState(const FSMAGFShowcaseSettings& Candidate, ESMAGFShowcasePreset Preset, bool bSaveCustom)
{
	if (bCommitting) { return false; }
	FSMAGFShowcaseSettings Validated = Candidate;
	Validated.Validate();
	const bool bSettingsChanged = !(Settings == Validated);
	const bool bPresetChanged = ActivePreset != Preset;
	if (!bSettingsChanged && !bPresetChanged) { return false; }
	TGuardValue<bool> Guard(bCommitting, true);
	Settings = MoveTemp(Validated);
	ActivePreset = Preset;
	if (Preset != ESMAGFShowcasePreset::Custom) { LastNamedPreset = Preset; }
	if (bSaveCustom) { CustomSnapshot = Settings; bHasCustomSnapshot = true; }
	// All authoritative state is committed before either notification is emitted.
	if (bSettingsChanged) { OnSettingsChanged.Broadcast(this); OnSettingsChangedNative.Broadcast(); }
	if (bPresetChanged) { OnShowcasePresetChanged.Broadcast(Preset); OnPresetChangedNative.Broadcast(); }
	return true;
}

FSMAGFShowcaseSettings USMAGFShowcaseSubsystem::ResolvePreset(ESMAGFShowcasePreset Preset) const
{
	if (Preset == ESMAGFShowcasePreset::Custom) { return bHasCustomSnapshot ? CustomSnapshot : Settings; }
	if (static_cast<uint8>(Preset) > static_cast<uint8>(ESMAGFShowcasePreset::FullShowcase)) { return Settings; }
	FSMAGFShowcaseSettings Result = StartupDefaults;
	Result.Master.bShowcaseEnabled = Preset != ESMAGFShowcasePreset::Baseline;
	Result.Feeling.bEnabled = Preset >= ESMAGFShowcasePreset::Feeling;
	Result.Animation.bEnabled = Preset >= ESMAGFShowcasePreset::Animation;
	Result.Audiovisual.bEnabled = Preset >= ESMAGFShowcasePreset::Audiovisual;
	Result.Camera.bEnabled = Preset >= ESMAGFShowcasePreset::Audiovisual;
	Result.Permanence.bEnabled = Preset >= ESMAGFShowcasePreset::Audiovisual;
	Result.SmokeAndMirrors.bEnabled = Preset >= ESMAGFShowcasePreset::SmokeAndMirrors;
	Result.Focus.bEnabled = Preset >= ESMAGFShowcasePreset::Focus;
	Result.JuiceResponsibly.bEnabled = Preset == ESMAGFShowcasePreset::FullShowcase;
	return Result;
}

bool USMAGFShowcaseSubsystem::SetShowcasePreset(ESMAGFShowcasePreset Preset)
{
	if (static_cast<uint8>(Preset) > static_cast<uint8>(ESMAGFShowcasePreset::Custom)
		|| (Preset == ESMAGFShowcasePreset::Custom && !bHasCustomSnapshot)) { return false; }
	return CommitState(ResolvePreset(Preset), Preset, false);
}

bool USMAGFShowcaseSubsystem::NextShowcasePreset()
{
	const ESMAGFShowcasePreset Reference = ActivePreset == ESMAGFShowcasePreset::Custom ? LastNamedPreset : ActivePreset;
	if (Reference == ESMAGFShowcasePreset::FullShowcase) { return false; }
	return SetShowcasePreset(static_cast<ESMAGFShowcasePreset>(static_cast<uint8>(Reference) + 1));
}

bool USMAGFShowcaseSubsystem::PreviousShowcasePreset()
{
	const ESMAGFShowcasePreset Reference = ActivePreset == ESMAGFShowcasePreset::Custom ? LastNamedPreset : ActivePreset;
	if (Reference == ESMAGFShowcasePreset::Baseline) { return false; }
	return SetShowcasePreset(static_cast<ESMAGFShowcasePreset>(static_cast<uint8>(Reference) - 1));
}

bool USMAGFShowcaseSubsystem::ResetCurrentGroupToDefaults(ESMAGFShowcaseGroup Group)
{
	FSMAGFShowcaseSettings Copy = Settings;
	switch (Group)
	{
	case ESMAGFShowcaseGroup::Feeling: Copy.Feeling = StartupDefaults.Feeling; break;
	case ESMAGFShowcaseGroup::Animation: Copy.Animation = StartupDefaults.Animation; break;
	case ESMAGFShowcaseGroup::Audiovisual: Copy.Audiovisual = StartupDefaults.Audiovisual; break;
	case ESMAGFShowcaseGroup::Camera: Copy.Camera = StartupDefaults.Camera; break;
	case ESMAGFShowcaseGroup::Permanence: Copy.Permanence = StartupDefaults.Permanence; break;
	case ESMAGFShowcaseGroup::SmokeAndMirrors: Copy.SmokeAndMirrors = StartupDefaults.SmokeAndMirrors; break;
	case ESMAGFShowcaseGroup::Focus: Copy.Focus = StartupDefaults.Focus; break;
	case ESMAGFShowcaseGroup::JuiceResponsibly: Copy.JuiceResponsibly = StartupDefaults.JuiceResponsibly; break;
	default: return false;
	}
	return ApplySettings(Copy);
}
