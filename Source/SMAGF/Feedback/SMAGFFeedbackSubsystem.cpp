#include "Feedback/SMAGFFeedbackSubsystem.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/WorldSettings.h"
#include "HAL/PlatformTime.h"

bool USMAGFFeedbackSubsystem::DoesSupportWorldType(EWorldType::Type Type) const
{
	return Type == EWorldType::Game || Type == EWorldType::PIE;
}
void USMAGFFeedbackSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<USMAGFShowcaseSubsystem>();
	Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	SettingsHandle = Showcase->OnSettingsChangedNative.AddUObject(this, &USMAGFFeedbackSubsystem::SettingsChanged);
	TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateUObject(this, &USMAGFFeedbackSubsystem::TickRealTime));
}
void USMAGFFeedbackSubsystem::Deinitialize()
{
	FTSTicker::RemoveTicker(TickerHandle);
	ClearHitStop();
	if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(SettingsHandle); }
	Super::Deinitialize();
}
void USMAGFFeedbackSubsystem::SettingsChanged()
{
	if (!Showcase.IsValid() || !Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::HitStop)) { ClearHitStop(); }
}
void USMAGFFeedbackSubsystem::NotifyDamagingHit(bool bKilled)
{
	if (!Showcase.IsValid()) { return; }
	const FSMAGFFeelingSettings Settings = Showcase->GetSettings().Feeling;
	RequestHitStop(bKilled ? Settings.KillHitStopDuration : Settings.HitStopDuration, Settings.HitStopTimeScale);
}
int32 USMAGFFeedbackSubsystem::RequestHitStop(float Duration, float TimeScale)
{
	if (!GetWorld() || GetWorld()->IsPaused() || !Showcase.IsValid()
		|| !Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::HitStop)
		|| !FMath::IsFinite(Duration) || Duration <= 0.f || !FMath::IsFinite(TimeScale)) { return INDEX_NONE; }
	// Expire stale requests before capturing or extending a batch.
	TickRealTime(0.f);
	if (!bCaptured)
	{
		CapturedDilation = GetWorld()->GetWorldSettings()->TimeDilation;
		bCaptured = true;
	}
	NextId = NextId == MAX_int32 ? 1 : NextId + 1;
	Requests.Add({NextId, FPlatformTime::Seconds() + FMath::Min(Duration, 1.f), FMath::Clamp(TimeScale, 0.01f, 1.f)});
	ApplyStrongest();
	return NextId;
}
void USMAGFFeedbackSubsystem::ApplyStrongest()
{
	if (!bCaptured || !GetWorld()) { return; }
	float Scale = CapturedDilation;
	for (const FRequest& Request : Requests) { Scale = FMath::Min(Scale, Request.Scale); }
	GetWorld()->GetWorldSettings()->SetTimeDilation(Scale);
}
bool USMAGFFeedbackSubsystem::TickRealTime(float DeltaTime)
{
	const double Now = FPlatformTime::Seconds();
	Requests.RemoveAll([Now](const FRequest& R) { return R.Expires <= Now; });
	if (Requests.IsEmpty()) { ClearHitStop(); }
	else { ApplyStrongest(); }
	return true;
}
void USMAGFFeedbackSubsystem::ClearHitStop()
{
	Requests.Reset();
	if (bCaptured && GetWorld() && GetWorld()->GetWorldSettings())
	{
		GetWorld()->GetWorldSettings()->SetTimeDilation(CapturedDilation);
	}
	bCaptured = false;
}
