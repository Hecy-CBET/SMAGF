#include "Presentation/SMAGFEventPresentationComponent.h"
#include "SMAGF.h"
#include "Presentation/SMAGFPresentationVisual.h"
#include "Presentation/SMAGFPermanenceSubsystem.h"
#include "Camera/SMAGFPlayerCameraManager.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Sound/SoundBase.h"
#include "Engine/World.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	using F = ESMAGFShowcaseFeature;
	using V = ASMAGFPresentationVisual;
	const FLinearColor Warm(1.f,.35f,.04f), Hot(1.f,.85f,.3f);
	void Keep(UWorld* World, V* Actor, F Feature)
	{
		if (!Actor) { return; }
		if (auto* P = World->GetSubsystem<USMAGFPermanenceSubsystem>(); P && P->Retain(Actor,Feature)) { Actor->SetPersistent(); }
		else if (Actor->Feature != F::ShellEjection) { Actor->Destroy(); }
	}
	FVector Ground(UWorld* World, FVector Position, AActor* Ignore)
	{
		FHitResult Hit;
		FCollisionQueryParams Query(SCENE_QUERY_STAT(SMAGFEvidence),false,Ignore);
		return World->LineTraceSingleByObjectType(Hit,Position,Position-FVector(0,0,1000),FCollisionObjectQueryParams(ECC_WorldStatic),Query)
			? Hit.ImpactPoint + FVector(0,0,1) : Position;
	}
}
USMAGFEventPresentationComponent::USMAGFEventPresentationComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Variant_TwinStick/M_Glow.M_Glow"));
	Glow = Material.Object;
	static ConstructorHelpers::FObjectFinder<USoundBase> Primary(TEXT("/Game/Audio/SFX_Player_Fire_Primary.SFX_Player_Fire_Primary"));
	static ConstructorHelpers::FObjectFinder<USoundBase> Layer(TEXT("/Game/Audio/SFX_Player_Fire_Layer.SFX_Player_Fire_Layer"));
	static ConstructorHelpers::FObjectFinder<USoundBase> Impact(TEXT("/Game/Audio/SFX_Projectile_Impact_Enemy.SFX_Projectile_Impact_Enemy"));
	FireSound = Primary.Object;
	FireLayer = Layer.Object;
	ImpactSound = Impact.Object;
}
void USMAGFEventPresentationComponent::BeginPlay()
{
	Super::BeginPlay(); Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	Visual = GetOwner()->FindComponentByClass<UStaticMeshComponent>();
	if (Visual) { for (int32 I=0; I<Visual->GetNumMaterials(); ++I) { OriginalMaterials.Add(Visual->GetMaterial(I)); } }
	if (Showcase.IsValid()) { Handle=Showcase->OnSettingsChangedNative.AddUObject(this,&ThisClass::SettingsChanged); }
}
void USMAGFEventPresentationComponent::Shot(FVector Muzzle, FVector Direction)
{
	if (!Showcase.IsValid()) { return; }
	const auto A = Showcase->GetSettings().Audiovisual;
	const float Gain = Showcase->GetMasterIntensity();
	if (V* MuzzleVisual=V::Spawn(GetWorld(),Muzzle,F::MuzzleFlash,A.MuzzleFlashDuration,FVector(.28f,.12f,.12f)*A.MuzzleFlashScale,Hot,Direction*30.f))
	{ MuzzleVisual->SetActorRotation(Direction.Rotation()); }
	V::Spawn(GetWorld(),Muzzle,F::ShotLightFlash,A.MuzzleFlashDuration,FVector::ZeroVector,Hot,FVector::ZeroVector,A.ShotLightIntensity,A.ShotLightRadius);
	const FVector Side = FVector::CrossProduct(FVector::UpVector,Direction);
	V* Shell = V::Spawn(GetWorld(),Muzzle,F::ShellEjection,A.ShellLifetime,FVector(.075f,.025f,.025f),Warm,Side*100.f+FVector(0,0,35));
	Keep(GetWorld(),Shell,F::KeepShellCasings);
	// Pitch sequence is repeatable and independent of gameplay RNG.
	const float Unit = float((ShotSequence++ * 73u + 19u) % 101u) / 50.f - 1.f;
	const float Pitch = FMath::Clamp(1.f + Unit*A.FireSoundPitchVariation,.5f,2.f);
	if (Showcase->IsFeatureEffectivelyEnabled(F::FireSound) && FireSound && Gain>0.f)
	{
		const float Volume = FMath::Min(4.f,A.FireSoundVolume*Gain);
		UE_LOG(LogSMAGF, Verbose, TEXT("SMAGF_AUDIO_REQUEST FireSound volume=%.6f pitch=%.6f position=%s asset=%s"), Volume, Pitch, *Muzzle.ToCompactString(), *FireSound->GetPathName());
		UGameplayStatics::PlaySoundAtLocation(this,FireSound,Muzzle,Volume,Pitch);
	}
	if (Showcase->IsFeatureEffectivelyEnabled(F::LayeredFireSound) && FireLayer && Gain>0.f)
	{
		const float Volume = FMath::Min(2.f,A.FireSoundVolume*Gain*.35f);
		UE_LOG(LogSMAGF, Verbose, TEXT("SMAGF_AUDIO_REQUEST LayeredFireSound volume=%.6f pitch=%.6f position=%s asset=%s"), Volume, Pitch*.85f, *Muzzle.ToCompactString(), *FireLayer->GetPathName());
		UGameplayStatics::PlaySoundAtLocation(this,FireLayer,Muzzle,Volume,Pitch*.85f);
	}
	if (auto* Camera = Cast<ASMAGFPlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(this,0))) { Camera->NotifyShot(Direction); }
}
void USMAGFEventPresentationComponent::Hit(FVector Position,FVector Direction)
{
	if (!Showcase.IsValid()) { return; }
	const auto A=Showcase->GetSettings().Audiovisual;
	V::Spawn(GetWorld(),Position,F::ImpactFlash,A.ImpactDuration,FVector(.25f)*A.ImpactScale,Hot,FVector::ZeroVector,A.ImpactLightIntensity,180.f);
	if (Showcase->IsFeatureEffectivelyEnabled(F::ImpactSound) && ImpactSound && Showcase->GetMasterIntensity()>0.f)
	{
		UE_LOG(LogSMAGF, Verbose, TEXT("SMAGF_AUDIO_REQUEST ImpactSound volume=%.6f pitch=1.000000 position=%s asset=%s"), Showcase->GetMasterIntensity(), *Position.ToCompactString(), *ImpactSound->GetPathName());
		UGameplayStatics::PlaySoundAtLocation(this,ImpactSound,Position,Showcase->GetMasterIntensity());
	}
	if (Visual && !GetOwner()->IsActorBeingDestroyed() && Showcase->IsFeatureEffectivelyEnabled(F::EnemyFlash) && A.EnemyFlashDuration>0.f && Showcase->GetMasterIntensity()>0.f)
	{
		if (!Flash) { Flash=UMaterialInstanceDynamic::Create(Glow,this); }
		if (Flash)
		{
			Flash->SetVectorParameterValue(TEXT("Emissive Color"),Hot);
			Flash->SetScalarParameterValue(TEXT("Emissive Strength"),8.f*Showcase->GetMasterIntensity());
			for (int32 I=0;I<OriginalMaterials.Num();++I) { Visual->SetMaterial(I,Flash); }
			FlashRemaining=A.EnemyFlashDuration;
		}
	}
}
void USMAGFEventPresentationComponent::Death(FVector Position,FVector Direction)
{
	if (!Showcase.IsValid()) { return; }
	RestoreFlash(); // DeathVisual copies original materials, never a temporary flash MID.
	const auto S=Showcase->GetSettings();
	for (int32 I=0;I<8;++I)
	{
		const FVector Spread=FRotator(0,I*45.f,0).RotateVector(Direction);
		V::Spawn(GetWorld(),Position,F::DeathEffect,.45f,FVector(.18f)*S.Audiovisual.DeathEffectScale,Warm,Spread*160.f+FVector(0,0,70));
	}
	const FVector Floor=Ground(GetWorld(),Position,GetOwner());
	for (int32 I=0;I<3;++I)
	{
		const FVector Spread=FRotator(0,I*120.f,0).RotateVector(Direction);
		V* Debris=V::Spawn(GetWorld(),Floor+FVector(0,0,35),F::KeepDebris,S.Permanence.DebrisLifetime,FVector(.12f,.08f,.07f),FLinearColor(.12f,.08f,.04f),Spread*65.f+FVector(0,0,15));
		Keep(GetWorld(),Debris,F::KeepDebris);
	}
	if (auto* Camera=Cast<ASMAGFPlayerCameraManager>(UGameplayStatics::GetPlayerCameraManager(this,0))) { Camera->NotifyKill(Position); }
}
void USMAGFEventPresentationComponent::RestoreFlash()
{
	if (Visual && FlashRemaining>0.f) { for (int32 I=0;I<OriginalMaterials.Num();++I) { Visual->SetMaterial(I,OriginalMaterials[I]); } }
	FlashRemaining=0.f;
}
void USMAGFEventPresentationComponent::SettingsChanged()
{
	if (!Showcase.IsValid() || !Showcase->IsFeatureEffectivelyEnabled(F::EnemyFlash) || Showcase->GetMasterIntensity()<=0.f) { RestoreFlash(); }
}
void USMAGFEventPresentationComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function)
{
	Super::TickComponent(Dt,Type,Function);
	if (FlashRemaining>0.f) { if (FlashRemaining<=Dt) { RestoreFlash(); } else { FlashRemaining-=Dt; } }
}
void USMAGFEventPresentationComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	RestoreFlash();
	if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(Handle); }
	Super::EndPlay(Reason);
}
