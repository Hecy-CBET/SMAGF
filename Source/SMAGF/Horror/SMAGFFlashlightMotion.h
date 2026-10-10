#pragma once
#include "CoreMinimal.h"
#include "SMAGFGameMode.h"

/** Controller-local presentation math; never modifies gameplay view or aim. X=Yaw, Y=Pitch. */
struct FSMAGFFlashlightMotion
{
	FVector2D Lead = FVector2D::ZeroVector;
	FRotator Previous = FRotator::ZeroRotator;
	bool bHavePrevious = false;

	void Reset() { Lead = FVector2D::ZeroVector; bHavePrevious = false; }
	static float Bounded(float Value, float Fallback, float Min, float Max)
	{
		return FMath::Clamp(FMath::IsFinite(Value) ? Value : Fallback, Min, Max);
	}
	void Advance(const FRotator& View, float Dt, const ASMAGFGameMode& S)
	{
		if (View.ContainsNaN() || !FMath::IsFinite(Dt) || Dt < 0.f) { Reset(); return; }
		if (!bHavePrevious) { Previous = View; Lead = FVector2D::ZeroVector; bHavePrevious = true; return; }
		if (Dt <= SMALL_NUMBER) { return; }
		const FVector2D Velocity(FMath::FindDeltaAngleDegrees(Previous.Yaw, View.Yaw) / Dt,
			FMath::FindDeltaAngleDegrees(Previous.Pitch, View.Pitch) / Dt);
		Previous = View;
		const double MaxYaw = Bounded(S.HorrorFlashlightMaxLeadYaw, 6.f, 0.f, 45.f);
		const double MaxPitch = Bounded(S.HorrorFlashlightMaxLeadPitch, 4.f, 0.f, 45.f);
		const float Threshold = Bounded(S.HorrorFlashlightLeadAngularSpeedThreshold, 25.f, 0.f, 10000.f);
		const bool bMoving = Velocity.Size() > Threshold;
		const float LeadTime = Bounded(S.HorrorFlashlightLeadTime, .035f, 0.f, 1.f);
		const FVector2D Desired = bMoving ? FVector2D(FMath::Clamp(Velocity.X * LeadTime, -MaxYaw, MaxYaw),
			FMath::Clamp(Velocity.Y * LeadTime, -MaxPitch, MaxPitch)) : FVector2D::ZeroVector;
		const float Speed = bMoving ? Bounded(S.HorrorFlashlightLeadInterpSpeed, 12.f, .01f, 100.f)
			: Bounded(S.HorrorFlashlightReturnInterpSpeed, 6.f, .01f, 100.f);
		Lead += (Desired - Lead) * (1. - FMath::Exp(-Speed * Dt));
		Lead.X = FMath::Clamp(Lead.X, -MaxYaw, MaxYaw);
		Lead.Y = FMath::Clamp(Lead.Y, -MaxPitch, MaxPitch);
		if (!bMoving && Lead.IsNearlyZero(1.e-4)) { Lead = FVector2D::ZeroVector; }
	}
	FTransform MakeTransform(const FVector& ViewLocation, const FRotator& View, const ASMAGFGameMode& S) const
	{
		const FVector Offset = (S.HorrorFlashlightFirstPersonViewOffset.ContainsNaN()
			? FVector(0.f, 12.f, -10.f) : S.HorrorFlashlightFirstPersonViewOffset).GetClampedToMaxSize(200.f);
		const FVector Origin = ViewLocation + View.RotateVector(Offset);
		const float Distance = Bounded(S.HorrorFlashlightConvergenceDistance, 500.f, 1.f, 10000.f);
		const FRotator LedView(View.Pitch + Lead.Y, View.Yaw + Lead.X, 0.f);
		const FVector Target = ViewLocation + LedView.Vector() * Distance;
		return FTransform((Target - Origin).Rotation(), Origin);
	}
};