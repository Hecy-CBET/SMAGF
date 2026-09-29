#pragma once
#include "CoreMinimal.h"
#include "Showcase/SMAGFShowcaseTypes.h"

namespace SMAGFMotion
{
inline FVector PlanarDirection(const FVector& Incoming)
{
	if (Incoming.ContainsNaN()) { return FVector::ForwardVector; }
	const FVector Planar = Incoming.GetSafeNormal2D();
	return Planar.IsNearlyZero() ? FVector::ForwardVector : Planar;
}

inline FVector FallAxis(const FVector& Incoming)
{
	return FVector::CrossProduct(FVector::UpVector, PlanarDirection(Incoming)).GetSafeNormal();
}

// Unit interval input, exact endpoints, bounded overshoot for presentation consumers.
inline float Ease(float Alpha, ESMAGFEasingType Type)
{
	const float T = FMath::IsFinite(Alpha) ? FMath::Clamp(Alpha, 0.f, 1.f) : 0.f;
	if (T == 0.f || T == 1.f) { return T; }
	switch (Type)
	{
	case ESMAGFEasingType::EaseIn: return T * T;
	case ESMAGFEasingType::EaseOut: return 1.f - FMath::Square(1.f - T);
	case ESMAGFEasingType::EaseInOut: return T * T * (3.f - 2.f * T);
	case ESMAGFEasingType::Back:
		return 1.f + 2.70158f * FMath::Pow(T - 1.f, 3.f) + 1.70158f * FMath::Square(T - 1.f);
	case ESMAGFEasingType::Elastic:
		return 1.f + FMath::Pow(2.f, -10.f * T) * FMath::Sin((T * 10.f - 0.75f) * (2.f * PI / 3.f));
	case ESMAGFEasingType::Bounce:
	{
		float U = T;
		if (U < 1.f / 2.75f) { return 7.5625f * U * U; }
		if (U < 2.f / 2.75f) { U -= 1.5f / 2.75f; return 7.5625f * U * U + 0.75f; }
		if (U < 2.5f / 2.75f) { U -= 2.25f / 2.75f; return 7.5625f * U * U + 0.9375f; }
		U -= 2.625f / 2.75f; return 7.5625f * U * U + 0.984375f;
	}
	default: return T;
	}
}

struct FChannel
{
	FVector Offset = FVector::ZeroVector;
	FRotator Rotation = FRotator::ZeroRotator;
	float ScaleZ = 1.f;
	float Elapsed = 0.f, Attack = 0.f, Recovery = 0.f;
	float Frequency = 0.f;
	ESMAGFEasingType Easing = ESMAGFEasingType::EaseOut;
	bool Active() const { return Elapsed < Attack + Recovery; }
	void Advance(float Delta)
	{
		Elapsed += FMath::Max(0.f, Delta);
		if (!Active()) { *this = FChannel(); }
	}
	float Weight() const
	{
		if (!Active()) { return 0.f; }
		const float Value = Attack > 0.f && Elapsed < Attack
			? Ease(Elapsed / Attack, Easing)
			: (Recovery > 0.f ? 1.f - Ease((Elapsed - Attack) / Recovery, Easing) : 0.f);
		return FMath::Clamp(Value, -0.4f, 1.4f);
	}
};
}
