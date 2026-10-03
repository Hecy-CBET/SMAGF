#pragma once
#include "CoreMinimal.h"
#include "Showcase/SMAGFShowcaseTypes.h"

struct FSMAGFPresentationDecision
{
	float Scale = 1.f;
};

/** Pure arithmetic; no world, consumer, event queue or clock ownership. */
namespace SMAGFPresentationPolicy
{
inline bool Normalize(ESMAGFEventImportance Input, ESMAGFEventImportance& Output)
{
	const uint8 Value = static_cast<uint8>(Input);
	if (Value > 3) { return false; }
	Output = Value == 3 ? ESMAGFEventImportance::Major : Input; // Legacy Critical byte.
	return true;
}
inline FSMAGFPresentationDecision Resolve(float Pressure, const FSMAGFJuiceResponsiblySettings& Settings, ESMAGFEventImportance Input)
{
	ESMAGFEventImportance Importance;
	if (!Normalize(Input, Importance)) { return {}; }
	const float Resistance = Importance == ESMAGFEventImportance::Minor ? 1.f
		: Importance == ESMAGFEventImportance::Standard ? .55f : 0.f;
	const float P = FMath::IsFinite(Pressure) ? FMath::Clamp(Pressure, 0.f, 1.f) : 0.f;
	return {FMath::Lerp(1.f, Settings.JuiceMinimumScale, FMath::Clamp(P * Settings.JuiceBudgetStrength * Resistance, 0.f, 1.f))};
}
inline float Cost(ESMAGFEventImportance Input)
{
	ESMAGFEventImportance Importance;
	if (!Normalize(Input, Importance)) { return 0.f; }
	return Importance == ESMAGFEventImportance::Minor ? .2f : Importance == ESMAGFEventImportance::Standard ? .35f : .5f;
}
inline float Decay(float Pressure, float Rate, float DeltaGameTime)
{
	return FMath::Clamp(Pressure - Rate * FMath::Max(0.f, DeltaGameTime), 0.f, 1.f);
}
}
