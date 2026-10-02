#include "Showcase/SMAGFShowcaseDirector.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Camera/SMAGFPlayerCameraManager.h"
#include "Targets/SMAGFTarget.h"
#include "SMAGFGameMode.h"
#include "GameFramework/PlayerController.h"
#include "Components/PointLightComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/App.h"
#include "SMAGF.h"

ASMAGFShowcaseDirector::ASMAGFShowcaseDirector()
{
 PrimaryActorTick.bCanEverTick=false;
 SetRootComponent(CreateDefaultSubobject<USceneComponent>(TEXT("Root")));
 SetActorEnableCollision(false);
 FocusLight=CreateDefaultSubobject<UPointLightComponent>(TEXT("FocusLight"));
 FocusLight->SetMobility(EComponentMobility::Movable);
 FocusLight->SetupAttachment(GetRootComponent());FocusLight->SetCanEverAffectNavigation(false);
 FocusLight->SetIntensity(0.f);FocusLight->SetAttenuationRadius(300.f);
 FocusLight->SetLightColor(FLinearColor(1.f,.94f,.84f));FocusLight->SetCastShadows(false);
 FocusLight->SetVisibility(false);
}
ASMAGFShowcaseDirector* ASMAGFShowcaseDirector::Get(UWorld* World)
{
 const auto* Mode=World?Cast<ASMAGFGameMode>(World->GetAuthGameMode()):nullptr;
 return Mode?Mode->GetShowcaseDirector():nullptr;
}
bool ASMAGFShowcaseDirector::Enabled() const
{ return Showcase.IsValid() && Showcase->IsShowcaseEnabled() && Showcase->IsGroupEnabled(ESMAGFShowcaseGroup::Focus); }
ASMAGFTarget* ASMAGFShowcaseDirector::GetFocusSubject() const { return FocusedTarget.Get(); }
void ASMAGFShowcaseDirector::BeginPlay()
{
 Super::BeginPlay();
 if(!GetWorld()->IsGameWorld()) { Destroy();return; }
 for(TActorIterator<ASMAGFShowcaseDirector> It(GetWorld());It;++It)
 { if(*It!=this && !It->IsActorBeingDestroyed()) { Destroy();return; } }
 // One bootstrap scan covers placed targets whose BeginPlay preceded GameMode.
 // Later targets register/unregister through their lifecycle; no per-frame world search.
 for(TActorIterator<ASMAGFTarget> It(GetWorld());It;++It) { RegisterTarget(*It); }
 Showcase=GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
 if(Showcase.IsValid()) { SettingsHandle=Showcase->OnSettingsChangedNative.AddUObject(this,&ThisClass::SettingsChanged); }
 SettingsChanged();
}
void ASMAGFShowcaseDirector::RegisterTarget(ASMAGFTarget* Target)
{
 if(IsValid(Target) && Target->GetWorld()==GetWorld() && Target->IsAliveForFocus())
 { Targets.AddUnique(TWeakObjectPtr<ASMAGFTarget>(Target)); }
}
void ASMAGFShowcaseDirector::UnregisterTarget(ASMAGFTarget* Target)
{
 if(FocusedTarget.Get()==Target) { Release(TEXT("target_invalid"),true);UpdateLight(); }
 Targets.RemoveAll([Target](const TWeakObjectPtr<ASMAGFTarget>& Entry){return !Entry.IsValid() || Entry.Get()==Target;});
}
void ASMAGFShowcaseDirector::Select(ASMAGFTarget* Target,float Distance)
{
 const bool bSwitch=FocusedTarget.IsValid();
 UE_LOG(LogSMAGF,Verbose,TEXT("%s target=%s current_distance=%g candidate_distance=%g reason=cursor"),
     bSwitch?TEXT("FOCUS_MOUSE_SWITCH"):TEXT("FOCUS_MOUSE_ACQUIRE"),*GetNameSafe(Target),CurrentDistance,Distance);
 FocusedTarget=Target;FocusPoint=Target->GetFocusAnchor();CurrentDistance=Distance;
 bReleasePending=false;ReleaseRemaining=0.f;bInvalidRelease=false;Age=0.f;++SelectionChanges;
}
void ASMAGFShowcaseDirector::Release(const TCHAR* Reason,bool bInvalid)
{
 if(FocusedTarget.IsValid())
 {
  UE_LOG(LogSMAGF,Verbose,TEXT("%s target=%s distance=%g reason=%s"),
      bInvalid?TEXT("FOCUS_MOUSE_INVALIDATE"):TEXT("FOCUS_MOUSE_RELEASE"),*GetNameSafe(FocusedTarget.Get()),CurrentDistance,Reason);
  ++SelectionChanges;
 }
 FocusedTarget.Reset();bReleasePending=false;ReleaseRemaining=0.f;CurrentDistance=-1.f;bInvalidRelease=bInvalid;
}
void ASMAGFShowcaseDirector::SettingsChanged()
{
 if(!Enabled()) { Release(TEXT("settings_disabled"));Strength=0.f; }
 UpdateLight();
}
bool ASMAGFShowcaseDirector::SetValidationMouseInput(FVector2D Screen,FIntPoint ViewportSize,bool bAvailable)
{
#if WITH_EDITOR
 if(FApp::IsUnattended() && GetWorld() && GetWorld()->IsGameWorld())
 { ValidationScreen=Screen;ValidationSize=ViewportSize;bValidationAvailable=bAvailable;bValidationInput=true;return true; }
#endif
 return false;
}
void ASMAGFShowcaseDirector::ClearValidationMouseInput() { bValidationInput=false; }
void ASMAGFShowcaseDirector::UpdateMouseFocus(ASMAGFPlayerCameraManager* Camera,APlayerController* Controller,float Dt)
{
 if(!Enabled()) { SettingsChanged();return; }
 if(GetWorld()->IsPaused()) { return; }
 Dt=FMath::Max(0.f,Dt);
 if(FocusedTarget.IsValid() && !FocusedTarget->IsAliveForFocus()) { Release(TEXT("target_dead"),true); }
 Targets.RemoveAll([](const TWeakObjectPtr<ASMAGFTarget>& Entry){return !Entry.IsValid() || !Entry->IsAliveForFocus();});
 FVector2D Cursor=FVector2D::ZeroVector;
 FIntPoint Size=FIntPoint::ZeroValue;
 bool bCursorValid=false;
 if(Controller)
 {
  float X=0.f,Y=0.f;
  bCursorValid=Controller->GetMousePosition(X,Y);Cursor=FVector2D(X,Y);
  Controller->GetViewportSize(Size.X,Size.Y);
 }
 if(bValidationInput) { Cursor=ValidationScreen;Size=ValidationSize;bCursorValid=bValidationAvailable; }
 bCursorValid=bCursorValid && Camera && Size.X>0 && Size.Y>0 && !Cursor.ContainsNaN()
     && Cursor.X>=0 && Cursor.Y>=0 && Cursor.X<Size.X && Cursor.Y<Size.Y;
 const auto S=Showcase->GetSettings().Focus;
 ASMAGFTarget* Best=nullptr;
 float BestDistance=TNumericLimits<float>::Max();
 CurrentDistance=TNumericLimits<float>::Max();
 if(bCursorValid)
 {
  const float Scale=float(FMath::Min(Size.X,Size.Y));
  for(const auto& Entry:Targets)
  {
   ASMAGFTarget* Target=Entry.Get();
   FVector2D Screen;
   const bool bProjected=bValidationInput
       ?Camera->ProjectWorldToLogicalViewport(Target->GetFocusAnchor(),Size,Screen)
       :Camera->ProjectWorldToLogicalScreen(Target->GetFocusAnchor(),Screen);
   if(!bProjected) { continue; }
   const float Distance=float(FVector2D::Distance(Cursor,Screen)/Scale);
   if(Target==FocusedTarget.Get()) { CurrentDistance=Distance; }
   if(Distance<BestDistance) { Best=Target;BestDistance=Distance; }
  }
 }
 if(!FocusedTarget.IsValid())
 {
  if(Best && BestDistance<=S.FocusAcquireRadius) { Select(Best,BestDistance); }
 }
 else if(Best && Best!=FocusedTarget.Get() && BestDistance<=S.FocusAcquireRadius
     && (CurrentDistance>S.FocusReleaseRadius || BestDistance+S.FocusSwitchAdvantage<CurrentDistance))
 { Select(Best,BestDistance); }
 else if(CurrentDistance<=S.FocusReleaseRadius)
 {
  if(bReleasePending)
  { UE_LOG(LogSMAGF,Verbose,TEXT("FOCUS_MOUSE_RETAIN target=%s distance=%g reason=grace_cancelled"),*GetNameSafe(FocusedTarget.Get()),CurrentDistance); }
  bReleasePending=false;ReleaseRemaining=0.f;
 }
 else
 {
  if(!bReleasePending)
  {
   bReleasePending=true;ReleaseRemaining=S.FocusDuration;
   UE_LOG(LogSMAGF,Verbose,TEXT("FOCUS_MOUSE_RELEASE_PENDING target=%s distance=%g candidate_distance=%g reason=%s"),
       *GetNameSafe(FocusedTarget.Get()),CurrentDistance,BestDistance,bCursorValid?TEXT("outside_release_or_unprojectable"):TEXT("cursor_unavailable"));
  }
  ReleaseRemaining=FMath::Max(0.f,ReleaseRemaining-Dt);
  if(ReleaseRemaining<=0.f) { Release(TEXT("grace_expired")); }
 }
 if(FocusedTarget.IsValid()) { FocusPoint=FocusedTarget->GetFocusAnchor();Age+=Dt; }
 Strength=FMath::FInterpTo(Strength,FocusedTarget.IsValid()?1.f:0.f,Dt,S.FocusCameraInterpSpeed);
 if(!FocusedTarget.IsValid() && Strength<.001f) { Strength=0.f; }
 UpdateLight();
}
void ASMAGFShowcaseDirector::UpdateLight()
{
 if(!Enabled() || bInvalidRelease) { FocusLight->SetIntensity(0.f);FocusLight->SetVisibility(false);return; }
 const auto Feature=[this](ESMAGFShowcaseFeature F){return Showcase->IsFeatureEffectivelyEnabled(F);};
 const float Steady=Feature(ESMAGFShowcaseFeature::TargetEmphasis)?.75f:0.f;
 const float Pulse=Feature(ESMAGFShowcaseFeature::ContextualLightingPulse)?.25f*(.75f+.25f*FMath::Sin(Age*2.f*PI*2.f)):0.f;
 const float Gain=FMath::Clamp(Strength*Showcase->GetSettings().Focus.FocusIntensity*Showcase->GetMasterIntensity(),0.f,2.f);
 FocusLight->SetWorldLocation(FocusPoint+FVector(0,0,120));
 const float Intensity=FMath::Clamp(1800.f*Gain*(Steady+Pulse),0.f,3600.f);
 FocusLight->SetIntensity(Intensity);FocusLight->SetVisibility(Intensity>.01f);
}
void ASMAGFShowcaseDirector::EndPlay(const EEndPlayReason::Type Reason)
{
 Release(TEXT("world_end"));Targets.Reset();Strength=0.f;FocusLight->SetIntensity(0.f);FocusLight->SetVisibility(false);
 if(Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(SettingsHandle); }
 Super::EndPlay(Reason);
}
