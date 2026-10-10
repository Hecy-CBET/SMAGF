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
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror")
	bool bHorrorMode = false;
	UFUNCTION(BlueprintPure, Category="SMAGF|Horror")
	bool IsHorrorMode() const { return bHorrorMode; }
	/** Presentation class supplied by the Horror GameMode asset; no native asset fallback. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror")
	TSubclassOf<AActor> HorrorFlashlightClass = nullptr;
	TSubclassOf<AActor> GetHorrorFlashlightClass() const { return HorrorFlashlightClass; }
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(Units="cm"))
	FVector HorrorFlashlightFirstPersonViewOffset = FVector(0.f,12.f,-10.f);
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(ClampMin="1", ClampMax="10000", Units="cm"))
	float HorrorFlashlightConvergenceDistance = 500.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(ClampMin="0", ClampMax="10000"))
	float HorrorFlashlightLeadAngularSpeedThreshold = 25.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(ClampMin="0", ClampMax="1", Units="s"))
	float HorrorFlashlightLeadTime = .035f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(ClampMin="0", ClampMax="45"))
	float HorrorFlashlightMaxLeadYaw = 6.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(ClampMin="0", ClampMax="45"))
	float HorrorFlashlightMaxLeadPitch = 4.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(ClampMin="0.01", ClampMax="100"))
	float HorrorFlashlightLeadInterpSpeed = 12.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="SMAGF|Horror|Flashlight", meta=(ClampMin="0.01", ClampMax="100"))
	float HorrorFlashlightReturnInterpSpeed = 6.f;

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
	bool IsAutomaticRespawnEligible() const;
	void ShowcasePresetChanged();
	UFUNCTION() void ShowcaseEnemyDestroyed(AActor* DestroyedActor);
	UPROPERTY(Transient) ESMAGFEnemyDebugMode EnemyDebugMode = ESMAGFEnemyDebugMode::Aggressive;
	UPROPERTY(Transient) TObjectPtr<class ASMAGFShowcaseDirector> ShowcaseDirector;
protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
};
