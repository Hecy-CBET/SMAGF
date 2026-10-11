#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "SMAGFPlayerController.h"
#include "AudioDevice.h"
#include "AudioThread.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "SMAGF.h"

struct FSMAGFAudioToggleFixture
{
 static void Run(const TArray<FString>& Args, UWorld* World)
 {
  if (!World || !World->IsGameWorld() || Args.Num()!=1) { return; }
  auto* Player=Cast<ASMAGFPlayerController>(World->GetFirstPlayerController());
  if (!Player) { return; }
  const FString Action=Args[0];
  bool Handled=false;
  if (Action==TEXT("ctrl") || Action==TEXT("repeat") || Action==TEXT("release")
   || Action==TEXT("bare") || Action==TEXT("shift") || Action==TEXT("alt")
   || Action==TEXT("command") || Action==TEXT("ctrlshift"))
  {
   const bool Ctrl=Action==TEXT("ctrl") || Action==TEXT("repeat") || Action==TEXT("release") || Action==TEXT("ctrlshift");
   const bool Extra=Action==TEXT("shift") || Action==TEXT("alt") || Action==TEXT("command") || Action==TEXT("ctrlshift");
   const EInputEvent Event=Action==TEXT("repeat") ? IE_Repeat : Action==TEXT("release") ? IE_Released : IE_Pressed;
   Handled=Player->HandleGameAudioKey(EKeys::S,Event,Ctrl,Extra);
  }
  auto Device=World->GetAudioDevice();
  const bool Enabled=Player->IsGameAudioEnabled();
  if (Device)
  {
   FAudioThread::RunCommandOnAudioThread([Device,Action,Handled,Enabled]() mutable
   {
    if(Action==TEXT("seed")) { Device->SetTransientPrimaryVolume(.42f); }
    UE_LOG(LogSMAGF, Display, TEXT("SMAGF_AUDIO_TOGGLE_TEST action=%s enabled=%d handled=%d multiplier=%.6f"),
     *Action,Enabled,Handled,Device->GetTransientPrimaryVolume());
   });
  }
 }
};
static FAutoConsoleCommandWithWorldAndArgs GAudioToggleFixture(
 TEXT("SMAGF.TestAudioToggle"),TEXT("Development-only master audio input/lifecycle fixture."),
 FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FSMAGFAudioToggleFixture::Run));
#endif
