#if WITH_EDITOR && !UE_BUILD_SHIPPING
#include "AIController.h"
#include "Enemy/SMAGFEnemyShowcaseComponent.h"
#include "Targets/SMAGFTarget.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "TimerManager.h"
#include "SMAGF.h"

namespace SMAGFNavigationDiagnostics
{
struct FSampleState
{
	FVector RequestedVelocity = FVector::ZeroVector;
	FTimerHandle Timer;
	int32 Remaining = 24;
};

static void Sample(const TArray<FString>& Args, UWorld* World)
{
	if (!World || !World->IsGameWorld()) { return; }
	const int32 Index = Args.Num() ? FCString::Atoi(*Args[0]) : 0;
	for (TActorIterator<ASMAGFTarget> It(World); It; ++It)
	{
		auto* Behavior = It->FindComponentByClass<USMAGFEnemyShowcaseComponent>();
		if (!Behavior || Behavior->GetSpawnIndex() != Index) { continue; }
		auto* Controller = Cast<AAIController>(It->GetController());
		auto* Following = Controller ? Controller->GetPathFollowingComponent() : nullptr;
		auto* Movement = Cast<UFloatingPawnMovement>(It->GetMovementComponent());
		// Never replace an existing observer/processor or run overlapping samplers.
		if (!Following || !Movement || Following->PostProcessMove.IsBound()) { return; }
		const auto State = MakeShared<FSampleState>();
		Following->PostProcessMove.BindLambda([State](UPathFollowingComponent*, FVector& Requested)
		{
			State->RequestedVelocity = Requested; // Observe only; never modify navigation input.
		});
		const TWeakObjectPtr<ASMAGFTarget> Target(*It);
		const TWeakObjectPtr<UPathFollowingComponent> PathFollowing(Following);
		const TWeakObjectPtr<UFloatingPawnMovement> NavMovement(Movement);
		const TWeakObjectPtr<UWorld> SampleWorld(World);
		World->GetTimerManager().SetTimer(State->Timer, [State, Target, PathFollowing, NavMovement, SampleWorld]()
		{
			if (Target.IsValid() && PathFollowing.IsValid() && NavMovement.IsValid())
			{
				const auto Path = PathFollowing->GetPath();
				FVector Current = FVector::ZeroVector, Next = FVector::ZeroVector;
				if (Path.IsValid())
				{
					const auto& Points = Path->GetPathPoints();
					const int32 A = static_cast<int32>(PathFollowing->GetCurrentPathIndex());
					const int32 B = static_cast<int32>(PathFollowing->GetNextPathIndex());
					if (Points.IsValidIndex(A)) { Current = Points[A].Location; }
					if (Points.IsValidIndex(B)) { Next = Points[B].Location; }
				}
				FNavLocation Projected;
				auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(SampleWorld.Get());
				const bool Found = Nav && Nav->ProjectPointToNavigation(Target->GetNavAgentLocation(), Projected,
					FVector(35,35,100), &NavMovement->GetNavAgentPropertiesRef());
				UE_LOG(LogSMAGF, Verbose, TEXT("NAV_SAMPLE actor=%s agent=%s current=%s next=%s requested=%s velocity=%s projected=%s found=%d status=%d"),
					*Target->GetActorLocation().ToString(), *Target->GetNavAgentLocation().ToString(),
					*Current.ToString(), *Next.ToString(), *State->RequestedVelocity.ToString(),
					*NavMovement->Velocity.ToString(), *(Found ? Projected.Location : FVector::ZeroVector).ToString(),
					Found, static_cast<int32>(PathFollowing->GetStatus()));
			}
			else { State->Remaining = 0; }
			if (--State->Remaining <= 0)
			{
				if (PathFollowing.IsValid()) { PathFollowing->PostProcessMove.Unbind(); }
				if (SampleWorld.IsValid()) { SampleWorld->GetTimerManager().ClearTimer(State->Timer); }
			}
		}, .25f, true);
		return;
	}
}

static FAutoConsoleCommandWithWorldAndArgs Command(
	TEXT("SMAGF.NavigationDiagnostics"),
	TEXT("Opt-in editor diagnostic: sample one SpawnIndex at Verbose every .25s for 6 game seconds."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&Sample));
}
#endif
