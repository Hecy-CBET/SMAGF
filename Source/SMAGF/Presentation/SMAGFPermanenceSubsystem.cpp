#include "Presentation/SMAGFPermanenceSubsystem.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "GameFramework/Actor.h"
#include "Engine/World.h"

void USMAGFPermanenceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Collection.InitializeDependency<USMAGFShowcaseSubsystem>();
	Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	Handle = Showcase->OnSettingsChangedNative.AddUObject(this, &ThisClass::SettingsChanged);
}
int32 USMAGFPermanenceSubsystem::Budget(ESMAGFShowcaseFeature Feature) const
{
	const auto P = Showcase->GetSettings().Permanence;
	switch (Feature)
	{
	case ESMAGFShowcaseFeature::KeepEnemyBodies: return P.MaxPersistentCorpses;
	case ESMAGFShowcaseFeature::KeepShellCasings: return P.MaxPersistentShells;
	case ESMAGFShowcaseFeature::KeepDebris: return P.MaxPersistentDebris;
	case ESMAGFShowcaseFeature::KeepImpactEvidence: return 128;
	case ESMAGFShowcaseFeature::KeepAftermathSmoke: return 32;
	default: return 0;
	}
}
bool USMAGFPermanenceSubsystem::Retain(AActor* Actor, ESMAGFShowcaseFeature Feature)
{
	if (!IsValid(Actor) || !Showcase.IsValid() || !Showcase->IsFeatureEffectivelyEnabled(Feature) || Budget(Feature) <= 0) { return false; }
	const auto P = Showcase->GetSettings().Permanence;
	float Lifetime = 0.f;
	switch (Feature)
	{
	case ESMAGFShowcaseFeature::KeepEnemyBodies: Lifetime = P.CorpseLifetime; break;
	case ESMAGFShowcaseFeature::KeepShellCasings: Lifetime = P.ShellLifetime; break;
	case ESMAGFShowcaseFeature::KeepDebris: Lifetime = P.DebrisLifetime; break;
	case ESMAGFShowcaseFeature::KeepImpactEvidence: Lifetime = P.ImpactEvidenceLifetime; break;
	case ESMAGFShowcaseFeature::KeepAftermathSmoke: Lifetime = FMath::Min(P.DebrisLifetime, 8.f); break;
	default: return false;
	}
	if (Lifetime <= 0.f) { return false; }
	Entries.RemoveAll([](const FEntry& E) { return !E.Actor.IsValid(); });
	Entries.Add({Actor, Feature});
	Actor->SetLifeSpan(Lifetime);
	SettingsChanged();
	return IsValid(Actor);
}
void USMAGFPermanenceSubsystem::SettingsChanged()
{
	if (!Showcase.IsValid()) { return; }
	// Walk newest first; the oldest entries lose when a live budget shrinks.
	TMap<ESMAGFShowcaseFeature,int32> Counts;
	for (int32 I = Entries.Num()-1; I >= 0; --I)
	{
		const FEntry E = Entries[I];
		if (!E.Actor.IsValid()) { Entries.RemoveAt(I); continue; }
		int32& Count = Counts.FindOrAdd(E.Feature);
		if (!Showcase->IsFeatureEffectivelyEnabled(E.Feature) || ++Count > Budget(E.Feature))
		{
			E.Actor->SetActorHiddenInGame(true);
			E.Actor->Destroy();
			Entries.RemoveAt(I);
		}
	}
}
int32 USMAGFPermanenceSubsystem::GetEvidenceCount(ESMAGFShowcaseFeature Feature) const
{
	int32 Count = 0;
	for (const FEntry& E : Entries) { if (E.Actor.IsValid() && E.Feature == Feature) { ++Count; } }
	return Count;
}
void USMAGFPermanenceSubsystem::Deinitialize()
{
	if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(Handle); }
	Entries.Empty();
	Super::Deinitialize();
}
