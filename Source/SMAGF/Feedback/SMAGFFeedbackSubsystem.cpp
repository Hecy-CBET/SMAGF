#include "Feedback/SMAGFFeedbackSubsystem.h"
#include "Feedback/SMAGFPresentationPolicy.h"
#include "SMAGF.h"
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
	if (!IsPressureEnabled()) { PresentationPressure = 0.f; LastScale = 1.f; }
}
void USMAGFFeedbackSubsystem::NotifyDamagingHit(bool bKilled)
{
	NotifyContact(1.f, bKilled);
}
bool USMAGFFeedbackSubsystem::IsPressureEnabled() const
{
	return Showcase.IsValid() && Showcase->IsFeatureEffectivelyEnabled(ESMAGFShowcaseFeature::EventImportanceScaling);
}
void USMAGFFeedbackSubsystem::Tick(float DeltaTime)
{
	if (!IsPressureEnabled()) { PresentationPressure = 0.f; return; }
	if (!GetWorld() || GetWorld()->IsPaused() || !FMath::IsFinite(DeltaTime)) { return; }
	PresentationPressure = SMAGFPresentationPolicy::Decay(PresentationPressure, Showcase->GetSettings().JuiceResponsibly.JuicePressureDecay, DeltaTime);
}
void USMAGFFeedbackSubsystem::NotifyContact(float AcceptedDamage, bool bKilled)
{
	const bool bDamage = FMath::IsFinite(AcceptedDamage) && AcceptedDamage > 0.f;
	const auto Importance = bDamage ? (bKilled ? ESMAGFEventImportance::Major : ESMAGFEventImportance::Standard) : ESMAGFEventImportance::Minor;
	ProcessPresentationEvent(Importance, bDamage ? (bKilled ? TEXT("Kill") : TEXT("EnemyHit")) : TEXT("EnvironmentImpact"), bDamage, bKilled);
}
void USMAGFFeedbackSubsystem::NotifyPlayerHit()
{
	ProcessPresentationEvent(ESMAGFEventImportance::Major, TEXT("PlayerHit"), false, false);
}
void USMAGFFeedbackSubsystem::ProcessPresentationEvent(ESMAGFEventImportance Importance, const TCHAR* Kind, bool bDamage, bool bKilled)
{
	if (!Showcase.IsValid() || !GetWorld() || GetWorld()->IsPaused()) { return; }
	const auto Settings = Showcase->GetSettings();
	const bool bEnabled = IsPressureEnabled();
	if (!bEnabled) { PresentationPressure = 0.f; }
	const float Before = PresentationPressure;
	const FSMAGFPresentationDecision Decision = bEnabled ? SMAGFPresentationPolicy::Resolve(Before, Settings.JuiceResponsibly, Importance) : FSMAGFPresentationDecision{};
	LastScale = Decision.Scale;
	LastImportance = Importance;
	EventCount = EventCount == MAX_int32 ? 1 : EventCount + 1;
	LastAuthoredDuration = LastResolvedDuration = 0.f;
	if (bDamage)
	{
		LastAuthoredDuration = bKilled ? Settings.Feeling.KillHitStopDuration : Settings.Feeling.HitStopDuration;
		LastResolvedDuration = LastAuthoredDuration * Decision.Scale;
		RequestHitStop(LastResolvedDuration, Settings.Feeling.HitStopTimeScale);
	}
	// Commit only after all consumers have received this decision. No consumer resolves again.
	if (bEnabled) { PresentationPressure = FMath::Clamp(Before + SMAGFPresentationPolicy::Cost(Importance), 0.f, 1.f); }
	UE_LOG(LogSMAGF, Verbose, TEXT("JUICE_EVENT count=%d game_time=%.6f event_kind=%s importance=%d pressure_before=%.6f scale=%.6f pressure_after=%.6f authored_hitstop=%.6f resolved_hitstop=%.6f"),
		EventCount, GetWorld()->GetTimeSeconds(), Kind, static_cast<int32>(Importance), Before, Decision.Scale, PresentationPressure, LastAuthoredDuration, LastResolvedDuration);
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
