#include "Feedback/SMAGFPresentationPolicy.h"
#if WITH_DEV_AUTOMATION_TESTS
#include "Misc/AutomationTest.h"
#include "Feedback/SMAGFFeedbackSubsystem.h"
#include "UObject/EnumProperty.h"
#include "UObject/UnrealType.h"
#include "UObject/CoreRedirects.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/StructuredArchiveAdapters.h"
#include <limits>

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FSMAGFJuicePolicyTest, "SMAGF.Juice.PolicyAndMigration",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FSMAGFJuicePolicyTest::RunTest(const FString& Parameters)
{
	using namespace SMAGFPresentationPolicy;
	using I = ESMAGFEventImportance;
	FSMAGFShowcaseSettings Settings;
	const auto Defaults = Settings.JuiceResponsibly;
	TestEqual(TEXT("First isolated response"), Resolve(0.f, Defaults, I::Minor).Scale, 1.f);
	float Pressure = 0.f, Previous = 1.f;
	for (int32 Index = 0; Index < 10; ++Index)
	{
		const float Scale = Resolve(Pressure, Defaults, I::Minor).Scale;
		TestTrue(TEXT("Minor burst monotonic and bounded"), Scale <= Previous && Scale >= Defaults.JuiceMinimumScale);
		Pressure = FMath::Clamp(Pressure + Cost(I::Minor), 0.f, 1.f);
		Previous = Scale;
		TestTrue(TEXT("Standard outranks Minor"), Resolve(Pressure, Defaults, I::Standard).Scale > Resolve(Pressure, Defaults, I::Minor).Scale);
		TestEqual(TEXT("Major full"), Resolve(Pressure, Defaults, I::Major).Scale, 1.f);
	}
	TestEqual(TEXT("Pressure cap"), Pressure, 1.f);
	TestEqual(TEXT("Quiet recovery"), Decay(Pressure, Defaults.JuicePressureDecay, .8f), 0.f);
	TestEqual(TEXT("Pause delta neutral"), Decay(Pressure, Defaults.JuicePressureDecay, 0.f), Pressure);
	I Normalized = I::Minor;
	TestTrue(TEXT("Legacy numeric 3 accepted"), Normalize(static_cast<I>(3), Normalized));
	TestEqual(TEXT("Legacy Critical maps Major"), Normalized, I::Major);
	TestEqual(TEXT("Legacy policy Major full"), Resolve(1.f, Defaults, static_cast<I>(3)).Scale, 1.f);
	TestFalse(TEXT("Invalid enum rejected"), Normalize(static_cast<I>(254), Normalized));
	TestEqual(TEXT("Invalid enum has no cost"), Cost(I::Count), 0.f);
	const auto* Redirects = FCoreRedirects::GetValueRedirects(ECoreRedirectFlags::Type_Enum, FCoreRedirectObjectName(TEXT("/Script/SMAGF.ESMAGFEventImportance")));
	TestNotNull(TEXT("Enum redirects registered"), Redirects);
	if (Redirects)
	{
		TestEqual(TEXT("Normal name migration"), Redirects->FindRef(TEXT("Normal")), FString(TEXT("Standard")));
		TestEqual(TEXT("Critical name migration"), Redirects->FindRef(TEXT("Critical")), FString(TEXT("Major")));
	}
	TestEqual(TEXT("Standard reflected value"), StaticEnum<I>()->GetValueByNameString(TEXT("Standard")), int64(1));
	// Exercise the real enum-property archive path, not only redirect registration.
	const UFunction* Getter = USMAGFFeedbackSubsystem::StaticClass()->FindFunctionByName(TEXT("GetLastImportance"));
	const FEnumProperty* EnumProperty = Getter ? CastField<FEnumProperty>(Getter->GetReturnProperty()) : nullptr;
	TestNotNull(TEXT("Reflected enum return property"), EnumProperty);
	if (EnumProperty)
	{
		for (const auto& Entry : TArray<TPair<FName, I>>{
			{FName(TEXT("ESMAGFEventImportance::Normal")), I::Standard},
			{FName(TEXT("ESMAGFEventImportance::Critical")), I::Major}})
		{
			TArray<uint8> LegacyBytes;
			FMemoryWriter LegacyWriter(LegacyBytes);
			FObjectAndNameAsStringProxyArchive LegacySave(LegacyWriter, false);
			FName Name = Entry.Key;
			// Use the archive interface so the inherited FName overload dispatches virtually.
			static_cast<FArchive&>(LegacySave) << Name;
			FMemoryReader LegacyReader(LegacyBytes);
			FObjectAndNameAsStringProxyArchive LegacyLoad(LegacyReader, false);
			FStructuredArchiveFromArchive LegacyArchive(LegacyLoad);
			I Value = I::Count;
			EnumProperty->SerializeItem(LegacyArchive.GetSlot(), &Value, nullptr);
			TestEqual(TEXT("Old enum name deserializes through redirect"), Value, Entry.Value);
		}
		TArray<uint8> EnumBytes;
		I Standard = I::Standard, ReloadedStandard = I::Count;
		FMemoryWriter EnumWriter(EnumBytes);
		FObjectAndNameAsStringProxyArchive EnumSave(EnumWriter, false);
		FStructuredArchiveFromArchive SaveArchive(EnumSave);
		EnumProperty->SerializeItem(SaveArchive.GetSlot(), &Standard, nullptr);
		FMemoryReader EnumReader(EnumBytes);
		FObjectAndNameAsStringProxyArchive EnumLoad(EnumReader, false);
		FStructuredArchiveFromArchive LoadArchive(EnumLoad);
		EnumProperty->SerializeItem(LoadArchive.GetSlot(), &ReloadedStandard, nullptr);
		TestEqual(TEXT("New Standard enum archive round-trip"), ReloadedStandard, I::Standard);
	}
	TestEqual(TEXT("Stable FullShowcase value"), static_cast<uint8>(ESMAGFShowcasePreset::FullShowcase), uint8(6));
	TestEqual(TEXT("Stable Custom value"), static_cast<uint8>(ESMAGFShowcasePreset::Custom), uint8(7));
	TestEqual(TEXT("Juice display position"), SMAGFOrderedPresets().Find(ESMAGFShowcasePreset::JuiceResponsibly), 6);
	TestEqual(TEXT("Full display position"), SMAGFOrderedPresets().Find(ESMAGFShowcasePreset::FullShowcase), 7);
	Settings.JuiceResponsibly.JuiceBudgetStrength = 10.f;
	Settings.JuiceResponsibly.JuicePressureDecay = -1.f;
	Settings.JuiceResponsibly.JuiceMinimumScale = std::numeric_limits<float>::quiet_NaN();
	Settings.Validate();
	TestEqual(TEXT("Strength clamped"), Settings.JuiceResponsibly.JuiceBudgetStrength, 1.f);
	TestEqual(TEXT("Decay clamped"), Settings.JuiceResponsibly.JuicePressureDecay, .05f);
	TestEqual(TEXT("Nonfinite floor default"), Settings.JuiceResponsibly.JuiceMinimumScale, .35f);
	FSMAGFJuiceResponsiblySettings Imported;
	auto* Struct = FSMAGFJuiceResponsiblySettings::StaticStruct();
	const TCHAR* Tail = Struct->ImportText(TEXT("(MinorMultiplier=3,NormalMultiplier=2,MajorMultiplier=1,CriticalMultiplier=0)"),
		&Imported, nullptr, PPF_UseDeprecatedProperties, GWarn, Struct->GetName());
	TestNotNull(TEXT("Legacy property text loads"), Tail);
	const TCHAR* Names[] = {TEXT("MinorMultiplier"), TEXT("NormalMultiplier"), TEXT("MajorMultiplier"), TEXT("CriticalMultiplier")};
	for (int32 Index = 0; Index < 4; ++Index)
	{
		auto* Property = FindFProperty<FFloatProperty>(Struct, Names[Index]);
		TestNotNull(TEXT("Legacy serialized property name retained"), Property);
		if (Property)
		{
			TestTrue(TEXT("Legacy property deprecated"), Property->HasAnyPropertyFlags(CPF_Deprecated));
			TestFalse(TEXT("Legacy hidden from editing and Blueprint"), Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible));
			TestEqual(TEXT("Legacy value consumed"), Property->GetPropertyValue_InContainer(&Imported), float(3-Index));
		}
	}
	TestTrue(TEXT("Legacy excluded from equality"), Imported == Defaults);
	TestEqual(TEXT("New strength default after old data"), Imported.JuiceBudgetStrength, 1.f);
	TestEqual(TEXT("New decay default after old data"), Imported.JuicePressureDecay, 1.25f);
	TestEqual(TEXT("Legacy cannot affect policy"), Resolve(1.f, Imported, I::Standard).Scale, Resolve(1.f, Defaults, I::Standard).Scale);
	// Tagged archive round-trip: deprecated properties must not be emitted in new data.
	Imported.JuiceBudgetStrength = .7f;
	TArray<uint8> Bytes;
	FMemoryWriter Writer(Bytes);
	FObjectAndNameAsStringProxyArchive Save(Writer, false);
	Struct->SerializeItem(Save, &Imported, nullptr);
	FSMAGFJuiceResponsiblySettings Reloaded;
	FMemoryReader Reader(Bytes);
	FObjectAndNameAsStringProxyArchive Load(Reader, false);
	Struct->SerializeItem(Load, &Reloaded, nullptr);
	TestTrue(TEXT("New tagged settings round-trip"), Reloaded == Imported);
	for (int32 Index = 0; Index < 4; ++Index)
	{
		if (auto* Property = FindFProperty<FFloatProperty>(Struct, Names[Index]))
		{ TestEqual(TEXT("Deprecated not saved"), Property->GetPropertyValue_InContainer(&Reloaded), Property->GetPropertyValue_InContainer(&Defaults)); }
	}
	return true;
}
#endif
