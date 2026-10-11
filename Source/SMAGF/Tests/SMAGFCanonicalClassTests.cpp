#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "SMAGFGameMode.h"
#include "Targets/SMAGFTarget.h"
#include "Engine/Blueprint.h"
#include "Engine/World.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "HAL/IConsoleManager.h"
#include "UObject/StrongObjectPtr.h"
#include "SMAGF.h"

// Unsaved /Engine/Transient fixture; never creates a Content asset or production fallback.
struct FSMAGFCanonicalClassFixture
{
 static FString Selection;
 static TStrongObjectPtr<UBlueprint> Blueprint;
 static FDelegateHandle InitializedHandle;
 static void BeforeBeginPlay(const FActorsInitializedParams& Params)
 {
  UWorld* World=Params.World;
  if (!World || !World->IsGameWorld()) { return; }
  auto* Mode=World->GetAuthGameMode<ASMAGFGameMode>();
  if (!Mode || Mode->HasActorBegunPlay()) { return; }
  if (Selection==TEXT("subclass"))
  {
   if (!Blueprint.IsValid() || !Blueprint->GeneratedClass) { return; }
   Mode->ShowcaseEnemyClass=Blueprint->GeneratedClass;
  }
  else if (Selection==TEXT("null")) { Mode->ShowcaseEnemyClass=nullptr; }
  UE_LOG(LogSMAGF,Display,TEXT("SMAGF_CLASS_FIXTURE before_begin=1 selection=%s class=%s"),
   *Selection,*GetNameSafe(Mode->ShowcaseEnemyClass.Get()));
 }
 static void Run(const TArray<FString>& Args,UWorld* World)
 {
  if (Args.Num()!=1) { return; }
  if (Args[0]==TEXT("subclass") || Args[0]==TEXT("null") || Args[0]==TEXT("native"))
  {
   Selection=Args[0];
   if (Selection==TEXT("subclass") && !Blueprint.IsValid())
   {
    Blueprint.Reset(FKismetEditorUtilities::CreateBlueprint(ASMAGFTarget::StaticClass(),
     GetTransientPackage(),MakeUniqueObjectName(GetTransientPackage(),UBlueprint::StaticClass(),TEXT("SMAGFCanonicalTest")),BPTYPE_Normal));
    if (!Blueprint.IsValid()) { return; }
    Blueprint->SetFlags(RF_Transient);
    FKismetEditorUtilities::CompileBlueprint(Blueprint.Get());
    if (!Blueprint->GeneratedClass) { return; }
   }
   if (!InitializedHandle.IsValid())
   { InitializedHandle=FWorldDelegates::OnWorldInitializedActors.AddStatic(&BeforeBeginPlay); }
  }
  else if (Args[0]==TEXT("status") && World)
  {
   if (auto* Mode=World->GetAuthGameMode<ASMAGFGameMode>())
   {
    for(int32 Index=0;Index<Mode->SpawnSlots.Num();++Index)
    {
     const auto& Slot=Mode->SpawnSlots[Index];
     UE_LOG(LogSMAGF,Display,TEXT("SMAGF_CLASS_SLOT index=%d actor=%s class=%s original=%s"),
      Index,*GetNameSafe(Slot.Enemy.Get()),*GetNameSafe(Slot.Enemy.IsValid()?Slot.Enemy->GetClass():nullptr),
      *Slot.OriginalTransform.ToString());
    }
   }
  }
 }
};
FString FSMAGFCanonicalClassFixture::Selection;
TStrongObjectPtr<UBlueprint> FSMAGFCanonicalClassFixture::Blueprint;
FDelegateHandle FSMAGFCanonicalClassFixture::InitializedHandle;
static FAutoConsoleCommandWithWorldAndArgs GCanonicalClassFixture(
 TEXT("SMAGF.TestCanonicalClass"),TEXT("Editor-only fixture: arm native/subclass/null before PIE; status inspects slots."),
 FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FSMAGFCanonicalClassFixture::Run));
#endif
