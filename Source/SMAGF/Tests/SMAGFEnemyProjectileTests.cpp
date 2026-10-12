#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS && !UE_BUILD_SHIPPING
#include "Enemy/SMAGFEnemyProjectile.h"
#include "Enemy/SMAGFEnemyShowcaseComponent.h"
#include "Targets/SMAGFTarget.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Misc/AutomationTest.h"
#include "UObject/UnrealType.h"
#include "SMAGF.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFEnemyProjectileAuthoringTest,"SMAGF.Enemy.ProjectilePresentationAuthoring",
 EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSMAGFEnemyProjectileAuthoringTest::RunTest(const FString&)
{
 int32 Count=0;
 auto Check=[&](const TCHAR* Name,bool Pass) { ++Count;TestTrue(Name,Pass); };
 for(const FName Name : {FName(TEXT("EnemyProjectileEmissiveColor")),FName(TEXT("EnemyProjectileEmissiveStrength"))})
 {
  const FProperty* Property=FindFProperty<FProperty>(USMAGFEnemyShowcaseComponent::StaticClass(),Name);
  Check(TEXT("Property reflected"),Property!=nullptr);
  if(!Property) { continue; }
  Check(TEXT("Defaults editable"),Property->HasAllPropertyFlags(CPF_Edit|CPF_DisableEditOnInstance));
  Check(TEXT("Blueprint read-only"),Property->HasAllPropertyFlags(CPF_BlueprintVisible|CPF_BlueprintReadOnly));
  Check(TEXT("No Config"),!Property->HasAnyPropertyFlags(CPF_Config));
  Check(TEXT("Presentation category"),Property->GetMetaData(TEXT("Category"))==TEXT("SMAGF|Enemy|Presentation"));
 }
 const auto* Component=GetDefault<ASMAGFTarget>()->FindComponentByClass<USMAGFEnemyShowcaseComponent>();
 Check(TEXT("Native component exists"),Component!=nullptr);
 if(Component)
 {
  Check(TEXT("Native cyan"),Component->EnemyProjectileEmissiveColor==FLinearColor(0.f,.7f,1.f,1.f));
  Check(TEXT("Native strength four"),Component->EnemyProjectileEmissiveStrength==4.f);
  Check(TEXT("Inherited component editable"),Component->IsEditableWhenInherited());
 }
 AddInfo(FString::Printf(TEXT("SMAGF_PROJECTILE_AUTHORING assertions=%d"),Count));
 return true;
}

// Tests both lifecycle orders in a live PIE world, without saving any asset.
static void TestEnemyProjectilePresentation(UWorld* World)
{
 if(!World || !World->IsGameWorld()) { return; }
 int32 Count=0,Failures=0;
 auto Check=[&](const TCHAR* Name,bool Pass)
 {
  ++Count;if(!Pass) { ++Failures; }
  UE_LOG(LogSMAGF,Display,TEXT("SMAGF_PROJECTILE_PRESENTATION_CHECK name=%s pass=%d"),Name,Pass);
 };
 FActorSpawnParameters Params;Params.Owner=World->GetFirstPlayerController();
 Params.SpawnCollisionHandlingOverride=ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
 auto* Shot=World->SpawnActor<ASMAGFEnemyProjectile>(FVector(0,0,20000),FRotator::ZeroRotator,Params);
 Check(TEXT("OrdinarySpawn"),IsValid(Shot));
 if(!IsValid(Shot)) { return; }
 Check(TEXT("BeginPlayBeforeReturn"),Shot->HasActorBegunPlay());
 auto* Visual=Shot->FindComponentByClass<UStaticMeshComponent>();
 auto* MID=Visual?Cast<UMaterialInstanceDynamic>(Visual->GetMaterial(0)):nullptr;
 auto* Collision=Shot->FindComponentByClass<USphereComponent>();
 auto* Movement=Shot->FindComponentByClass<UProjectileMovementComponent>();
 Check(TEXT("ExistingMID"),MID!=nullptr);
 if(!MID || !Collision || !Movement) { Shot->Destroy();return; }
 const FLinearColor Cyan(0.f,.7f,1.f,1.f);
 Check(TEXT("UnconfiguredCyan"),MID->K2_GetVectorParameterValue(TEXT("Emissive Color"))==Cyan);
 Check(TEXT("UnconfiguredFour"),MID->K2_GetScalarParameterValue(TEXT("Emissive Strength"))==4.f);
 Shot->InitializeFlight(FVector(3,4,2),123.f);
 const FVector Velocity=Movement->Velocity;
 const FTransform Transform=Shot->GetActorTransform();
 const FTransform VisualTransform=Visual->GetRelativeTransform();
 const float Life=Shot->GetLifeSpan();
 const FCollisionResponseContainer Responses=Collision->GetCollisionResponseToChannels();
 auto Apply=[&](const FLinearColor& Color,float Strength,const FLinearColor& Expected,float ExpectedStrength)
 {
  Shot->SetPresentation(Color,Strength);
  Check(TEXT("SameMID"),Visual->GetMaterial(0)==MID);
  Check(TEXT("EffectiveColor"),MID->K2_GetVectorParameterValue(TEXT("Emissive Color"))==Expected);
  Check(TEXT("EffectiveStrength"),MID->K2_GetScalarParameterValue(TEXT("Emissive Strength"))==ExpectedStrength);
  Check(TEXT("FlightUnchanged"),Movement->Velocity==Velocity && Movement->MaxSpeed==123.f && Movement->InitialSpeed==123.f);
  Check(TEXT("TransformsUnchanged"),Shot->GetActorTransform().Equals(Transform) && Visual->GetRelativeTransform().Equals(VisualTransform));
  Check(TEXT("CollisionUnchanged"),Collision->GetCollisionEnabled()==ECollisionEnabled::QueryOnly && Collision->GetCollisionResponseToChannels()==Responses && Collision->GetUnscaledSphereRadius()==6.f);
  Check(TEXT("OwnerAndLifetimeUnchanged"),Shot->GetOwner()==Params.Owner && Shot->GetLifeSpan()==Life);
 };
 const FLinearColor TestColor(.2f,.8f,.1f,1.f);
 Apply(TestColor,7.f,TestColor,7.f);
 Apply(TestColor,-1.f,TestColor,0.f);
 Apply(TestColor,101.f,TestColor,100.f);
 Apply(TestColor,std::numeric_limits<float>::quiet_NaN(),TestColor,4.f);
 Apply(TestColor,std::numeric_limits<float>::infinity(),TestColor,4.f);
 const FLinearColor HDR(-.5f,2.f,.25f,.5f);
 Apply(HDR,0.f,HDR,0.f);
 Apply(FLinearColor(std::numeric_limits<float>::quiet_NaN(),.8f,.1f,1.f),7.f,Cyan,7.f);
 Apply(FLinearColor(.2f,.8f,.1f,std::numeric_limits<float>::infinity()),7.f,Cyan,7.f);

 Params.bDeferConstruction=true;
 auto* Early=World->SpawnActor<ASMAGFEnemyProjectile>(FVector(100,0,20000),FRotator::ZeroRotator,Params);
 Check(TEXT("DeferredFixtureCreated"),IsValid(Early));
 if(IsValid(Early))
 {
  Check(TEXT("BeforeBeginPlay"),!Early->HasActorBegunPlay());
  Early->SetPresentation(TestColor,7.f);
  Early->FinishSpawning(FTransform(FVector(100,0,20000)));
  Check(TEXT("DeferredBeginPlayCompleted"),Early->HasActorBegunPlay());
  auto* EarlyVisual=Early->FindComponentByClass<UStaticMeshComponent>();
  auto* EarlyMID=EarlyVisual?Cast<UMaterialInstanceDynamic>(EarlyVisual->GetMaterial(0)):nullptr;
  Check(TEXT("StoredValuesAppliedInBeginPlay"),EarlyMID && EarlyMID->K2_GetVectorParameterValue(TEXT("Emissive Color"))==TestColor && EarlyMID->K2_GetScalarParameterValue(TEXT("Emissive Strength"))==7.f);
  Check(TEXT("PerProjectileMID"),EarlyMID && EarlyMID!=MID);
  Shot->SetPresentation(Cyan,4.f);
  Check(TEXT("OtherProjectileUnaffected"),EarlyMID && EarlyMID->K2_GetVectorParameterValue(TEXT("Emissive Color"))==TestColor && EarlyMID->K2_GetScalarParameterValue(TEXT("Emissive Strength"))==7.f);
  Early->Destroy();
 }
 Shot->Destroy();
 UE_LOG(LogSMAGF,Display,TEXT("SMAGF_PROJECTILE_PRESENTATION_SUMMARY checks=%d failures=%d"),Count,Failures);
}
static FAutoConsoleCommandWithWorld GEnemyProjectilePresentationTest(
 TEXT("SMAGF.TestEnemyProjectilePresentation"),TEXT("Editor-only presentation lifecycle and invariant tests."),
 FConsoleCommandWithWorldDelegate::CreateStatic(&TestEnemyProjectilePresentation));
#endif
