#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "SMAGFShowcaseDeveloperSettings.generated.h"

/** Authored startup defaults; runtime tuning never writes this object or config. */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="SMAGF Showcase"))
class SMAGF_API USMAGFShowcaseDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:
	virtual FName GetCategoryName() const override;

	UPROPERTY(Config, EditAnywhere, Category="Startup")
	FSMAGFShowcaseSettings Defaults;

	UPROPERTY(Config, EditAnywhere, Category="Debug")
	bool bEnableQuickRestartShortcut = true;
};
