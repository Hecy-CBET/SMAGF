#pragma once
#include "CoreMinimal.h"

namespace SMAGFSurfaceImpact
{
	inline FVector Normal(const FVector& Surface, const FVector& Travel)
	{
		if (!Surface.ContainsNaN() && !Surface.IsNearlyZero()) { return Surface.GetSafeNormal(); }
		if (!Travel.ContainsNaN() && !Travel.IsNearlyZero()) { return -Travel.GetSafeNormal(); }
		return FVector::UpVector;
	}
	inline FQuat Basis(const FVector& Normal, float RollDegrees = 0.f)
	{
		return FQuat(Normal,FMath::DegreesToRadians(RollDegrees)) * FRotationMatrix::MakeFromX(Normal).ToQuat();
	}
}
