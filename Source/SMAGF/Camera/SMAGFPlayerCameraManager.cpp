#include "Camera/SMAGFPlayerCameraManager.h"
#include "Showcase/SMAGFShowcaseDirector.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Feedback/SMAGFMotion.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "SceneView.h"
#include "UnrealClient.h"
#include "Engine/World.h"

namespace
{
bool HasFocusPostProcessContribution(bool bEnabled, float Amount)
{
	// A zero-weight effect must not override a volume's authored value with the pawn POV default.
	return bEnabled && Amount != 0.f;
}
}

void ASMAGFPlayerCameraManager::NotifyShot(FVector Direction)
{
	auto* S = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (!S) { return; }
	ShotSettings = S->GetSettings().Camera;
	if (!S->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::ScreenShake)) { ShotSettings.ShakeDuration = 0.f; }
	if (!S->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::DirectionalCameraKick)) { ShotSettings.CameraKickDuration = ShotSettings.CameraKickRecovery = 0.f; }
	ShotDirection = SMAGFMotion::PlanarDirection(Direction);
	ShotGain = S->GetMasterIntensity();
	ShotAge = 0.f; // Bounded single channel; no accumulation under sustained fire.
}

void ASMAGFPlayerCameraManager::NotifyKill(FVector Position)
{
	auto* S = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (!S || !S->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::KillCameraEmphasis)) { return; }
	KillSettings = S->GetSettings().Camera;
	KillGain = S->GetMasterIntensity();
	KillPosition = Position;
	KillAge = 0.f;
}

void ASMAGFPlayerCameraManager::SettingsChanged()
{
	if (!Showcase.IsValid()) { return; }
	if (!Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::ScreenShake)) { ShotSettings.ShakeDuration = 0.f; }
	if (!Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::DirectionalCameraKick)) { ShotSettings.CameraKickDuration = ShotSettings.CameraKickRecovery = 0.f; }
	if (!Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::KillCameraEmphasis)) { KillAge = 100.f; }
	if (!Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::AimLookAhead)) { Look = FVector::ZeroVector; }
	if (Showcase->GetMasterIntensity() <= 0.f) { ShotAge = KillAge = 100.f; Look = FVector::ZeroVector; }
	if (bHaveView) { UpdateCamera(0.f); }
}

void ASMAGFPlayerCameraManager::UpdateViewTargetInternal(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTargetInternal(OutVT, DeltaTime);
	LogicalPOV = OutVT.POV;
	if (!Showcase.IsValid())
	{
		Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
		if (Showcase.IsValid()) { SettingsHandle = Showcase->OnSettingsChangedNative.AddUObject(this, &ThisClass::SettingsChanged); }
	}
	if (!bHaveView) { Follow = LogicalPOV.Location; bHaveView = true; }
	if (!Showcase.IsValid()) { return; }
	const auto S = Showcase->GetSettings();
	const auto Enabled = [this](ESMAGFShowcaseFeature F) { return Showcase->IsFeatureEffectivelyEnabled(F); };
	const float Gain = S.Master.MasterIntensity;
	const float Dt = GetWorld()->IsPaused() ? 0.f : FMath::Max(0.f, DeltaTime);
	if(!FocusDirector.IsValid()) { FocusDirector=ASMAGFShowcaseDirector::Get(GetWorld()); }
	if(FocusDirector.IsValid()) { FocusDirector->UpdateMouseFocus(this,PCOwner,Dt); }
	ShotAge += Dt; KillAge += Dt;
	Follow = Enabled(ESMAGFShowcaseFeature::CameraFollowSmoothing) && Gain > 0.f
		? FMath::VInterpTo(Follow, LogicalPOV.Location, Dt, S.Camera.CameraFollowSpeed) : LogicalPOV.Location;
	Follow = LogicalPOV.Location + (Follow - LogicalPOV.Location).GetClampedToMaxSize(250.f);
	FVector Aim = PCOwner && PCOwner->GetPawn() ? PCOwner->GetPawn()->GetActorForwardVector() : FVector::ZeroVector;
	Look = Enabled(ESMAGFShowcaseFeature::AimLookAhead) && Gain > 0.f
		? FMath::VInterpTo(Look, Aim * S.Camera.AimLookAheadDistance * Gain, Dt, S.Camera.AimLookAheadInterpolation) : FVector::ZeroVector;
	FVector Offset = Follow - LogicalPOV.Location + Look.GetClampedToMaxSize(2000.f);
	const bool FocusEnabled=Showcase->IsShowcaseEnabled() && Showcase->IsGroupEnabled(ESMAGFShowcaseGroup::Focus);
	const float Strength=FocusEnabled && FocusDirector.IsValid()?FocusDirector->GetFocusStrength():0.f;
	const float FocusGain=FMath::Clamp(Strength*S.Focus.FocusIntensity*Gain,0.f,1.f);
	FVector DesiredFocus=FVector::ZeroVector;
	if(FocusDirector.IsValid() && PCOwner && PCOwner->GetPawn())
	{
		if(Enabled(ESMAGFShowcaseFeature::ContextualCameraEmphasis))
		{ DesiredFocus=(FocusDirector->GetFocusPoint()-PCOwner->GetPawn()->GetActorLocation())*S.Focus.FocusFramingWeight; }
		if(Enabled(ESMAGFShowcaseFeature::AimDirectionFraming)) { DesiredFocus+=Aim*S.Focus.FocusCameraBias; }
	}
	DesiredFocus=DesiredFocus.GetClampedToMaxSize(S.Focus.FocusMaxCameraOffset)*FocusGain;
	FocusOffset=FMath::VInterpTo(FocusOffset,DesiredFocus,Dt,S.Focus.FocusCameraInterpSpeed);
	const float DesiredZoom=Enabled(ESMAGFShowcaseFeature::ContextualCameraEmphasis)?S.Focus.FocusFOVReduction*FocusGain:0.f;
	FocusZoom=FMath::FInterpTo(FocusZoom,DesiredZoom,Dt,S.Focus.FocusCameraInterpSpeed);
	if(Gain<=0.f || S.Focus.FocusIntensity<=0.f) { FocusOffset=FVector::ZeroVector;FocusZoom=0.f; }
	if(DesiredFocus.IsNearlyZero() && FocusOffset.Size()<.01f) { FocusOffset=FVector::ZeroVector; }
	if(DesiredZoom==0.f && FocusZoom<.001f) { FocusZoom=0.f; }
	FocusOffset=FocusOffset.GetClampedToMaxSize(S.Focus.FocusMaxCameraOffset);
	FocusZoom=FMath::Clamp(FocusZoom,0.f,S.Focus.FocusFOVReduction);
	Offset+=FocusOffset;
	OutVT.POV.FOV=FocusZoom>0.f?FMath::Clamp(LogicalPOV.FOV-FocusZoom,5.f,170.f):LogicalPOV.FOV;
	// Separate cached overrides preserve the authored POV and its blend weight.
	FocusPostProcess=FPostProcessSettings();FocusPostProcessWeight=FocusGain;
	if(FocusGain>0.f)
	{
		auto& FocusPP=FocusPostProcess;
		if(HasFocusPostProcessContribution(Enabled(ESMAGFShowcaseFeature::BackgroundDeemphasis),S.Focus.FocusVignetteAmount))
		{ FocusPP.bOverride_VignetteIntensity=true;FocusPP.VignetteIntensity=FMath::Clamp(LogicalPOV.PostProcessSettings.VignetteIntensity+FMath::Min(S.Focus.FocusVignetteAmount,.35f),0.f,1.f); }
		if(HasFocusPostProcessContribution(Enabled(ESMAGFShowcaseFeature::ContextualPostProcess),S.Focus.FocusExposureBias))
		{ FocusPP.bOverride_AutoExposureBias=true;FocusPP.AutoExposureBias=LogicalPOV.PostProcessSettings.AutoExposureBias+FMath::Clamp(S.Focus.FocusExposureBias,-.25f,.25f); }
	}
	FRotator Rotation = FRotator::ZeroRotator;
	if (Enabled(ESMAGFShowcaseFeature::KillCameraEmphasis) && KillAge < KillSettings.KillCameraDuration)
	{
		const float Weight = 1.f - KillAge / KillSettings.KillCameraDuration;
		const FVector Pivot = PCOwner && PCOwner->GetPawn() ? PCOwner->GetPawn()->GetActorLocation() : LogicalPOV.Location;
		Offset += (KillPosition - Pivot).GetSafeNormal2D() * (20.f * KillGain * Weight);
	}
	const float Total = ShotSettings.CameraKickDuration + ShotSettings.CameraKickRecovery;
	if (Enabled(ESMAGFShowcaseFeature::DirectionalCameraKick) && ShotAge < Total)
	{
		const float Weight = ShotAge < ShotSettings.CameraKickDuration ? 1.f
			: 1.f - (ShotAge - ShotSettings.CameraKickDuration) / FMath::Max(SMALL_NUMBER, ShotSettings.CameraKickRecovery);
		Offset -= ShotDirection * (ShotSettings.CameraKickDistance * ShotGain * Weight);
		const FVector Local = LogicalPOV.Rotation.UnrotateVector(ShotDirection);
		Rotation += FRotator(Local.X, Local.Y, 0.f) * (ShotSettings.CameraKickRotation * ShotGain * Weight);
	}
	if (Enabled(ESMAGFShowcaseFeature::ScreenShake) && ShotAge < ShotSettings.ShakeDuration)
	{
		const float Weight = FMath::Pow(1.f - ShotAge / ShotSettings.ShakeDuration, ShotSettings.ShakeFalloff);
		const float Phase = ShotAge * ShotSettings.ShakeFrequency * 2.f * PI;
		const FVector Wave(FMath::Sin(Phase), FMath::Sin(Phase * 1.37f), FMath::Sin(Phase * 1.71f));
		Offset += Wave * (ShotSettings.ShakeTranslationAmplitude * ShotGain * Weight);
		Rotation += FRotator(Wave.Y, Wave.Z, Wave.X) * (ShotSettings.ShakeRotationAmplitude * ShotGain * Weight);
	}
	OutVT.POV.Location = LogicalPOV.Location + Offset.GetClampedToMaxSize(2400.f);
	OutVT.POV.Rotation = LogicalPOV.Rotation + FRotator(FMath::Clamp(Rotation.Pitch,-15.f,15.f), FMath::Clamp(Rotation.Yaw,-15.f,15.f), FMath::Clamp(Rotation.Roll,-15.f,15.f));
}

void ASMAGFPlayerCameraManager::ApplyCameraModifiers(float DeltaTime,FMinimalViewInfo& InOutPOV)
{
 Super::ApplyCameraModifiers(DeltaTime,InOutPOV); // Base clears cached post-process blends.
 if(FocusPostProcessWeight>0.f) { AddCachedPPBlend(FocusPostProcess,FocusPostProcessWeight,VTBlendOrder_Override); }
}

bool ASMAGFPlayerCameraManager::DeprojectLogicalScreen(FVector2D Screen, FVector& Origin, FVector& Direction) const
{
	const ULocalPlayer* LP = PCOwner ? PCOwner->GetLocalPlayer() : nullptr;
	if (!bHaveView || !LP || !LP->ViewportClient || !LP->ViewportClient->Viewport) { return false; }
	FSceneViewProjectionData Data;
	if (!LP->GetProjectionData(LP->ViewportClient->Viewport, Data)) { return false; }
	return DeprojectLogicalRect(Screen, Data.GetViewRect(), LP->AspectRatioAxisConstraint, Origin, Direction);
}

bool ASMAGFPlayerCameraManager::DeprojectLogicalViewport(FVector2D Screen, FIntPoint ViewportSize, FVector& Origin, FVector& Direction) const
{
	const ULocalPlayer* LP = PCOwner ? PCOwner->GetLocalPlayer() : nullptr;
	return DeprojectLogicalRect(Screen,FIntRect(FIntPoint::ZeroValue,ViewportSize),
		LP ? LP->AspectRatioAxisConstraint : TEnumAsByte<EAspectRatioAxisConstraint>(AspectRatio_MaintainXFOV),Origin,Direction);
}

bool ASMAGFPlayerCameraManager::DeprojectLogicalRect(FVector2D Screen, const FIntRect& Rect,
	TEnumAsByte<EAspectRatioAxisConstraint> Constraint, FVector& Origin, FVector& Direction) const
{
	if (!bHaveView || Rect.Width()<=0 || Rect.Height()<=0) { return false; }
	FSceneViewProjectionData Data;
	Data.SetViewRectangle(Rect);
	FMinimalViewInfo View = LogicalPOV;
	Data.ViewOrigin = View.Location;
	Data.ViewRotationMatrix = FInverseRotationMatrix(View.Rotation) * FMatrix(
		FPlane(0,0,1,0), FPlane(1,0,0,0), FPlane(0,1,0,0), FPlane(0,0,0,1));
	FMinimalViewInfo::CalculateProjectionMatrixGivenViewRectangle(View, Constraint, Rect, Data);
	FSceneView::DeprojectScreenToWorld(Screen, Data.GetConstrainedViewRect(), Data.ComputeViewProjectionMatrix().InverseFast(), Origin, Direction);
	return !Origin.ContainsNaN() && !Direction.ContainsNaN();
}

void ASMAGFPlayerCameraManager::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(SettingsHandle); }
	Super::EndPlay(Reason);
}

bool ASMAGFPlayerCameraManager::ProjectWorldToLogicalScreen(FVector WorldPosition,FVector2D& Screen) const
{
	const ULocalPlayer* LP=PCOwner?PCOwner->GetLocalPlayer():nullptr;
	if(!bHaveView || !LP || !LP->ViewportClient || !LP->ViewportClient->Viewport) { return false; }
	FSceneViewProjectionData Data;
	if(!LP->GetProjectionData(LP->ViewportClient->Viewport,Data)) { return false; }
	return ProjectLogicalRect(WorldPosition,Data.GetViewRect(),LP->AspectRatioAxisConstraint,Screen);
}

bool ASMAGFPlayerCameraManager::ProjectWorldToLogicalViewport(FVector WorldPosition,FIntPoint ViewportSize,FVector2D& Screen) const
{
	const ULocalPlayer* LP=PCOwner?PCOwner->GetLocalPlayer():nullptr;
	return ProjectLogicalRect(WorldPosition,FIntRect(FIntPoint::ZeroValue,ViewportSize),
		LP?LP->AspectRatioAxisConstraint:TEnumAsByte<EAspectRatioAxisConstraint>(AspectRatio_MaintainXFOV),Screen);
}

bool ASMAGFPlayerCameraManager::ProjectLogicalRect(FVector WorldPosition,const FIntRect& Rect,
	TEnumAsByte<EAspectRatioAxisConstraint> Constraint,FVector2D& Screen) const
{
	if(!bHaveView || Rect.Width()<=0 || Rect.Height()<=0 || WorldPosition.ContainsNaN()) { return false; }
	FSceneViewProjectionData Data;
	Data.SetViewRectangle(Rect);
	FMinimalViewInfo View=LogicalPOV;
	Data.ViewOrigin=View.Location;
	Data.ViewRotationMatrix=FInverseRotationMatrix(View.Rotation)*FMatrix(
		FPlane(0,0,1,0),FPlane(1,0,0,0),FPlane(0,1,0,0),FPlane(0,0,0,1));
	FMinimalViewInfo::CalculateProjectionMatrixGivenViewRectangle(View,Constraint,Rect,Data);
	if(!FSceneView::ProjectWorldToScreen(WorldPosition,Data.GetConstrainedViewRect(),Data.ComputeViewProjectionMatrix(),Screen)
		|| Screen.ContainsNaN()) { return false; }
	const FIntRect ViewRect=Data.GetConstrainedViewRect();
	return Screen.X>=ViewRect.Min.X && Screen.Y>=ViewRect.Min.Y && Screen.X<ViewRect.Max.X && Screen.Y<ViewRect.Max.Y;
}

#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFFocusNeutralPostProcessTest,"SMAGF.Focus.NeutralPostProcess",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSMAGFFocusNeutralPostProcessTest::RunTest(const FString& Parameters)
{
	TestFalse(TEXT("Default exposure does not override authored exposure"),HasFocusPostProcessContribution(true,FSMAGFFocusSettings().FocusExposureBias));
	TestFalse(TEXT("Zero vignette does not override authored vignette"),HasFocusPostProcessContribution(true,0.f));
	TestFalse(TEXT("Disabled feature does not override"),HasFocusPostProcessContribution(false,.2f));
	TestTrue(TEXT("Positive authored contribution remains available"),HasFocusPostProcessContribution(true,.2f));
	TestTrue(TEXT("Negative exposure contribution remains available"),HasFocusPostProcessContribution(true,-.2f));
	return true;
}
#endif
