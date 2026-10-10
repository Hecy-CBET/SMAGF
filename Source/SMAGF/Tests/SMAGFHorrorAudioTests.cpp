#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "SMAGFGameMode.h"
#include "Components/AudioComponent.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Sound/SoundWave.h"
#include "SMAGF.h"

// In-memory fixture only. No Content references, mutations or saved defaults.
struct FSMAGFHorrorAudioFixture
{
 static void Run(const TArray<FString>& Args, UWorld* World)
 {
  if (!World || !World->IsGameWorld() || Args.Num()!=1) { return; }
  auto* Mode=World->GetAuthGameMode<ASMAGFGameMode>();
  if (!Mode) { return; }
  if (Args[0]==TEXT("configured") || Args[0]==TEXT("null"))
  {
   Mode->StopHorrorAmbience();
   Mode->bHorrorAmbienceInitialized=false;
   USoundWave* Sound = Args[0]==TEXT("configured") ? NewObject<USoundWave>(Mode) : nullptr;
   Mode->HorrorAmbienceSound=Sound;
   Mode->HorrorMonsterDeathSound=Sound;
   Mode->HorrorAmbienceVolumeMultiplier=.37f;
   Mode->HorrorMonsterDeathVolumeMultiplier=.61f;
   Mode->InitializeHorrorAmbience();
  }
  else if (Args[0]==TEXT("repeat")) { Mode->InitializeHorrorAmbience(); }
  else if (Args[0]==TEXT("stop")) { Mode->StopHorrorAmbience(); }
  else if (Args[0]!=TEXT("status")) { return; }
  const auto* Audio=Mode->HorrorAmbienceComponent.Get();
  UE_LOG(LogSMAGF, Display, TEXT("SMAGF_HORROR_AUDIO_FIXTURE action=%s horror=%d initialized=%d component=%s soundmatch=%d volume=%.6f spatial=%d autodestroy=%d"),
   *Args[0],Mode->IsHorrorMode(),Mode->bHorrorAmbienceInitialized,
   *GetNameSafe(Audio),Audio && Audio->Sound==Mode->HorrorAmbienceSound,
   Audio ? Audio->VolumeMultiplier : -1.f, Audio ? Audio->bAllowSpatialization : -1,
   Audio ? Audio->bAutoDestroy : -1);
 }
};
static FAutoConsoleCommandWithWorldAndArgs GHorrorAudioFixture(
 TEXT("SMAGF.TestHorrorAudio"),TEXT("Development-only audio fixture: configured, null, repeat, stop, status."),
 FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FSMAGFHorrorAudioFixture::Run));
#endif
