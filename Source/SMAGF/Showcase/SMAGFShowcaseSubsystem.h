#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "SMAGFShowcaseSubsystem.generated.h"

class USMAGFShowcaseSubsystem;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSMAGFShowcaseSettingsChanged, USMAGFShowcaseSubsystem*, Subsystem);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSMAGFShowcasePresetChanged, ESMAGFShowcasePreset, Preset);
DECLARE_MULTICAST_DELEGATE(FSMAGFShowcaseNativeChanged);

/** The only mutable showcase state for a Game/PIE world. No effect consumers yet. */
UCLASS()
class SMAGF_API USMAGFShowcaseSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool SetShowcasePreset(ESMAGFShowcasePreset Preset);
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	ESMAGFShowcasePreset GetShowcasePreset() const { return ActivePreset; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	ESMAGFShowcasePreset GetLastNamedShowcasePreset() const { return LastNamedPreset; }
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	bool HasCustomShowcaseSnapshot() const { return bHasCustomSnapshot; }
	/** Custom returns its saved snapshot, or current settings if none exists. */
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	FSMAGFShowcaseSettings ResolvePreset(ESMAGFShowcasePreset Preset) const;
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool NextShowcasePreset();
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool PreviousShowcasePreset();
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool ResetCurrentGroupToDefaults(ESMAGFShowcaseGroup Group);
	UPROPERTY(BlueprintAssignable, Category="SMAGF|Showcase")
	FSMAGFShowcasePresetChanged OnShowcasePresetChanged;
	// Slate uses weak shared-pointer bindings; the Blueprint delegates remain available.
	FSMAGFShowcaseNativeChanged OnSettingsChangedNative;
	FSMAGFShowcaseNativeChanged OnPresetChangedNative;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	FSMAGFShowcaseSettings GetSettings() const { return Settings; }

	/** Validate and atomically replace a complete snapshot; emits at most one notification. */
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool ApplySettings(const FSMAGFShowcaseSettings& NewSettings);

	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool ResetToDefaults();
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool SetShowcaseEnabled(bool bEnabled);
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	bool IsShowcaseEnabled() const { return Settings.Master.bShowcaseEnabled; }
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool SetMasterIntensity(float Intensity);
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	float GetMasterIntensity() const { return Settings.Master.MasterIntensity; }
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool SetGroupEnabled(ESMAGFShowcaseGroup Group, bool bEnabled);
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	bool IsGroupEnabled(ESMAGFShowcaseGroup Group) const;
	UFUNCTION(BlueprintCallable, Category="SMAGF|Showcase")
	bool SetFeatureEnabled(ESMAGFShowcaseFeature Feature, bool bEnabled);
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	bool IsFeatureSelectedEnabled(ESMAGFShowcaseFeature Feature) const;
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	bool IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature Feature) const;
	UFUNCTION(BlueprintPure, Category="SMAGF|Showcase")
	ESMAGFShowcaseGroup GetFeatureGroup(ESMAGFShowcaseFeature Feature) const;

	UPROPERTY(BlueprintAssignable, Category="SMAGF|Showcase")
	FSMAGFShowcaseSettingsChanged OnSettingsChanged;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
private:
	bool CommitState(const FSMAGFShowcaseSettings& Candidate, ESMAGFShowcasePreset Preset, bool bSaveCustom);
	UPROPERTY(Transient)
	ESMAGFShowcasePreset ActivePreset = ESMAGFShowcasePreset::Baseline;
	UPROPERTY(Transient)
	ESMAGFShowcasePreset LastNamedPreset = ESMAGFShowcasePreset::Baseline;
	UPROPERTY(Transient)
	FSMAGFShowcaseSettings CustomSnapshot;
	UPROPERTY(Transient)
	bool bHasCustomSnapshot = false;
	bool bCommitting = false;
	UPROPERTY(Transient)
	FSMAGFShowcaseSettings Settings;
	UPROPERTY(Transient)
	FSMAGFShowcaseSettings StartupDefaults;
};
