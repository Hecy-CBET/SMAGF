#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SMAGFGameMode.generated.h"

UCLASS()
class SMAGF_API ASMAGFGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASMAGFGameMode();
	class ASMAGFShowcaseDirector* GetShowcaseDirector() const { return ShowcaseDirector; }
private:
	UPROPERTY(Transient) TObjectPtr<class ASMAGFShowcaseDirector> ShowcaseDirector;
protected:
	virtual void BeginPlay() override;
};
