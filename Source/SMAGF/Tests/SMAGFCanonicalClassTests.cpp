#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "SMAGFGameMode.h"
#include "Enemy/SMAGFEnemyShowcaseComponent.h"
#include "SMAGFPlayerController.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "Targets/SMAGFTarget.h"
#include "Engine/Blueprint.h"
#include "Engine/World.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "HAL/IConsoleManager.h"
#include "UObject/StrongObjectPtr.h"
#include "SMAGF.h"
#include "Misc/PackageName.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

// Editor-only fixtures: transient subclasses or the externally saved persistence package.
// Never writes Content or installs a production fallback.
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
  if ((Selection==TEXT("subclass") || Selection==TEXT("marker-disabled") || Selection==TEXT("projectile-authored") || Selection==TEXT("charge-authored") || Selection==TEXT("charge-disabled")))
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
  if (Args[0]==TEXT("subclass") || Args[0]==TEXT("marker-disabled") || Args[0]==TEXT("projectile-authored") || Args[0]==TEXT("charge-authored") || Args[0]==TEXT("charge-disabled") || Args[0]==TEXT("null") || Args[0]==TEXT("native"))
  {
   Selection=Args[0];
   if(Selection==TEXT("charge-authored") || Selection==TEXT("charge-disabled"))
   {
    FString Directory;
    if(!FParse::Value(FCommandLine::Get(),TEXT("SMAGFPresentationFixtureDir="),Directory)) { return; }
    FPackageName::RegisterMountPoint(TEXT("/SMAGFPresentationTest/"),Directory);
    Blueprint.Reset(LoadObject<UBlueprint>(nullptr,TEXT("/SMAGFPresentationTest/BP_Attack.BP_Attack")));
    if(!Blueprint.IsValid() || !Blueprint->GeneratedClass) { return; }
    if(Selection==TEXT("charge-disabled")) { CastChecked<ASMAGFTarget>(Blueprint->GeneratedClass->GetDefaultObject())->bFrontMarkerEnabled=false; }
   }
   if ((Selection==TEXT("subclass") || Selection==TEXT("marker-disabled") || Selection==TEXT("projectile-authored") || Selection==TEXT("charge-authored") || Selection==TEXT("charge-disabled")) && !Blueprint.IsValid())
   {
    Blueprint.Reset(FKismetEditorUtilities::CreateBlueprint(ASMAGFTarget::StaticClass(),
     GetTransientPackage(),MakeUniqueObjectName(GetTransientPackage(),UBlueprint::StaticClass(),TEXT("SMAGFCanonicalTest")),BPTYPE_Normal));
    if (!Blueprint.IsValid()) { return; }
    Blueprint->SetFlags(RF_Transient);
    FKismetEditorUtilities::CompileBlueprint(Blueprint.Get());
    if (!Blueprint->GeneratedClass) { return; }
    // Author the unsaved fixture's actor Class Defaults.
    if (Selection==TEXT("marker-disabled") || Selection==TEXT("projectile-authored") || Selection==TEXT("charge-authored") || Selection==TEXT("charge-disabled"))
    {
     auto* Defaults=Cast<ASMAGFTarget>(Blueprint->GeneratedClass->GetDefaultObject());
     auto* Component=Defaults?Defaults->FindComponentByClass<USMAGFEnemyShowcaseComponent>():nullptr;
     if (!Component) { return; }
     if (Selection==TEXT("marker-disabled")) { Defaults->bFrontMarkerEnabled=false; }
     else
     {
      Defaults->EnemyProjectileEmissiveColor=FLinearColor(.2f,.8f,.1f,1.f);
      Defaults->EnemyProjectileEmissiveStrength=7.f;
     }
    }
   }
   // Rebuild Blueprint post-construction defaults after programmatic CDO authoring.
   if(Blueprint.IsValid() && (Selection==TEXT("marker-disabled") || Selection==TEXT("charge-disabled") || Selection==TEXT("projectile-authored")))
   {
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint.Get());
    FKismetEditorUtilities::CompileBlueprint(Blueprint.Get());
   }
   if (!InitializedHandle.IsValid())
   { InitializedHandle=FWorldDelegates::OnWorldInitializedActors.AddStatic(&BeforeBeginPlay); }
  }
  else if (Args[0]==TEXT("restore-key") && World)
  {
   if (auto* Controller=Cast<ASMAGFPlayerController>(World->GetFirstPlayerController()))
   { Controller->HandleShowcaseToggleKey(EKeys::T,true,false); }
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
 TEXT("SMAGF.TestCanonicalClass"),TEXT("Editor-only fixture: arm native/subclass/null/marker-disabled/projectile-authored before PIE; status inspects slots; restore-key exercises Ctrl+T handler."),
 FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&FSMAGFCanonicalClassFixture::Run));

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFFrontMarkerAuthoringTest,"SMAGF.Enemy.FrontMarkerAuthoring",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSMAGFFrontMarkerAuthoringTest::RunTest(const FString& Parameters)
{
 const auto* Property=FindFProperty<FBoolProperty>(ASMAGFTarget::StaticClass(),TEXT("bFrontMarkerEnabled"));
 TestNotNull(TEXT("Authored bool is reflected"),Property);
 if (!Property) { return false; }
 TestTrue(TEXT("Editable defaults"),Property->HasAnyPropertyFlags(CPF_Edit));
 TestTrue(TEXT("Not an instance authoring property"),Property->HasAnyPropertyFlags(CPF_DisableEditOnInstance));
 TestTrue(TEXT("Blueprint readable"),Property->HasAnyPropertyFlags(CPF_BlueprintVisible));
 TestTrue(TEXT("Blueprint read only"),Property->HasAnyPropertyFlags(CPF_BlueprintReadOnly));
 TestEqual(TEXT("Presentation category"),Property->GetMetaData(TEXT("Category")),FString(TEXT("SMAGF|Enemy|Presentation")));
 const auto* Component=GetDefault<ASMAGFTarget>()->FindComponentByClass<USMAGFEnemyShowcaseComponent>();
 TestNotNull(TEXT("Native target owns inherited component"),Component);
 if (!Component) { return false; }
 TestTrue(TEXT("Native gate remains true"),GetDefault<ASMAGFTarget>()->bFrontMarkerEnabled);
 TestTrue(TEXT("Inherited component is editable"),Component->IsEditableWhenInherited());
 AddInfo(TEXT("SMAGF_MARKER_AUTHORING assertions=9"));
 return true;
}
#endif
