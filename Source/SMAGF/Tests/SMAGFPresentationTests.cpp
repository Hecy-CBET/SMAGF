#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Presentation/SMAGFSurfaceImpact.h"
#include "Debug/SMAGFShowcaseDebugPanel.h"
#include "Debug/SMAGFShowcaseLayerIndicator.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Framework/Application/SlateApplication.h"
#include "Layout/Children.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFSurfaceTest,"SMAGF.Presentation.SurfaceBasisAndUI",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSMAGFSurfaceTest::RunTest(const FString& Parameters)
{
	for (const FVector N : {FVector(1,0,0),FVector(-1,0,0),FVector(0,1,0),FVector(0,-1,0),FVector(0,0,1),FVector(1,2,3).GetSafeNormal()})
	{
		for (float Roll : {0.f,137.5f,275.f})
		{
			const auto Basis=SMAGFSurfaceImpact::Basis(N,Roll);
			TestTrue(TEXT("Compression/projector X aligns with surface"),Basis.GetAxisX().Equals(N,.0001));
			TestTrue(TEXT("Tangent orthogonal"),FMath::Abs(FVector::DotProduct(Basis.GetAxisY(),N))<.0001);
			TestFalse(TEXT("Finite surface basis"),Basis.ContainsNaN());
		}
		TestTrue(TEXT("Invalid normal fallback opposes travel"),SMAGFSurfaceImpact::Normal(FVector::ZeroVector,-N).Equals(N));
	}
	TestTrue(TEXT("All-zero input safe"),SMAGFSurfaceImpact::Normal(FVector::ZeroVector,FVector::ZeroVector).Equals(FVector::UpVector));
	TestTrue(TEXT("NaN normal fallback"),SMAGFSurfaceImpact::Normal(FVector(std::numeric_limits<double>::quiet_NaN(),0,0),FVector::ForwardVector).Equals(-FVector::ForwardVector));
	TestTrue(TEXT("Infinity normal fallback"),SMAGFSurfaceImpact::Normal(FVector(std::numeric_limits<double>::infinity(),0,0),FVector::ForwardVector).Equals(-FVector::ForwardVector));
	const FSMAGFCameraSettings Camera;
	TestEqual(TEXT("Translation default"),Camera.ShakeTranslationAmplitude,1.2f);
	TestEqual(TEXT("Rotation default"),Camera.ShakeRotationAmplitude,.4f);
	TestEqual(TEXT("Kick distance unchanged"),Camera.CameraKickDistance,5.f);
	TestEqual(TEXT("Kick rotation unchanged"),Camera.CameraKickRotation,1.f);
	if (FSlateApplication::IsInitialized())
	{
		auto Panel=SNew(SSMAGFShowcaseDebugPanel).Controller(nullptr).Subsystem(nullptr);
		int32 Count=0;
		TFunction<void(TSharedRef<SWidget>)> Visit=[&](TSharedRef<SWidget> Widget)
		{
			if (Widget->GetType()==FName(TEXT("SExpandableArea")))
			{
				++Count;
				auto Area=StaticCastSharedRef<SExpandableArea>(Widget);
				TestFalse(TEXT("Group initially collapsed"),Area->IsExpanded());
				Area->SetExpanded(true); TestTrue(TEXT("Group expands"),Area->IsExpanded());
				Area->SetExpanded(false); TestFalse(TEXT("Group collapses"),Area->IsExpanded());
			}
			FChildren* Children=Widget->GetChildren();
			for(int32 I=0;I<Children->Num();++I) { Visit(Children->GetChildAt(I)); }
		};
		Visit(Panel);
		TestEqual(TEXT("Eight explicit sections"),Count,8);
		auto Indicator=SNew(SSMAGFShowcaseLayerIndicator).Subsystem(nullptr);
		TestEqual(TEXT("Initial label"),Indicator->GetLabel().ToString(),FString(TEXT("BASELINE")));
		TestTrue(TEXT("HUD never captures input"),Indicator->GetVisibility()==EVisibility::HitTestInvisible);
	}
	else { AddError(TEXT("Slate is required for this UI test")); }
	return true;
}
#endif
