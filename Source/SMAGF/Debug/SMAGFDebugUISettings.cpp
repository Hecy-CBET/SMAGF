#include "Debug/SMAGFDebugUISettings.h"

FName USMAGFDebugUISettings::GetCategoryName() const
{
	return TEXT("SMAGF");
}

FName USMAGFDebugUISettings::GetSectionName() const
{
	return TEXT("DebugUI");
}

float USMAGFDebugUISettings::GetBackgroundOpacity() const
{
	return FMath::IsFinite(DebugPanelBackgroundOpacity)
		? FMath::Clamp(DebugPanelBackgroundOpacity, 0.0f, 1.0f) : 0.75f;
}
