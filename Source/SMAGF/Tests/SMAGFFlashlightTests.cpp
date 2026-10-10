#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "SMAGFGameMode.h"
#include "SMAGFCharacter.h"
#include "SMAGFPlayerController.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"

// Test-only injection: no asset/default edits and no production class-path fallback.
struct FSMAGFFlashlightFixture
{
	static void Run(const TArray<FString>& Args, UWorld* World)
	{
		if (!World || !World->IsGameWorld() || Args.Num() != 1) { return; }
		auto* Mode = World->GetAuthGameMode<ASMAGFGameMode>();
		auto* Player = Cast<ASMAGFPlayerController>(World->GetFirstPlayerController());
		if (!Mode || !Player) { return; }
		if (Args[0] == TEXT("configured") || Args[0] == TEXT("null"))
		{
			Mode->HorrorFlashlightClass = Args[0] == TEXT("configured")
				? LoadClass<AActor>(nullptr, TEXT("/Game/TopDown/Horror/BP_Flashlight.BP_Flashlight_C")) : nullptr;
			if (APawn* Pawn = Player->GetPawn()) { Pawn->Destroy(); }
			Mode->RestartPlayer(Player);
		}
		else if (Args[0] == TEXT("aim") && !Player->IsFirstPersonPerspective())
		{
			Player->LastTopDownAimPoint = FVector(500.f, 400.f, 0.f);
			Player->bHasTopDownAimPoint = true;
			Player->UpdateFlashlight();
		}
		else if (Args[0] == TEXT("repossess"))
		{
			APawn* Pawn = Player->GetPawn();
			if (Pawn) { Player->UnPossess(); Player->Possess(Pawn); }
		}
	}
};

static FAutoConsoleCommandWithWorldAndArgs GFlashlightFixture(
	TEXT("SMAGF.TestFlashlight"), TEXT("Editor development fixture: configured, null, aim, repossess."),
	FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FSMAGFFlashlightFixture::Run));
#endif