#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Horror/SMAGFFlashlightMotion.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFFlashlightLeadTest, "SMAGF.Horror.FlashlightLead",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSMAGFFlashlightLeadTest::RunTest(const FString& Parameters)
{
 const auto& Settings = *GetDefault<ASMAGFGameMode>();
 int32 Checks = 0;
 auto Check = [this, &Checks](const TCHAR* Name, bool Value) { ++Checks; TestTrue(Name, Value); };
 for (float Dt : {1.f / 30.f, 1.f / 120.f})
 {
  for (const FVector2D Rate : {FVector2D(90,0), FVector2D(-90,0), FVector2D(0,90), FVector2D(0,-90)})
  {
   FSMAGFFlashlightMotion Motion;
   FRotator View = FRotator::ZeroRotator;
   Motion.Advance(View, 0.f, Settings);
   for (int32 I=0; I<FMath::RoundToInt(1.f/Dt); ++I)
   {
    View.Yaw += Rate.X*Dt; View.Pitch += Rate.Y*Dt;
    Motion.Advance(View, Dt, Settings);
   }
   Check(TEXT("Lead follows angular motion sign"), FVector2D::DotProduct(Motion.Lead, Rate)>0.);
   Check(TEXT("Authored bounds"), FMath::Abs(Motion.Lead.X)<=6. && FMath::Abs(Motion.Lead.Y)<=4.);
   const double Before=Motion.Lead.Size();
   Motion.Advance(View, Dt, Settings);
   Check(TEXT("Recenter decreases without instant snap"), Motion.Lead.Size()>0. && Motion.Lead.Size()<Before);
   double Previous=Motion.Lead.Size(); bool Monotonic=true;
   for(int32 I=0; I<FMath::RoundToInt(3.f/Dt); ++I)
   {
    Motion.Advance(View, Dt, Settings);
    Monotonic &= Motion.Lead.Size()<=Previous; Previous=Motion.Lead.Size();
   }
   Check(TEXT("Recenter has no oscillation"), Monotonic);
   Check(TEXT("Settles to zero"), Motion.Lead.IsNearlyZero(1.e-4));
  }
 }
 FSMAGFFlashlightMotion Slow, Fast;
 Slow.Advance(FRotator::ZeroRotator,0.f,Settings); Fast.Advance(FRotator::ZeroRotator,0.f,Settings);
 for(int32 I=1; I<=30; ++I) { Slow.Advance(FRotator(0,I*3.f,0),1.f/30.f,Settings); }
 for(int32 I=1; I<=120; ++I) { Fast.Advance(FRotator(0,I*.75f,0),1.f/120.f,Settings); }
 Check(TEXT("Equal-time frame-rate consistency"), (Slow.Lead-Fast.Lead).Size()<.001);
 const FVector2D Saved=Slow.Lead;
 Slow.Advance(FRotator(0,100,0),0.f,Settings);
 Check(TEXT("Zero-time refresh does not advance"), Slow.Lead.Equals(Saved));
 Slow.Reset(); Slow.Advance(FRotator(45,-150,0),1.f/60.f,Settings);
 Check(TEXT("Reset seeds without spike"), Slow.Lead.IsZero());
 Slow.Reset(); Slow.Advance(FRotator(0,179,0),0.f,Settings); Slow.Advance(FRotator(0,-179,0),.1f,Settings);
 Check(TEXT("Wraparound is below threshold"), Slow.Lead.IsZero());
 Slow.Advance(FRotator(0,-170,0),.1f,Settings);
 Check(TEXT("Normalized positive yaw"), Slow.Lead.X>0.);
 const FVector ViewLocation(100,200,300); const FRotator ViewRotation(20,40,0);
 Slow.Reset();
 const FTransform Transform=Slow.MakeTransform(ViewLocation,ViewRotation,Settings);
 const FVector Origin=ViewLocation+ViewRotation.RotateVector(FVector(0,12,-10));
 const FVector Target=ViewLocation+ViewRotation.Vector()*500.f;
 Check(TEXT("Camera-local source offset"), Transform.GetLocation().Equals(Origin,.001));
 Check(TEXT("Converges to center target"), FVector::DotProduct(Transform.GetRotation().GetForwardVector(),(Target-Origin).GetSafeNormal())>.99999);
 Check(TEXT("Offset convergence is not parallel"), !Transform.GetRotation().GetForwardVector().Equals(ViewRotation.Vector(),.001));
 Slow.Advance(ViewRotation,-1.f,Settings);
 Check(TEXT("Invalid delta resets safely"), Slow.Lead.IsZero());
 AddInfo(FString::Printf(TEXT("FlashlightLead assertions: %d"),Checks));
 return true;
}
#endif