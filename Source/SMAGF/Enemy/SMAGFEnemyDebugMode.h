#pragma once

#include "CoreMinimal.h"
#include "SMAGFEnemyDebugMode.generated.h"

UENUM(BlueprintType)
enum class ESMAGFEnemyDebugMode : uint8
{
	Aggressive = 0,
	Calm = 1,
	Passive = 2
};
