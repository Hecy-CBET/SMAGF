#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Feedback/SMAGFMotion.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFMotionTest, "SMAGF.Animation.EasingAndLifetime",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSMAGFMotionTest::RunTest(const FString& Parameters)
{
	for (uint8 Mode = 0; Mode < static_cast<uint8>(ESMAGFEasingType::Count); ++Mode)
	{
		const auto Type = static_cast<ESMAGFEasingType>(Mode);
		TestEqual(TEXT("Exact start"), SMAGFMotion::Ease(0.f, Type), 0.f);
		TestEqual(TEXT("Exact end"), SMAGFMotion::Ease(1.f, Type), 1.f);
		for (int32 Index = 0; Index <= 100; ++Index)
		{
			const float Value = SMAGFMotion::Ease(Index / 100.f, Type);
			TestTrue(TEXT("Finite bounded easing"), FMath::IsFinite(Value) && Value >= -0.4f && Value <= 1.4f);
		}
		for (float Attack : {0.f, 0.15f})
		{
			for (float Recovery : {0.f, 0.2f})
			{
				SMAGFMotion::FChannel Channel;
				Channel.Attack = Attack; Channel.Recovery = Recovery; Channel.Easing = Type;
				TestTrue(TEXT("Finite zero-duration handling"), FMath::IsFinite(Channel.Weight()));
				Channel.Advance(1.f);
				TestFalse(TEXT("Terminates"), Channel.Active());
				TestEqual(TEXT("Exact terminal neutral"), Channel.Weight(), 0.f);
			}
		}
	}
	for (const FVector& Direction : {FVector(1, 0, 0), FVector(-1, 0, 0), FVector(0, 1, 0), FVector(0, -1, 0)})
	{
		const FVector Axis = SMAGFMotion::FallAxis(Direction);
		const FVector TiltedUp = FQuat(Axis, FMath::DegreesToRadians(45.f)).RotateVector(FVector::UpVector);
		TestTrue(TEXT("Fall tilts away from source"), FVector::DotProduct(TiltedUp, Direction) > 0.7f);
		TestTrue(TEXT("Orthogonal axis"), FMath::Abs(FVector::DotProduct(Axis, Direction)) < KINDA_SMALL_NUMBER);
		TestTrue(TEXT("Opposite inputs have opposite axes"), Axis.Equals(-SMAGFMotion::FallAxis(-Direction)));
		for (float Yaw : {0.f, 37.f, 90.f, 180.f})
		{
			const FQuat Parent = FRotator(0.f, Yaw, 0.f).Quaternion();
			const FVector LocalAxis = Parent.Inverse().RotateVector(Axis);
			const FQuat Authored = FRotator(13.f, -29.f, 7.f).Quaternion();
			const FQuat LocalDelta(LocalAxis, FMath::DegreesToRadians(45.f));
			const FQuat Composed = Parent * LocalDelta * Authored;
			const FQuat Expected = FQuat(Axis, FMath::DegreesToRadians(45.f)) * Parent * Authored;
			TestTrue(TEXT("Parent and authored rotation preserved"), Composed.Equals(Expected, 0.0001f));
		}
	}
	TestTrue(TEXT("Orthogonal shots have orthogonal axes"), FMath::Abs(FVector::DotProduct(
		SMAGFMotion::FallAxis(FVector::ForwardVector), SMAGFMotion::FallAxis(FVector::RightVector))) < KINDA_SMALL_NUMBER);
	for (const FVector& Degenerate : {FVector::ZeroVector, FVector(0, 0, 1), FVector(1.e-12, 0, 1.e-12)})
	{
		TestTrue(TEXT("Safe degenerate fallback"), SMAGFMotion::PlanarDirection(Degenerate).Equals(FVector::ForwardVector));
		TestFalse(TEXT("Finite fallback axis"), SMAGFMotion::FallAxis(Degenerate).ContainsNaN());
	}
	return true;
}
#endif
