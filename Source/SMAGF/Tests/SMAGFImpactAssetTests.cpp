#if WITH_EDITOR && WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "HAL/IConsoleManager.h"
#include "Misc/PackageName.h"
#include "NiagaraSystem.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterHandle.h"
#include "NiagaraSpriteRendererProperties.h"
#include "NiagaraScriptSource.h"
#include "NiagaraGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"

namespace SMAGFImpactAsset
{
static const TCHAR* Path = TEXT("/Game/SMAGF/VFX/NS_ProjectileImpact_Circle.NS_ProjectileImpact_Circle");
static UEdGraphPin* FacingInput(FVersionedNiagaraEmitterData* Data)
{
	auto* Source = Data ? Cast<UNiagaraScriptSource>(Data->GraphSource) : nullptr;
	if (!Source || !Source->NodeGraph) { return nullptr; }
	UEdGraphPin* Found = nullptr;
	for (UEdGraphNode* Node : Source->NodeGraph->Nodes)
	{
		for (auto* Pin : Node->Pins)
		{
			if (Pin->Direction == EGPD_Input && Pin->PinName.ToString().EndsWith(TEXT(".Particles.SpriteFacing")))
			{
				if (Found) { return nullptr; }
				Found = Pin;
			}
		}
	}
	return Found;
}

// Explicit editor repair command; never runs during gameplay or ordinary tests.
// Only the approved project-owned duplicate is writable through this command.
static void Configure()
{
	UNiagaraSystem* System = LoadObject<UNiagaraSystem>(nullptr, Path);
	if (!System || System->GetEmitterHandles().Num() != 1)
	{
		UE_LOG(LogTemp, Error, TEXT("SMAGF impact asset missing or unexpected emitter count"));
		return;
	}
	const FVersionedNiagaraEmitter Instance = System->GetEmitterHandles()[0].GetInstance();
	auto* Data = Instance.Emitter ? Instance.Emitter->GetEmitterData(Instance.Version) : nullptr;
	auto* Sprite = Data && Data->GetRenderers().Num() == 1
		? Cast<UNiagaraSpriteRendererProperties>(Data->GetRenderer(0)) : nullptr;
	if (!Sprite || Sprite->SpriteFacingBinding.GetParamMapBindableVariable().GetName() != FName(TEXT("Particles.SpriteFacing")))
	{
		UE_LOG(LogTemp, Error, TEXT("SMAGF impact asset has unexpected renderer/facing binding"));
		return;
	}
	auto* Facing = FacingInput(Data);
	if (!Facing) { UE_LOG(LogTemp, Error, TEXT("SMAGF expected facing assignment override missing")); return; }
	System->Modify(); Instance.Emitter->Modify(); Sprite->Modify();
	Facing->GetOwningNode()->Modify();
	// The old RotateVector override reads the previous facing with a zero fallback.
	// Write the actual UV sprite-plane normal every update, independent of that feedback.
	Facing->BreakAllPinLinks();
	Facing->DefaultValue = TEXT("1.0,0.0,0.0");
	Facing->bDefaultValueIsIgnored = false;
	// Niagara's override marks the node unsynchronized and notifies its graph.
	// Use the public virtual node API; no direct unexported graph calls are needed.
	Facing->GetOwningNode()->PinDefaultValueChanged(Facing);
	Data->bLocalSpace = true;
	Sprite->FacingMode = ENiagaraSpriteFacingMode::CustomFacingVector;
	Sprite->PostEditChange();
	Instance.Emitter->PostEditChange();
	System->RequestCompile(true);
	System->WaitForCompilationComplete(true, false);
	System->MarkPackageDirty();
	const FString Filename = FPackageName::LongPackageNameToFilename(System->GetOutermost()->GetName(), FPackageName::GetAssetPackageExtension());
	FSavePackageArgs Args;
	Args.TopLevelFlags = RF_Public | RF_Standalone;
	Args.SaveFlags = SAVE_NoError;
	const bool Saved = UPackage::SavePackage(System->GetOutermost(), System, *Filename, Args);
	UE_LOG(LogTemp, Display, TEXT("SMAGF impact orientation asset saved=%d; local normal +X"), Saved);
}
static FAutoConsoleCommand ConfigureCommand(TEXT("SMAGF.ConfigureImpactAsset"),
	TEXT("Apply the approved local-space/custom-facing orientation to the SMAGF impact duplicate only."),
	FConsoleCommandDelegate::CreateStatic(&Configure));
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFImpactAssetTest, "SMAGF.Presentation.ImpactAssetOrientation",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSMAGFImpactAssetTest::RunTest(const FString& Parameters)
{
	auto* System = LoadObject<UNiagaraSystem>(nullptr, SMAGFImpactAsset::Path);
	if (!TestNotNull(TEXT("Project-owned Niagara asset resolves"), System)) { return false; }
	if (!TestEqual(TEXT("One preserved emitter"), System->GetEmitterHandles().Num(), 1)) { return false; }
	const auto Instance = System->GetEmitterHandles()[0].GetInstance();
	auto* Data = Instance.Emitter->GetEmitterData(Instance.Version);
	if (!TestNotNull(TEXT("Active version data"), Data)) { return false; }
	TestTrue(TEXT("Component owns simulation frame"), Data->bLocalSpace);
	if (!TestEqual(TEXT("One preserved renderer"), Data->GetRenderers().Num(), 1)) { return false; }
	const auto* Sprite = Cast<UNiagaraSpriteRendererProperties>(Data->GetRenderer(0));
	if (!TestNotNull(TEXT("Sprite renderer"), Sprite)) { return false; }
	TestTrue(TEXT("No camera-facing plane"), Sprite->FacingMode == ENiagaraSpriteFacingMode::CustomFacingVector);
	TestEqual(TEXT("Existing facing attribute preserved"), Sprite->SpriteFacingBinding.GetParamMapBindableVariable().GetName(), FName(TEXT("Particles.SpriteFacing")));
	auto* Facing = SMAGFImpactAsset::FacingInput(Data);
	if (!TestNotNull(TEXT("Explicit geometric sprite-plane input"), Facing)) { return false; }
	TestEqual(TEXT("No previous-facing feedback connection"), Facing->LinkedTo.Num(), 0);
	TestEqual(TEXT("Constant local UV-plane normal"), Facing->DefaultValue, FString(TEXT("1.0,0.0,0.0")));
	return true;
}
#endif
