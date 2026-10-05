#include "Horror/SMAGFFlickeringComponent.h"
#include "Components/LightComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"

namespace
{
	float SafeNonnegative(float Value, float Fallback)
	{
		return FMath::IsFinite(Value) ? FMath::Max(0.f, Value) : Fallback;
	}
}

USMAGFFlickeringComponent::USMAGFFlickeringComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bAutoActivate = true;
}

void USMAGFFlickeringComponent::BeginPlay()
{
	Super::BeginPlay();
	Random.GenerateNewSeed();
	bRuntimeReady = true;
	RefreshControlledLights();
}

void USMAGFFlickeringComponent::RefreshControlledLights()
{
	// Never capture a currently modulated intensity as a new baseline.
	RestoreLights();
	Lights.Reset();
	if (AActor* Owner = GetOwner())
	{
		TInlineComponentArray<ULightComponent*> OwnedLights;
		Owner->GetComponents(OwnedLights);
		for (ULightComponent* Light : OwnedLights)
		{
			if (IsValid(Light)) { Lights.Add({Light, Light->Intensity}); }
		}
	}
	if (Lights.IsEmpty()) { CancelSequence(); }
	else if (bRuntimeReady && IsActive() && bFlickeringEnabled && !bBurstActive
		&& !GetWorld()->GetTimerManager().IsTimerActive(SequenceTimer)) { ScheduleRest(true); }
}

float USMAGFFlickeringComponent::RandomDuration(float Duration, float Jitter)
{
	const float Base = SafeNonnegative(Duration, .01f);
	const float Spread = SafeNonnegative(Jitter, 0.f);
	const float Result = Base + Random.FRandRange(-Spread, Spread);
	return FMath::IsFinite(Result) ? FMath::Max(.01f, Result) : .01f;
}

void USMAGFFlickeringComponent::ScheduleRest(bool bInitialPhase)
{
	if (!bRuntimeReady || !IsActive() || !bFlickeringEnabled || Lights.IsEmpty()) { return; }
	// Stagger startup even when the authored rest jitter is zero.
	const float Rest = RandomDuration(FlickerRestDuration, FlickerRestDurationJitter);
	const float Delay = bInitialPhase ? Random.FRandRange(.01f, Rest) : Rest;
	GetWorld()->GetTimerManager().SetTimer(SequenceTimer, this,
		&ThisClass::TriggerFlickerBurst, Delay, false);
}

void USMAGFFlickeringComponent::TriggerFlickerBurst()
{
	if (!bRuntimeReady || !IsActive()) { return; }
	CancelSequence();
	RefreshControlledLights();
	if (Lights.IsEmpty()) { return; }
	bBurstActive = true;
	BurstEndTime = GetWorld()->GetTimeSeconds() + RandomDuration(FlickerBurstDuration, FlickerBurstDurationJitter);
	StepBurst();
}

void USMAGFFlickeringComponent::StepBurst()
{
	if (!bRuntimeReady || !IsActive() || !bBurstActive) { return; }
	const double Remaining = BurstEndTime - GetWorld()->GetTimeSeconds();
	if (Remaining <= 0.)
	{
		bBurstActive = false;
		RestoreLights();
		ScheduleRest();
		return;
	}
	const float Min = SafeNonnegative(FlickerMinIntensityMultiplier, .15f);
	const float Max = FMath::Max(Min, SafeNonnegative(FlickerMaxIntensityMultiplier, 1.f));
	const float Multiplier = Random.FRandRange(Min, Max);
	bool bHasLight = false;
	for (const auto& Entry : Lights)
	{
		if (ULightComponent* Light = Entry.Light.Get())
		{
			const float Intensity = Entry.Baseline * Multiplier;
			Light->SetIntensity(FMath::IsFinite(Intensity) ? Intensity : Entry.Baseline);
			bHasLight = true;
		}
	}
	if (!bHasLight) { CancelSequence(); return; }
	const float MinStep = FMath::Max(.01f, SafeNonnegative(FlickerStepMin, .04f));
	const float MaxStep = FMath::Max(MinStep, SafeNonnegative(FlickerStepMax, .12f));
	GetWorld()->GetTimerManager().SetTimer(SequenceTimer, this, &ThisClass::StepBurst,
		FMath::Min(Random.FRandRange(MinStep, MaxStep), static_cast<float>(Remaining)), false);
}

void USMAGFFlickeringComponent::RestoreLights()
{
	for (const auto& Entry : Lights)
	{
		if (ULightComponent* Light = Entry.Light.Get()) { Light->SetIntensity(Entry.Baseline); }
	}
}

void USMAGFFlickeringComponent::CancelSequence()
{
	if (GetWorld()) { GetWorld()->GetTimerManager().ClearTimer(SequenceTimer); }
	bBurstActive = false;
	RestoreLights();
}

void USMAGFFlickeringComponent::SetFlickeringEnabled(bool bEnabled)
{
	bFlickeringEnabled = bEnabled;
	if (!bEnabled) { CancelSequence(); }
	else if (bRuntimeReady && IsActive() && !bBurstActive)
	{
		RefreshControlledLights();
	}
}

void USMAGFFlickeringComponent::StopFlickering()
{
	bFlickeringEnabled = false;
	CancelSequence();
}

void USMAGFFlickeringComponent::Activate(bool bReset)
{
	Super::Activate(bReset);
	if (bRuntimeReady && bFlickeringEnabled && !bBurstActive) { RefreshControlledLights(); }
}

void USMAGFFlickeringComponent::Deactivate()
{
	CancelSequence();
	Super::Deactivate();
}

void USMAGFFlickeringComponent::EndPlay(const EEndPlayReason::Type Reason)
{
	bRuntimeReady = false;
	CancelSequence();
	Super::EndPlay(Reason);
}

void USMAGFFlickeringComponent::OnUnregister()
{
	CancelSequence();
	Super::OnUnregister();
}
