#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Enemy/SMAGFEnemyDebugMode.h"
#include "TimerManager.h"
#include "SMAGFGameMode.generated.h"

UCLASS()
class SMAGF_API ASMAGFGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASMAGFGameMode();
	UFUNCTION(BlueprintPure, Category="SMAGF|Debug")
	ESMAGFEnemyDebugMode GetEnemyDebugMode() const { return EnemyDebugMode; }
	UFUNCTION(BlueprintCallable, Category="SMAGF|Debug")
	void CycleEnemyDebugMode();
	UFUNCTION(BlueprintCallable, Category="SMAGF|Debug")
	void SetEnemyDebugMode(ESMAGFEnemyDebugMode NewMode);
	UFUNCTION(BlueprintCallable, Category="SMAGF|Debug")
	int32 RestoreShowcaseEnemies();
	class ASMAGFShowcaseDirector* GetShowcaseDirector() const { return ShowcaseDirector; }
private:
	struct FShowcaseSpawnSlot
	{
		FTransform OriginalTransform;
		TWeakObjectPtr<class ASMAGFTarget> Enemy;
		FTimerHandle RespawnTimer;
		bool bHasTransform = false;
	};
	TArray<FShowcaseSpawnSlot> SpawnSlots;
	TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle PresetChangedHandle;
	bool bEndingPlay = false;
	bool SpawnShowcaseEnemy(int32 SlotIndex, bool bInitialSpawn = false);
	bool IsFullShowcaseActive() const;
	void ShowcasePresetChanged();
	UFUNCTION() void ShowcaseEnemyDestroyed(AActor* DestroyedActor);
	UPROPERTY(Transient) ESMAGFEnemyDebugMode EnemyDebugMode = ESMAGFEnemyDebugMode::Aggressive;
	UPROPERTY(Transient) TObjectPtr<class ASMAGFShowcaseDirector> ShowcaseDirector;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
};
