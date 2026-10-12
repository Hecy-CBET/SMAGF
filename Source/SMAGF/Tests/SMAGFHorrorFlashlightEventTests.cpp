#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "SMAGFCharacter.h"
#include "Horror/SMAGFFlickeringComponent.h"
#include "SMAGFPlayerController.h"
#include "Enemy/SMAGFEnemyProjectile.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include <limits>

// Real collision fixture. Never changes Content or bypasses hostile-hit acceptance.
static void HorrorFlashlightEventFixture(const TArray<FString>& Args,UWorld* World)
{
 if(!World || !World->IsGameWorld() || Args.Num()!=1) { return; }
 auto* Controller=Cast<ASMAGFPlayerController>(World->GetFirstPlayerController());
 auto* Player=Controller?Cast<ASMAGFCharacter>(Controller->GetPawn()):nullptr;
 if(!Player) { return; }
 if(Args[0]==TEXT("tune"))
 {
  if(auto* Lamp=Player->GetHorrorFlashlight())
  {
   if(auto* Flicker=Lamp->FindComponentByClass<USMAGFFlickeringComponent>())
   {
    // Runtime-only test values; editor property edits would reconstruct SCS components.
    Flicker->FlickerBurstDuration=.4f;Flicker->FlickerBurstDurationJitter=0.f;
    Flicker->FlickerMinIntensityMultiplier=.25f;Flicker->FlickerMaxIntensityMultiplier=.25f;
   }
  }
 }
 else if(Args[0]==TEXT("perspective")) { Controller->HandleShowcaseToggleKey(EKeys::H,true,false); }
 else if(Args[0]==TEXT("restore")) { Controller->HandleShowcaseToggleKey(EKeys::T,true,false); }
 else if(Args[0]==TEXT("reject"))
 {
  Player->FindComponentByClass<USMAGFReactionComponent>()->ReceiveShowcaseHostileHit(FVector(std::numeric_limits<double>::quiet_NaN(),0,0));
 }
 else if(Args[0]==TEXT("hit") || Args[0]==TEXT("miss"))
 {
  const FVector Origin=Player->GetActorLocation()+FVector(150,0,Args[0]==TEXT("miss")?500:0);
  FActorSpawnParameters Params;Params.Owner=Controller;
  Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
  if(auto* Shot=World->SpawnActor<ASMAGFEnemyProjectile>(Origin,FRotator::ZeroRotator,Params))
  { Shot->InitializeFlight(FVector(-1,0,0),650.f); }
 }
}
static FAutoConsoleCommandWithWorldAndArgs GHorrorFlashlightEventFixture(
 TEXT("SMAGF.TestHorrorFlashlightEvent"),TEXT("Editor-only flashlight event fixture: hit, miss, reject, perspective, restore."),
 FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&HorrorFlashlightEventFixture));
#endif
