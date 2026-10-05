#include "SMAGFGameMode.h"
#include "Showcase/SMAGFShowcaseDirector.h"
#include "Enemy/SMAGFEnemyShowcaseComponent.h"
#include "SMAGF.h"
#include "SMAGFCharacter.h"
#include "SMAGFPlayerController.h"
#include "Targets/SMAGFTarget.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Combat/SMAGFHealthComponent.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"

void ASMAGFGameMode::CycleEnemyDebugMode()
{
	switch (EnemyDebugMode)
	{
	case ESMAGFEnemyDebugMode::Aggressive: SetEnemyDebugMode(ESMAGFEnemyDebugMode::Calm); break;
	case ESMAGFEnemyDebugMode::Calm: SetEnemyDebugMode(ESMAGFEnemyDebugMode::Passive); break;
	case ESMAGFEnemyDebugMode::Passive: SetEnemyDebugMode(ESMAGFEnemyDebugMode::Aggressive); break;
	}
}

void ASMAGFGameMode::SetEnemyDebugMode(ESMAGFEnemyDebugMode NewMode)
{
	if (NewMode != ESMAGFEnemyDebugMode::Aggressive && NewMode != ESMAGFEnemyDebugMode::Calm
		&& NewMode != ESMAGFEnemyDebugMode::Passive) { return; }
	if (EnemyDebugMode == NewMode) { return; }
	EnemyDebugMode = NewMode;
	for (TActorIterator<ASMAGFTarget> It(GetWorld()); It; ++It)
	{
		if (!It->IsAliveForFocus()) { continue; }
		if (auto* Behavior = It->FindComponentByClass<USMAGFEnemyShowcaseComponent>())
		{ Behavior->ApplyDebugBehaviorMode(NewMode); }
	}
}

ASMAGFGameMode::ASMAGFGameMode()
{
	DefaultPawnClass = ASMAGFCharacter::StaticClass();
	PlayerControllerClass = ASMAGFPlayerController::StaticClass();
}

void ASMAGFGameMode::BeginPlay()
{
	Super::BeginPlay();
	Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (Showcase.IsValid())
	{ PresetChangedHandle = Showcase->OnPresetChangedNative.AddUObject(this, &ThisClass::ShowcasePresetChanged); }
	if (GetWorld()->IsGameWorld() && !IsValid(ShowcaseDirector))
	{ ShowcaseDirector=GetWorld()->SpawnActor<ASMAGFShowcaseDirector>(); }
	// Flat platforms: the previous 650 cm offsets landed on the sloped ramps.
	const FVector2D Positions[] = { {425.f, 0.f}, {-425.f, 0.f}, {0.f, 425.f}, {0.f, -425.f} };
	const FVector Extent = ASMAGFTarget::GetCollisionExtent();
	FCollisionQueryParams Query(SCENE_QUERY_STAT(SMAGFTargetGround), false);
	const FCollisionObjectQueryParams GroundObjects(ECC_WorldStatic);
	SpawnSlots.SetNum(UE_ARRAY_COUNT(Positions));
	for (int32 SlotIndex = 0; SlotIndex < SpawnSlots.Num(); ++SlotIndex)
	{
		const FVector2D& XY = Positions[SlotIndex];
		FHitResult Ground;
		if (!GetWorld()->LineTraceSingleByObjectType(Ground, FVector(XY.X, XY.Y, 2000.f),
			FVector(XY.X, XY.Y, -2000.f), GroundObjects, Query) || Ground.ImpactNormal.Z < 0.9f)
		{
			UE_LOG(LogSMAGF, Warning, TEXT("Skipping target at %s: no suitable ground."), *XY.ToString());
			continue;
		}
		bool bSupported = true;
		for (float X : {-Extent.X, Extent.X})
		{
			for (float Y : {-Extent.Y, Extent.Y})
			{
				FHitResult Corner;
				const FVector Point(XY.X + X, XY.Y + Y, Ground.ImpactPoint.Z);
				if (!GetWorld()->LineTraceSingleByObjectType(Corner, Point + FVector(0.f, 0.f, 10.f),
					Point - FVector(0.f, 0.f, 10.f), GroundObjects, Query)
					|| Corner.ImpactNormal.Z < 0.9f || FMath::Abs(Corner.ImpactPoint.Z - Point.Z) > 2.f)
				{
					bSupported = false;
				}
			}
		}
		const FVector Location(XY.X, XY.Y, Ground.ImpactPoint.Z + Extent.Z + 2.f);
		if (bSupported)
		{
			SpawnSlots[SlotIndex].OriginalTransform = FTransform(FRotator::ZeroRotator, Location);
			SpawnSlots[SlotIndex].bHasTransform = true;
		}
		if (!bSupported || GetWorld()->OverlapBlockingTestByChannel(Location, FQuat::Identity,
			ECC_WorldDynamic, FCollisionShape::MakeBox(Extent), Query))
		{
			UE_LOG(LogSMAGF, Warning, TEXT("Skipping target at %s: unsupported or obstructed."), *XY.ToString());
			continue;
		}
		if (SpawnShowcaseEnemy(SlotIndex, true))
		{
			UE_LOG(LogSMAGF, Log, TEXT("Baseline target %s at %s (ground Z %.1f)."),
				*SpawnSlots[SlotIndex].Enemy->GetName(), *Location.ToString(), Ground.ImpactPoint.Z);
		}
	}
}

bool ASMAGFGameMode::IsFullShowcaseActive() const
{
	return !bEndingPlay && Showcase.IsValid()
		&& Showcase->GetShowcasePreset() == ESMAGFShowcasePreset::FullShowcase;
}

bool ASMAGFGameMode::SpawnShowcaseEnemy(int32 SlotIndex, bool bInitialSpawn)
{
	if (bEndingPlay || !SpawnSlots.IsValidIndex(SlotIndex)) { return false; }
	auto& Slot = SpawnSlots[SlotIndex];
	if (!Slot.bHasTransform || (Slot.Enemy.IsValid() && Slot.Enemy->IsAliveForFocus())) { return false; }
	FActorSpawnParameters Params;
	// Replacement must use the original transform; presentation debris or a pawn
	// crossing the slot must not adjust it or silently consume the replacement.
	Params.SpawnCollisionHandlingOverride = bInitialSpawn
		? ESpawnActorCollisionHandlingMethod::DontSpawnIfColliding
		: ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	ASMAGFTarget* Target = GetWorld()->SpawnActor<ASMAGFTarget>(
		ASMAGFTarget::StaticClass(), Slot.OriginalTransform, Params);
	if (!Target) { return false; }
	Slot.Enemy = Target;
	Target->FindComponentByClass<USMAGFEnemyShowcaseComponent>()->SetSpawnIndex(SlotIndex);
	Target->OnDestroyed.AddDynamic(this, &ThisClass::ShowcaseEnemyDestroyed);
	return true;
}

void ASMAGFGameMode::ShowcaseEnemyDestroyed(AActor* DestroyedActor)
{
	if (bEndingPlay) { return; }
	for (int32 Index = 0; Index < SpawnSlots.Num(); ++Index)
	{
		auto& Slot = SpawnSlots[Index];
		// OnDestroyed runs while the actor is pending destruction.
		if (Slot.Enemy.Get(true) != DestroyedActor) { continue; }
		Slot.Enemy.Reset();
		GetWorldTimerManager().ClearTimer(Slot.RespawnTimer);
		const auto* Health = DestroyedActor->FindComponentByClass<USMAGFHealthComponent>();
		if (Health && Health->CurrentHealth <= 0.f && IsFullShowcaseActive())
		{
			constexpr float ShowcaseRespawnDelay = 2.f;
			GetWorldTimerManager().SetTimer(Slot.RespawnTimer,
				FTimerDelegate::CreateWeakLambda(this, [this, Index]()
				{
					if (IsFullShowcaseActive()) { SpawnShowcaseEnemy(Index); }
				}), ShowcaseRespawnDelay, false);
		}
		return;
	}
}

void ASMAGFGameMode::ShowcasePresetChanged()
{
	if (IsFullShowcaseActive()) { return; }
	for (auto& Slot : SpawnSlots) { GetWorldTimerManager().ClearTimer(Slot.RespawnTimer); }
}

int32 ASMAGFGameMode::RestoreShowcaseEnemies()
{
	if (bEndingPlay) { return 0; }
	int32 Restored = 0;
	for (int32 Index = 0; Index < SpawnSlots.Num(); ++Index)
	{
		auto& Slot = SpawnSlots[Index];
		if (Slot.Enemy.IsValid() && Slot.Enemy->IsAliveForFocus()) { continue; }
		GetWorldTimerManager().ClearTimer(Slot.RespawnTimer);
		if (SpawnShowcaseEnemy(Index)) { ++Restored; }
	}
	return Restored;
}

void ASMAGFGameMode::EndPlay(const EEndPlayReason::Type Reason)
{
	bEndingPlay = true;
	for (auto& Slot : SpawnSlots)
	{
		GetWorldTimerManager().ClearTimer(Slot.RespawnTimer);
		if (Slot.Enemy.IsValid()) { Slot.Enemy->OnDestroyed.RemoveDynamic(this, &ThisClass::ShowcaseEnemyDestroyed); }
	}
	if (Showcase.IsValid()) { Showcase->OnPresetChangedNative.Remove(PresetChangedHandle); }
	Super::EndPlay(Reason);
}
