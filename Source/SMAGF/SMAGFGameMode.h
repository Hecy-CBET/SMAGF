#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Enemy/SMAGFEnemyDebugMode.h"
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
	class ASMAGFShowcaseDirector* GetShowcaseDirector() const { return ShowcaseDirector; }
private:
	UPROPERTY(Transient) ESMAGFEnemyDebugMode EnemyDebugMode = ESMAGFEnemyDebugMode::Aggressive;
	UPROPERTY(Transient) TObjectPtr<class ASMAGFShowcaseDirector> ShowcaseDirector;
protected:
	virtual void BeginPlay() override;
};
