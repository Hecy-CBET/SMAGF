#pragma once
#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "SMAGFPermanenceSubsystem.generated.h"

/** Only owns lifetime/budget policy, never gameplay or effect creation. */
UCLASS()
class SMAGF_API USMAGFPermanenceSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	bool Retain(AActor* Actor, ESMAGFShowcaseFeature Feature);
	float NextImpactRoll() { return FMath::Fmod(float(++ImpactSequence)*137.50776f,360.f); }
	UFUNCTION(BlueprintPure, Category="SMAGF|Permanence")
	int32 GetEvidenceCount(ESMAGFShowcaseFeature Feature) const;
protected:
	virtual bool DoesSupportWorldType(EWorldType::Type Type) const override { return Type == EWorldType::Game || Type == EWorldType::PIE; }
private:
	uint32 ImpactSequence = 0;
	struct FEntry { TWeakObjectPtr<AActor> Actor; ESMAGFShowcaseFeature Feature; };
	TArray<FEntry> Entries;
	TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle Handle;
	void SettingsChanged();
	int32 Budget(ESMAGFShowcaseFeature Feature) const;
};
