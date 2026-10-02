#include "Showcase/SMAGFShowcaseTypes.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFMouseFocusSettingsTest,"SMAGF.Focus.MouseSettings",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSMAGFMouseFocusSettingsTest::RunTest(const FString& Parameters)
{
 FSMAGFShowcaseSettings Settings;
 TestEqual(TEXT("Acquire default"),Settings.Focus.FocusAcquireRadius,.12f);
 TestEqual(TEXT("Release default"),Settings.Focus.FocusReleaseRadius,.18f);
 TestEqual(TEXT("Advantage default"),Settings.Focus.FocusSwitchAdvantage,.04f);
 TestEqual(TEXT("Serialized duration keeps grace default"),Settings.Focus.FocusDuration,.2f);
 Settings.Focus.FocusAcquireRadius=.5f;Settings.Focus.FocusReleaseRadius=.01f;
 Settings.Focus.FocusSwitchAdvantage=-1.f;Settings.Validate();
 TestEqual(TEXT("Release cannot fall below acquire"),Settings.Focus.FocusReleaseRadius,.5f);
 TestEqual(TEXT("Advantage cannot be negative"),Settings.Focus.FocusSwitchAdvantage,0.f);
 Settings.Focus.FocusAcquireRadius=std::numeric_limits<float>::quiet_NaN();
 Settings.Focus.FocusReleaseRadius=std::numeric_limits<float>::infinity();
 Settings.Focus.FocusSwitchAdvantage=-std::numeric_limits<float>::infinity();
 Settings.Validate();
 TestEqual(TEXT("Nonfinite acquire defaults"),Settings.Focus.FocusAcquireRadius,.12f);
 TestEqual(TEXT("Nonfinite release defaults"),Settings.Focus.FocusReleaseRadius,.18f);
 TestEqual(TEXT("Nonfinite advantage defaults"),Settings.Focus.FocusSwitchAdvantage,.04f);
 return true;
}
#endif
