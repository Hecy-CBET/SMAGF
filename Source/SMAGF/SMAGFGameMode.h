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
protected:
	virtual void BeginPlay() override;
};
