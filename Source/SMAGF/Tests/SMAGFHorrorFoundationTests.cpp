#include "Horror/SMAGFFlickeringComponent.h"
#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITOR
#include "Misc/AutomationTest.h"
#include "SMAGFGameMode.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "UObject/UnrealType.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFHorrorFoundationTest, "SMAGF.Horror.Foundation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FSMAGFHorrorFoundationTest::RunTest(const FString& Parameters)
{
	int32 Checks = 0;
	auto Check = [this, &Checks](const TCHAR* Name, bool Value)
	{
		++Checks; TestTrue(Name, Value);
		AddInfo(FString::Printf(TEXT("Foundation check %d: %s — %s"), Checks, Name, Value ? TEXT("PASS") : TEXT("FAIL")));
	};
	Check(TEXT("Native HorrorMode false"), !GetDefault<ASMAGFGameMode>()->IsHorrorMode());
	const FProperty* Property = FindFProperty<FProperty>(ASMAGFGameMode::StaticClass(), TEXT("bHorrorMode"));
	Check(TEXT("Defaults editable and Blueprint read-only"), Property && Property->HasAllPropertyFlags(CPF_Edit | CPF_BlueprintVisible | CPF_BlueprintReadOnly | CPF_DisableEditOnInstance));
	UBlueprint* Blueprint = FKismetEditorUtilities::CreateBlueprint(ASMAGFGameMode::StaticClass(), GetTransientPackage(),
		MakeUniqueObjectName(GetTransientPackage(), UBlueprint::StaticClass(), TEXT("SMAGFHorrorValidation")),
		BPTYPE_Normal, UBlueprint::StaticClass(), UBlueprintGeneratedClass::StaticClass());
	if (!TestNotNull(TEXT("Transient Blueprint created"), Blueprint)) { return false; }
	++Checks;
	CastChecked<ASMAGFGameMode>(Blueprint->GeneratedClass->GetDefaultObject())->bHorrorMode = true;
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	Check(TEXT("Compiled Blueprint defaults override getter"), CastChecked<ASMAGFGameMode>(Blueprint->GeneratedClass->GetDefaultObject())->IsHorrorMode());
	Check(TEXT("Native default unchanged"), !GetDefault<ASMAGFGameMode>()->IsHorrorMode());

	const auto Initialization = UWorld::InitializationValues().AllowAudioPlayback(false).CreatePhysicsScene(false)
		.RequiresHitProxies(false).CreateNavigation(false).CreateAISystem(false).ShouldSimulatePhysics(false);
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, false, NAME_None, nullptr, true, ERHIFeatureLevel::Num, &Initialization);
	GEngine->CreateNewWorldContext(EWorldType::Game).SetCurrentWorld(World);
	AActor* Owner = World->SpawnActor<AActor>();
	UPointLightComponent* Point = NewObject<UPointLightComponent>(Owner);
	Owner->AddInstanceComponent(Point); Point->SetIntensity(120.f); Point->RegisterComponent();
	USpotLightComponent* Spot = NewObject<USpotLightComponent>(Owner);
	Owner->AddInstanceComponent(Spot); Spot->SetIntensity(300.f); Spot->RegisterComponent();
	USMAGFFlickeringComponent* Flicker = NewObject<USMAGFFlickeringComponent>(Owner);
	Owner->AddInstanceComponent(Flicker); Flicker->RegisterComponent(); Flicker->RegisterAllComponentTickFunctions(true);
	// This isolated fixture does not initialize its actor through the gameplay startup path.
	Flicker->Activate(true); Flicker->BeginPlay();
	Check(TEXT("Fixture component active before requests"), Flicker->IsActive());
	auto Advance = [World](float Seconds)
	{
		for (int32 I = 0, Steps = FMath::CeilToInt(Seconds / .01f); I < Steps; ++I)
		{
			++GFrameCounter;
			World->Tick(LEVELTICK_TimeOnly, .01f);
			World->GetTimerManager().Tick(.01f);
		}
	};
	Check(TEXT("Owner discovers point and spot"), Flicker->Lights.Num() == 2);
	Check(TEXT("Disabled automatic default"), !Flicker->bFlickeringEnabled && !World->GetTimerManager().IsTimerActive(Flicker->SequenceTimer));
	Check(TEXT("No component Tick"), !Flicker->PrimaryComponentTick.bCanEverTick);
	Flicker->Random.Initialize(731);
	Flicker->FlickerBurstDuration = .2f; Flicker->FlickerBurstDurationJitter = 0.f;
	Flicker->TriggerFlickerBurst();
	Check(TEXT("Manual works without automatic"), Flicker->bBurstActive && !Flicker->bFlickeringEnabled && Point->Intensity != 120.f);
	Check(TEXT("One multiplier across lights"), FMath::IsNearlyEqual(Point->Intensity / 120.f, Spot->Intensity / 300.f));
	const float FirstIntensity = Point->Intensity;
	Advance(.14f);
	Check(TEXT("Jagged step changes intensity"), Point->Intensity != FirstIntensity);
	Flicker->TriggerFlickerBurst();
	Advance(.1f);
	Check(TEXT("Repeated trigger restarts burst"), Flicker->bBurstActive);
	Flicker->RefreshControlledLights();
	Advance(.25f);
	Check(TEXT("Burst end restores exact baseline after refresh"), Point->Intensity == 120.f && Spot->Intensity == 300.f);
	Check(TEXT("Manual did not start loop"), !World->GetTimerManager().IsTimerActive(Flicker->SequenceTimer));
	Flicker->TriggerFlickerBurst(); Flicker->StopFlickering();
	Check(TEXT("Stop restores and clears timer"), Point->Intensity == 120.f && Spot->Intensity == 300.f && !World->GetTimerManager().IsTimerActive(Flicker->SequenceTimer));
	Flicker->FlickerRestDuration = .2f; Flicker->FlickerRestDurationJitter = .02f;
	Flicker->FlickerBurstDurationJitter = .02f;
	Flicker->SetFlickeringEnabled(true);
	const float InitialDelay = World->GetTimerManager().GetTimerRemaining(Flicker->SequenceTimer);
	Check(TEXT("Automatic begins with randomized phase"), !Flicker->bBurstActive && InitialDelay >= .01f && InitialDelay <= .22f);
	Advance(InitialDelay + .02f);
	Check(TEXT("Automatic rest enters burst"), Flicker->bBurstActive);
	Advance(static_cast<float>(Flicker->BurstEndTime - World->GetTimeSeconds()) + .02f);
	Check(TEXT("Automatic burst returns to rest and baseline"), !Flicker->bBurstActive && Point->Intensity == 120.f && World->GetTimerManager().IsTimerActive(Flicker->SequenceTimer));
	Advance(World->GetTimerManager().GetTimerRemaining(Flicker->SequenceTimer) + .02f);
	Check(TEXT("Automatic loops again"), Flicker->bBurstActive);
	Flicker->SetFlickeringEnabled(false);
	Check(TEXT("Disable restores"), Point->Intensity == 120.f && !Flicker->bBurstActive && !World->GetTimerManager().IsTimerActive(Flicker->SequenceTimer));
	float MinBurst = 100.f, MaxBurst = 0.f, MinRest = 100.f, MaxRest = 0.f;
	for (int32 I = 0; I < 64; ++I)
	{
		const float Burst = Flicker->RandomDuration(3.f, 1.f), Rest = Flicker->RandomDuration(12.f, 2.f);
		MinBurst = FMath::Min(MinBurst, Burst); MaxBurst = FMath::Max(MaxBurst, Burst);
		MinRest = FMath::Min(MinRest, Rest); MaxRest = FMath::Max(MaxRest, Rest);
	}
	Check(TEXT("Burst jitter within 2..4 and varies"), MinBurst >= 2.f && MaxBurst <= 4.f && MaxBurst - MinBurst > 1.f);
	Check(TEXT("Rest jitter within 10..14 and varies"), MinRest >= 10.f && MaxRest <= 14.f && MaxRest - MinRest > 2.f);
	AddInfo(FString::Printf(TEXT("Jitter samples (64): burst %.6f..%.6f; rest %.6f..%.6f"), MinBurst, MaxBurst, MinRest, MaxRest));
	Check(TEXT("Negative timing positive"), Flicker->RandomDuration(-5.f, 0.f) > 0.f);
	Check(TEXT("Nonfinite timing safe"), FMath::IsFinite(Flicker->RandomDuration(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::quiet_NaN())));
	Flicker->FlickerMinIntensityMultiplier = -.5f; Flicker->FlickerMaxIntensityMultiplier = -1.f;
	Flicker->FlickerStepMin = -1.f; Flicker->FlickerStepMax = -2.f;
	Flicker->TriggerFlickerBurst();
	Check(TEXT("Invalid ranges clamp safely"), Point->Intensity == 0.f && World->GetTimerManager().GetTimerRemaining(Flicker->SequenceTimer) > 0.f);
	Flicker->Deactivate();
	Check(TEXT("Deactivate restores baseline"), Point->Intensity == 120.f && !World->GetTimerManager().IsTimerActive(Flicker->SequenceTimer));
	Flicker->Activate(); Flicker->TriggerFlickerBurst();
	Flicker->EndPlay(EEndPlayReason::Destroyed);
	Check(TEXT("EndPlay restores and clears"), Point->Intensity == 120.f && Spot->Intensity == 300.f && !World->GetTimerManager().IsTimerActive(Flicker->SequenceTimer));
	Flicker->TriggerFlickerBurst();
	Check(TEXT("No trigger after EndPlay"), !Flicker->bBurstActive);
	AActor* EmptyOwner = World->SpawnActor<AActor>();
	USMAGFFlickeringComponent* Empty = NewObject<USMAGFFlickeringComponent>(EmptyOwner);
	EmptyOwner->AddInstanceComponent(Empty); Empty->RegisterComponent(); Empty->RegisterAllComponentTickFunctions(true);
	Empty->Activate(true); Empty->BeginPlay();
	Check(TEXT("No-light fixture active"), Empty->IsActive());
	Empty->SetFlickeringEnabled(true); Empty->TriggerFlickerBurst();
	Check(TEXT("No-light owner safe and idle"), Empty->Lights.IsEmpty() && !World->GetTimerManager().IsTimerActive(Empty->SequenceTimer));
	Empty->EndPlay(EEndPlayReason::Destroyed);
	World->DestroyWorld(false);
	GEngine->DestroyWorldContext(World);
	AddInfo(FString::Printf(TEXT("Foundation assertions: %d"), Checks));
	return true;
}
#endif
