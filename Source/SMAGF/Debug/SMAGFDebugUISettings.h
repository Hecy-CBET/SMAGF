#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "SMAGFDebugUISettings.generated.h"

/** Authored tool appearance; never part of showcase presets or runtime tuning. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Debug UI"))
class SMAGF_API USMAGFDebugUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	virtual FName GetCategoryName() const override;
	virtual FName GetSectionName() const override;

	UPROPERTY(Config, EditAnywhere, Category="Game Feel Lab", meta=(ClampMin="0.0", ClampMax="1.0", UIMin="0.0", UIMax="1.0"))
	float DebugPanelBackgroundOpacity = 0.75f;

	/** Safely consume config values without changing the authored CDO. */
	float GetBackgroundOpacity() const;
};
