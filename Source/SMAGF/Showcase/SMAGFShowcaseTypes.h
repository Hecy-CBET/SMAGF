#pragma once

#include "CoreMinimal.h"
#include "SMAGFShowcaseTypes.generated.h"

UENUM(BlueprintType)
enum class ESMAGFShowcasePreset : uint8
{
	Baseline, Feeling, Animation, Audiovisual, SmokeAndMirrors, Focus, FullShowcase, Custom
};

UENUM(BlueprintType)
enum class ESMAGFEasingType : uint8
{
	Linear,
	EaseIn,
	EaseOut,
	EaseInOut,
	Back,
	Elastic,
	Bounce,
	Count UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESMAGFEventImportance : uint8
{
	Minor,
	Normal,
	Major,
	Critical,
	Count UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESMAGFShowcaseGroup : uint8
{
	Feeling,
	Animation,
	Audiovisual,
	Camera,
	Permanence,
	SmokeAndMirrors,
	Focus,
	JuiceResponsibly,
	Count UMETA(Hidden)
};

UENUM(BlueprintType)
enum class ESMAGFShowcaseFeature : uint8
{
	FireAnticipation,
	MovementAcceleration,
	Recoil,
	ImpactResponse,
	HitStop,
	EnemyKnockback,
	PlayerKnockback,
	HitReaction,
	FireAnticipationAnimation,
	WeaponRecoilAnimation,
	CharacterFireReaction,
	EnemyHitReaction,
	EnemyDeathReaction,
	SquashAndStretch,
	OvershootRecovery,
	ProceduralWobble,
	MuzzleFlash,
	ShotLightFlash,
	ProjectileVisualScale,
	ProjectileTrail,
	ShellEjection,
	FireSound,
	LayeredFireSound,
	ImpactFlash,
	ImpactEffect,
	ImpactSound,
	EnemyFlash,
	DeathEffect,
	ExplosionEffect,
	CameraFollowSmoothing,
	AimLookAhead,
	ScreenShake,
	DirectionalCameraKick,
	KillCameraEmphasis,
	KeepEnemyBodies,
	KeepShellCasings,
	KeepImpactEvidence,
	KeepDebris,
	KeepAftermathSmoke,
	ScriptedEnemyWave,
	ForcedDramaticKill,
	ForcedExplosion,
	ShowcaseSlowMotion,
	PredefinedShowcaseSpawns,
	AutomaticEncounterReset,
	DeterministicRandomSeed,
	FreezeEnemies,
	ScriptedShowcaseSequence,
	AimDirectionFraming,
	TargetEmphasis,
	ContextualCameraEmphasis,
	ContextualLightingPulse,
	ContextualPostProcess,
	BackgroundDeemphasis,
	EventImportanceScaling,
	ProjectileReadability,
	ProjectileAnimation,
	Count UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFMasterSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Master")
	bool bShowcaseEnabled = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Master", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float MasterIntensity = 1.f;

	bool operator==(const FSMAGFMasterSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFFeelingSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bFireAnticipationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bMovementAccelerationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bRecoilEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bImpactResponseEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bHitStopEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bEnemyKnockbackEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bPlayerKnockbackEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bHitReactionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float FireAnticipationTime = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float AnticipationScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="10000", UIMin="0", UIMax="10000"))
	float MovementAcceleration = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="10000", UIMin="0", UIMax="10000"))
	float MovementDeceleration = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
	float RecoilDistance = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="45", UIMin="0", UIMax="45"))
	float RecoilRotation = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float RecoilDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float RecoilRecoveryDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
	float ImpactResponseDistance = 3.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float ImpactResponseDuration = 0.06f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float HitStopDuration = 0.04f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float KillHitStopDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float HitStopTimeScale = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="5000", UIMin="0", UIMax="5000"))
	float EnemyKnockbackStrength = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float EnemyKnockbackDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="5000", UIMin="0", UIMax="5000"))
	float PlayerKnockbackStrength = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float PlayerKnockbackRecovery = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float HitReactionDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float HitReactionIntensity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling")
	bool bProjectileReadabilityEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0.5", ClampMax="3.0", UIMin="0.5", UIMax="3.0"))
	float ProjectileVisualScale = 1.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Feeling", meta=(ClampMin="0.0", ClampMax="5.0", UIMin="0.0", UIMax="5.0"))
	float ProjectileGlowFadeDuration = 2.0f;

	bool operator==(const FSMAGFFeelingSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFAnimationSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bFireAnticipationAnimationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bWeaponRecoilAnimationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bCharacterFireReactionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bEnemyHitReactionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bEnemyDeathReactionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bSquashAndStretchEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bOvershootRecoveryEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bProceduralWobbleEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float AnimationIntensity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="0.9", UIMin="0", UIMax="0.9"))
	float SquashAmount = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float StretchAmount = 0.075f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float OvershootAmount = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float WobbleAmount = 0.1f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="30", UIMin="0", UIMax="30"))
	float WobbleFrequency = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float ReactionDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float RecoveryDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	ESMAGFEasingType Easing = ESMAGFEasingType::EaseOut;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation")
	bool bProjectileAnimationEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="1.0", ClampMax="4.0", UIMin="1.0", UIMax="4.0"))
	float ProjectileFlightStretch = 1.6f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="1.0", ClampMax="3.0", UIMin="1.0", UIMax="3.0"))
	float ProjectileFlightGrowth = 1.20f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0.1", ClampMax="1.0", UIMin="0.1", UIMax="1.0"))
	float ProjectileImpactSquash = 0.35f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animation", meta=(ClampMin="0.0", ClampMax="0.5", UIMin="0.0", UIMax="0.5"))
	float ProjectileImpactAnimationDuration = 0.06f;

	bool operator==(const FSMAGFAnimationSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFAudiovisualSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bMuzzleFlashEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bShotLightFlashEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bProjectileVisualScaleEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bProjectileTrailEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bShellEjectionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bFireSoundEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bLayeredFireSoundEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bImpactFlashEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bImpactEffectEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bImpactSoundEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bEnemyFlashEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bDeathEffectEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual")
	bool bExplosionEffectEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float MuzzleFlashScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float MuzzleFlashDuration = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="100000", UIMin="0", UIMax="100000"))
	float ShotLightIntensity = 1000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="5000", UIMin="0", UIMax="5000"))
	float ShotLightRadius = 200.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0.01", ClampMax="10", UIMin="0.01", UIMax="10"))
	float ProjectileVisualScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float TrailDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="120", UIMin="0", UIMax="120"))
	float ShellLifetime = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float FireSoundVolume = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float FireSoundPitchVariation = 0.05f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float ImpactScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float ImpactDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="100000", UIMin="0", UIMax="100000"))
	float ImpactLightIntensity = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float EnemyFlashDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float DeathEffectScale = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Audiovisual", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float ExplosionScale = 1.f;

	bool operator==(const FSMAGFAudiovisualSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFCameraSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bCameraFollowSmoothingEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bAimLookAheadEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bScreenShakeEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bDirectionalCameraKickEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera")
	bool bKillCameraEmphasisEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
	float CameraFollowSpeed = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000"))
	float AimLookAheadDistance = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
	float AimLookAheadInterpolation = 8.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
	float ShakeTranslationAmplitude = 1.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="30", UIMin="0", UIMax="30"))
	float ShakeRotationAmplitude = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
	float ShakeFrequency = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float ShakeDuration = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float ShakeFalloff = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="100", UIMin="0", UIMax="100"))
	float CameraKickDistance = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="30", UIMin="0", UIMax="30"))
	float CameraKickRotation = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float CameraKickDuration = 0.08f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float CameraKickRecovery = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Camera", meta=(ClampMin="0", ClampMax="5", UIMin="0", UIMax="5"))
	float KillCameraDuration = 0.2f;

	bool operator==(const FSMAGFCameraSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFPermanenceSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence")
	bool bKeepEnemyBodiesEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence")
	bool bKeepShellCasingsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence")
	bool bKeepImpactEvidenceEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence")
	bool bKeepDebrisEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence")
	bool bKeepAftermathSmokeEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="300", UIMin="0", UIMax="300"))
	float CorpseLifetime = 20.f; // Total retained lifetime, including dissolve.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="30", UIMin="0", UIMax="30"))
	float CorpseDissolveDuration = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="300", UIMin="0", UIMax="300"))
	float ShellLifetime = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="300", UIMin="0", UIMax="300"))
	float DebrisLifetime = 10.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="300", UIMin="0", UIMax="300"))
	float ImpactEvidenceLifetime = 20.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000"))
	int32 MaxPersistentCorpses = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000"))
	int32 MaxPersistentShells = 100;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Permanence", meta=(ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000"))
	int32 MaxPersistentDebris = 50;

	bool operator==(const FSMAGFPermanenceSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFSmokeAndMirrorsSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bScriptedEnemyWaveEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bForcedDramaticKillEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bForcedExplosionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bShowcaseSlowMotionEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bPredefinedShowcaseSpawnsEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bAutomaticEncounterResetEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bDeterministicRandomSeedEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bFreezeEnemiesEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors")
	bool bScriptedShowcaseSequenceEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors", meta=(ClampMin="1", ClampMax="100", UIMin="1", UIMax="100"))
	int32 WaveSize = 4;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors", meta=(ClampMin="0.01", ClampMax="1", UIMin="0.01", UIMax="1"))
	float SlowMotionTimeScale = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float SlowMotionDuration = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors", meta=(ClampMin="0", ClampMax="120", UIMin="0", UIMax="120"))
	float EncounterResetDelay = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors", meta=(ClampMin="0", ClampMax="2147483647", UIMin="0", UIMax="2147483647"))
	int32 RandomSeed = 1337;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="SmokeAndMirrors", meta=(ClampMin="0", ClampMax="300", UIMin="0", UIMax="300"))
	float SequenceDuration = 10.f;

	bool operator==(const FSMAGFSmokeAndMirrorsSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFFocusSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	bool bAimDirectionFramingEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	bool bTargetEmphasisEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	bool bContextualCameraEmphasisEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	bool bContextualLightingPulseEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	bool bContextualPostProcessEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus")
	bool bBackgroundDeemphasisEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus", meta=(ClampMin="0", ClampMax="2", UIMin="0", UIMax="2"))
	float FocusIntensity = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus", meta=(ClampMin="0", ClampMax="10", UIMin="0", UIMax="10"))
	float FocusDuration = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus", meta=(ClampMin="0", ClampMax="1000", UIMin="0", UIMax="1000"))
	float FocusCameraBias = 50.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus", meta=(ClampMin="0", ClampMax="1", UIMin="0", UIMax="1"))
	float FocusVignetteAmount = 0.2f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Focus", meta=(ClampMin="-5", ClampMax="5", UIMin="-5", UIMax="5"))
	float FocusExposureBias = 0.f;

	bool operator==(const FSMAGFFocusSettings& Other) const;
};

USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFJuiceResponsiblySettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JuiceResponsibly")
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JuiceResponsibly")
	bool bEventImportanceScalingEnabled = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JuiceResponsibly", meta=(ClampMin="0", ClampMax="4", UIMin="0", UIMax="4"))
	float MinorMultiplier = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JuiceResponsibly", meta=(ClampMin="0", ClampMax="4", UIMin="0", UIMax="4"))
	float NormalMultiplier = 1.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JuiceResponsibly", meta=(ClampMin="0", ClampMax="4", UIMin="0", UIMax="4"))
	float MajorMultiplier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="JuiceResponsibly", meta=(ClampMin="0", ClampMax="4", UIMin="0", UIMax="4"))
	float CriticalMultiplier = 2.f;

	float ResolveMultiplier(ESMAGFEventImportance Importance) const;
	bool operator==(const FSMAGFJuiceResponsiblySettings& Other) const;
};

/** Complete value snapshot. Runtime consumers mutate through the world subsystem. */
USTRUCT(BlueprintType)
struct SMAGF_API FSMAGFShowcaseSettings
{
	GENERATED_BODY()
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFMasterSettings Master;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFFeelingSettings Feeling;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFAnimationSettings Animation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFAudiovisualSettings Audiovisual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFCameraSettings Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFPermanenceSettings Permanence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFSmokeAndMirrorsSettings SmokeAndMirrors;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFFocusSettings Focus;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Showcase")
	FSMAGFJuiceResponsiblySettings JuiceResponsibly;

	void Validate();
	bool operator==(const FSMAGFShowcaseSettings& Other) const;
};

