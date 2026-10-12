#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Targets/SMAGFTarget.h"
#include "Enemy/SMAGFEnemyShowcaseComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "Editor.h"
#include "EngineUtils.h"
#include "Components/StaticMeshComponent.h"
#include "HAL/IConsoleManager.h"
#include "SMAGF.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/AutomationTest.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFAttackPresentationWrite,"SMAGF.Enemy.AttackPresentation.PersistenceWrite",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFAttackPresentationRead,"SMAGF.Enemy.AttackPresentation.PersistenceRead",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
static bool TestPersistence(FAutomationTestBase& Test,bool bWrite)
{
 int32 Count=0,Failures=0;
 auto Check=[&](const TCHAR* Label,bool Pass) { ++Count;if(!Pass) { ++Failures; } Test.TestTrue(Label,Pass); };
 FString Directory;
 Check(TEXT("External fixture directory supplied"),FParse::Value(FCommandLine::Get(),TEXT("SMAGFPresentationFixtureDir="),Directory));
 if(Directory.IsEmpty()) { return false; }
 FPackageName::RegisterMountPoint(TEXT("/SMAGFPresentationTest/"),Directory);
 IFileManager::Get().MakeDirectory(*Directory,true);
 UBlueprint* BP=nullptr;
 if(bWrite)
 {
  if(FPaths::FileExists(FPaths::Combine(Directory,TEXT("BP_Attack.uasset"))))
  { BP=LoadObject<UBlueprint>(nullptr,TEXT("/SMAGFPresentationTest/BP_Attack.BP_Attack")); }
  else
  {
   UPackage* Package=CreatePackage(TEXT("/SMAGFPresentationTest/BP_Attack"));
   BP=FKismetEditorUtilities::CreateBlueprint(ASMAGFTarget::StaticClass(),Package,TEXT("BP_Attack"),BPTYPE_Normal);
  }
  Check(TEXT("Created external test Blueprint"),BP!=nullptr);
  if(!BP) { return false; }
  FKismetEditorUtilities::CompileBlueprint(BP);
  auto* Defaults=Cast<ASMAGFTarget>(BP->GeneratedClass->GetDefaultObject());
  Defaults->Modify();
  Defaults->EnemyProjectileEmissiveColor=FLinearColor(.2f,.8f,.1f,1.f);
  Defaults->EnemyProjectileEmissiveStrength=7.f;
  Defaults->EnemyTelegraphPresentationMode=ESMAGFEnemyTelegraphPresentationMode::ChargeOnly;
  Defaults->EnemyTelegraphMesh=LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
  Defaults->EnemyTelegraphRelativeRotation=FRotator(90.f,0.f,0.f);
  Defaults->EnemyTelegraphTargetScale=FVector(.12f,.6f,.04f);
  Defaults->EnemyTelegraphLocalOffset=FVector(47.f,2.f,19.f);
  Defaults->EnemyTelegraphRetractDuration=.3f;
  FBlueprintEditorUtils::MarkBlueprintAsModified(BP);
 }
 else { BP=LoadObject<UBlueprint>(nullptr,TEXT("/SMAGFPresentationTest/BP_Attack.BP_Attack")); }
 Check(TEXT("Blueprint available"),BP && BP->GeneratedClass);
 if(!BP || !BP->GeneratedClass) { return false; }
 auto Verify=[&](const ASMAGFTarget* Target)
 {
  Check(TEXT("Target exists"),Target!=nullptr);if(!Target) { return; }
  Check(TEXT("Actor color persisted"),Target->EnemyProjectileEmissiveColor==FLinearColor(.2f,.8f,.1f,1.f));
  Check(TEXT("Actor strength persisted"),Target->EnemyProjectileEmissiveStrength==7.f);
  Check(TEXT("Actor mode persisted"),Target->EnemyTelegraphPresentationMode==ESMAGFEnemyTelegraphPresentationMode::ChargeOnly);
  Check(TEXT("Actor mesh persisted"),Target->EnemyTelegraphMesh && Target->EnemyTelegraphMesh->GetPathName()==TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
  Check(TEXT("Actor rotation persisted"),Target->EnemyTelegraphRelativeRotation==FRotator(90.f,0.f,0.f));
  Check(TEXT("Actor scale persisted"),Target->EnemyTelegraphTargetScale==FVector(.12f,.6f,.04f));
  Check(TEXT("Actor offset persisted"),Target->EnemyTelegraphLocalOffset==FVector(47.f,2.f,19.f));
  Check(TEXT("Actor retract persisted"),Target->EnemyTelegraphRetractDuration==.3f);
  Check(TEXT("Master gate enabled"),Target->bFrontMarkerEnabled);
 };
 Verify(Cast<ASMAGFTarget>(BP->GeneratedClass->GetDefaultObject()));
 // Force compilation with a live instance, exercising the reinstancing path.
 UWorld* World=GEditor->GetEditorWorldContext().World();
 FActorSpawnParameters Params;Params.ObjectFlags=RF_Transient;
 auto* Instance=World->SpawnActor<ASMAGFTarget>(BP->GeneratedClass,FVector(0,0,20000),FRotator::ZeroRotator,Params);
 Verify(Instance);
 const FName InstanceName=Instance?Instance->GetFName():NAME_None;
 FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(BP);
 FKismetEditorUtilities::CompileBlueprint(BP);
 Check(TEXT("Blueprint compile successful"),BP->Status!=BS_Error);
 Verify(Cast<ASMAGFTarget>(BP->GeneratedClass->GetDefaultObject()));
 auto* Reinstanced=FindObject<ASMAGFTarget>(World->GetCurrentLevel(),*InstanceName.ToString());
 Verify(Reinstanced);
 if(Reinstanced) { World->DestroyActor(Reinstanced); }
 auto* Fresh=World->SpawnActor<ASMAGFTarget>(BP->GeneratedClass,FVector(0,0,20000),FRotator::ZeroRotator,Params);
 Verify(Fresh);if(Fresh) { World->DestroyActor(Fresh); }
 for(const TCHAR* Name : {TEXT("bFrontMarkerEnabled"),TEXT("EnemyProjectileEmissiveColor"),TEXT("EnemyProjectileEmissiveStrength"),TEXT("EnemyTelegraphPresentationMode"),TEXT("EnemyTelegraphMesh"),TEXT("EnemyTelegraphRelativeRotation"),TEXT("EnemyTelegraphTargetScale"),TEXT("EnemyTelegraphLocalOffset"),TEXT("EnemyTelegraphRetractDuration")})
 {
  const auto* Property=FindFProperty<FProperty>(ASMAGFTarget::StaticClass(),Name);
  Check(TEXT("Actor defaults-only and Blueprint read-only metadata"),Property && Property->HasAllPropertyFlags(CPF_Edit|CPF_DisableEditOnInstance|CPF_BlueprintVisible|CPF_BlueprintReadOnly));
  Check(TEXT("No competing component authoring field"),FindFProperty<FProperty>(USMAGFEnemyShowcaseComponent::StaticClass(),Name)==nullptr);
 }
 const auto* Native=GetDefault<ASMAGFTarget>();
 Check(TEXT("Native Legacy unchanged"),Native->EnemyTelegraphPresentationMode==ESMAGFEnemyTelegraphPresentationMode::Legacy && Native->bFrontMarkerEnabled);
 Check(TEXT("Native projectile unchanged"),Native->EnemyProjectileEmissiveColor==FLinearColor(0.f,.7f,1.f,1.f) && Native->EnemyProjectileEmissiveStrength==4.f);
 if(bWrite)
 {
  FSavePackageArgs Args;Args.TopLevelFlags=RF_Public|RF_Standalone;Args.SaveFlags=SAVE_None;
  Check(TEXT("External fixture saved"),UPackage::SavePackage(BP->GetOutermost(),BP,*FPaths::Combine(Directory,TEXT("BP_Attack.uasset")),Args));
 }
 Test.AddInfo(FString::Printf(TEXT("SMAGF_ATTACK_PERSISTENCE write=%d assertions=%d failures=%d"),bWrite,Count,Failures));
 return Failures==0;
}
bool FSMAGFAttackPresentationWrite::RunTest(const FString&) { return TestPersistence(*this,true); }
bool FSMAGFAttackPresentationRead::RunTest(const FString&) { return TestPersistence(*this,false); }
// Observe every game tick: Slate/Python polling can skip the exact shot frame.
static FDelegateHandle CaptureHandle;
static void CaptureTelegraph(UWorld* World,ELevelTick,float Dt)
{
 if(!World || !World->IsGameWorld()) { return; }
 for(TActorIterator<ASMAGFTarget> It(World);It;++It)
 {
  auto* Behavior=It->FindComponentByClass<USMAGFEnemyShowcaseComponent>();
  if(!Behavior || Behavior->GetSpawnIndex()!=0) { continue; }
  TInlineComponentArray<UStaticMeshComponent*> Meshes(It.operator->());
  for(auto* Mesh:Meshes)
  {
   if(Mesh->GetFName()!=TEXT("EnemyFrontMarker")) { continue; }
   const FVector Scale=Mesh->GetRelativeScale3D();
   UE_LOG(LogSMAGF,Display,TEXT("SMAGF_CHARGE_SAMPLE time=%.9f dt=%.9f state=%d shots=%d alpha=%.9f visible=%d exact=%d"),
    World->GetTimeSeconds(),Dt,int32(Behavior->GetThreatState()),Behavior->GetShotsFired(),
    Scale.X/It->EnemyTelegraphTargetScale.X,Mesh->IsVisible(),Scale.Equals(It->EnemyTelegraphTargetScale,1.e-6));
  }
 }
}
static void ToggleCapture(const TArray<FString>& Args)
{
 if(CaptureHandle.IsValid()) { FWorldDelegates::OnWorldPostActorTick.Remove(CaptureHandle);CaptureHandle.Reset(); }
 if(Args.Num()==1 && Args[0]==TEXT("start"))
 { CaptureHandle=FWorldDelegates::OnWorldPostActorTick.AddStatic(&CaptureTelegraph); }
}
static FAutoConsoleCommand GCaptureTelegraph(TEXT("SMAGF.TestCaptureTelegraph"),TEXT("Editor-only game-tick presentation capture: start/stop."),
 FConsoleCommandWithArgsDelegate::CreateStatic(&ToggleCapture));
#endif
