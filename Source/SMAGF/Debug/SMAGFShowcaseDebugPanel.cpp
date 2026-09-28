#include "Debug/SMAGFShowcaseDebugPanel.h"
#include "Debug/SMAGFGameViewportClient.h"
#include "Debug/SMAGFDebugUISettings.h"
#include "Brushes/SlateColorBrush.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "SMAGFPlayerController.h"
#include "Engine/World.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

namespace
{
const FSlateBrush* PanelBackgroundBrush()
{
	// Unit-alpha, texture-free fill. The border tint is the sole authored alpha.
	static const FSlateColorBrush Brush(FLinearColor::White);
	return &Brush;
}

FText PresetName(ESMAGFShowcasePreset Preset)
{
	switch (Preset)
	{
	case ESMAGFShowcasePreset::SmokeAndMirrors: return FText::FromString(TEXT("Smoke & Mirrors"));
	case ESMAGFShowcasePreset::FullShowcase: return FText::FromString(TEXT("Full Showcase"));
	default: return StaticEnum<ESMAGFShowcasePreset>()->GetDisplayNameTextByValue(static_cast<int64>(Preset));
	}
}
}

void SSMAGFShowcaseDebugPanel::Construct(const FArguments& Args)
{
	Controller = Args._Controller;
	Subsystem = Args._Subsystem;
	Refresh();
	if (Subsystem.IsValid())
	{
		SettingsHandle = Subsystem->OnSettingsChangedNative.AddSP(this, &SSMAGFShowcaseDebugPanel::Refresh);
		PresetHandle = Subsystem->OnPresetChangedNative.AddSP(this, &SSMAGFShowcaseDebugPanel::Refresh);
	}
	TSharedRef<SVerticalBox> Rows = SNew(SVerticalBox);
	Rows->AddSlot().AutoHeight().Padding(0, 8)[SNew(STextBlock).Text(FText::FromString(TEXT("SMAGF — GAME FEEL LAB")))];
	Rows->AddSlot().AutoHeight().Padding(0, 4)[SNew(STextBlock).Text(this, &SSMAGFShowcaseDebugPanel::PresetText)];
	Rows->AddSlot().AutoHeight().Padding(0, 4)[SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(TEXT("Configuration only — effects arrive in later phases. Ctrl+F closes; PageUp/PageDown change layers.")))];
	TSharedRef<SWrapBox> Presets = SNew(SWrapBox).UseAllottedSize(true);
	for (uint8 Index = 0; Index <= static_cast<uint8>(ESMAGFShowcasePreset::FullShowcase); ++Index)
	{
		const ESMAGFShowcasePreset Preset = static_cast<ESMAGFShowcasePreset>(Index);
		Presets->AddSlot().Padding(2)[SNew(SButton).Text(PresetName(Preset))
			.OnClicked_Lambda([this, Preset]() { if (Subsystem.IsValid()) { Subsystem->SetShowcasePreset(Preset); } return FReply::Handled(); })];
	}
	Presets->AddSlot().Padding(2)[SNew(SButton).Text(FText::FromString(TEXT("Restore Custom")))
		.IsEnabled_Lambda([this]() { return bHasCustom; })
		.OnClicked_Lambda([this]() { if (Subsystem.IsValid()) { Subsystem->SetShowcasePreset(ESMAGFShowcasePreset::Custom); } return FReply::Handled(); })];
	Rows->AddSlot().AutoHeight()[Presets];
	TSharedRef<SWrapBox> Actions = SNew(SWrapBox).UseAllottedSize(true);
	Actions->AddSlot().Padding(2)[SNew(SButton).Text(FText::FromString(TEXT("Previous Layer")))
		.OnClicked_Lambda([this]() { if (Subsystem.IsValid()) { Subsystem->PreviousShowcasePreset(); } return FReply::Handled(); })];
	Actions->AddSlot().Padding(2)[SNew(SButton).Text(FText::FromString(TEXT("Next Layer")))
		.OnClicked_Lambda([this]() { if (Subsystem.IsValid()) { Subsystem->NextShowcasePreset(); } return FReply::Handled(); })];
	Actions->AddSlot().Padding(2)[SNew(SButton).Text(FText::FromString(TEXT("Restart Current Map")))
		.OnClicked_Lambda([this]() { if (Controller.IsValid() && Controller->GetWorld()) { if (USMAGFGameViewportClient* Viewport = Cast<USMAGFGameViewportClient>(Controller->GetWorld()->GetGameViewport())) { Viewport->RestartCurrentMap(); } } return FReply::Handled(); })];
	Actions->AddSlot().Padding(2)[SNew(SButton).Text(FText::FromString(TEXT("Close")))
		.OnClicked_Lambda([this]() { if (Controller.IsValid()) { Controller->CloseShowcasePanel(); } return FReply::Handled(); })];
	Rows->AddSlot().AutoHeight()[Actions];
	BuildControls(Rows);
	Rows->AddSlot().AutoHeight().Padding(0, 18)[SNew(SButton).Text(FText::FromString(TEXT("Reset Runtime Settings to Authored Defaults")))
		.OnClicked_Lambda([this]() { if (Subsystem.IsValid()) { Subsystem->ResetToDefaults(); } return FReply::Handled(); })];
	ChildSlot.HAlign(HAlign_Left).VAlign(VAlign_Fill).Padding(16)
	[
		SNew(SBox).WidthOverride(560)
		[
			SNew(SBorder).Padding(14).BorderImage(PanelBackgroundBrush())
			.BorderBackgroundColor_Lambda([]()
			{
				return FLinearColor(0.075f, 0.075f, 0.075f, GetDefault<USMAGFDebugUISettings>()->GetBackgroundOpacity());
			})
			// Bubble-phase fallback within the visual border; children handle their own input first.
			.OnMouseButtonDown_Lambda([](const FGeometry&, const FPointerEvent&) { return FReply::Handled(); })
			.OnMouseButtonUp_Lambda([](const FGeometry&, const FPointerEvent&) { return FReply::Handled(); })
			.OnMouseDoubleClick_Lambda([](const FGeometry&, const FPointerEvent&) { return FReply::Handled(); })
			[SNew(SScrollBox) + SScrollBox::Slot()[Rows]]
		]
	];
}

SSMAGFShowcaseDebugPanel::~SSMAGFShowcaseDebugPanel() { Disconnect(); }
void SSMAGFShowcaseDebugPanel::Disconnect()
{
	if (Subsystem.IsValid())
	{
		Subsystem->OnSettingsChangedNative.Remove(SettingsHandle);
		Subsystem->OnPresetChangedNative.Remove(PresetHandle);
	}
	Subsystem.Reset(); Controller.Reset();
}
void SSMAGFShowcaseDebugPanel::Refresh()
{
	TGuardValue<bool> Guard(bRefreshing, true);
	if (Subsystem.IsValid())
	{
		DisplaySettings = Subsystem->GetSettings();
		DisplayPreset = Subsystem->GetShowcasePreset();
		DisplayLastNamed = Subsystem->GetLastNamedShowcasePreset();
		bHasCustom = Subsystem->HasCustomShowcaseSnapshot();
	}
}
void SSMAGFShowcaseDebugPanel::Edit(TFunction<void(FSMAGFShowcaseSettings&)> Mutation)
{
	if (!bRefreshing && Subsystem.IsValid())
	{
		FSMAGFShowcaseSettings Copy = Subsystem->GetSettings();
		Mutation(Copy); Subsystem->ApplySettings(Copy);
	}
}
FText SSMAGFShowcaseDebugPanel::PresetText() const
{
	return DisplayPreset == ESMAGFShowcasePreset::Custom
		? FText::Format(FText::FromString(TEXT("Current Preset: Custom  |  Based on: {0}")), PresetName(DisplayLastNamed))
		: FText::Format(FText::FromString(TEXT("Current Preset: {0}")), PresetName(DisplayPreset));
}
FReply SSMAGFShowcaseDebugPanel::OnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event)
{
	const FKey Key = Event.GetKey();
	if (Controller.IsValid() && Controller->HandleShowcaseToggleKey(Key, Event.IsControlDown(), Event.IsRepeat()))
	{
		return FReply::Handled();
	}
	if (Key == EKeys::PageUp || Key == EKeys::PageDown || Key == EKeys::Home || Key == EKeys::End)
	{
		if (!Event.IsRepeat() && Controller.IsValid()) { Controller->HandleShowcaseKey(Key); }
		return FReply::Handled();
	}
	return FReply::Unhandled();
}
void SSMAGFShowcaseDebugPanel::AddBool(const TSharedRef<SVerticalBox>& Rows, const TCHAR* Label,
	TFunction<bool(const FSMAGFShowcaseSettings&)> Read, TFunction<void(FSMAGFShowcaseSettings&, bool)> Write)
{
	Rows->AddSlot().AutoHeight().Padding(0, 3)[SNew(SCheckBox)
		.IsChecked_Lambda([this, Read]() { return Read(DisplaySettings) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked; })
		.OnCheckStateChanged_Lambda([this, Write](ECheckBoxState State) { Edit([Write, State](FSMAGFShowcaseSettings& Copy) { Write(Copy, State == ECheckBoxState::Checked); }); })
		[SNew(STextBlock).Text(FText::FromString(Label))]];
}
template<typename T> void SSMAGFShowcaseDebugPanel::AddNumber(const TSharedRef<SVerticalBox>& Rows, const TCHAR* Label, T Min, T Max,
	TFunction<T(const FSMAGFShowcaseSettings&)> Read, TFunction<void(FSMAGFShowcaseSettings&, T)> Write)
{
	Rows->AddSlot().AutoHeight().Padding(0, 3)[SNew(SHorizontalBox)
		+ SHorizontalBox::Slot().FillWidth(1).VAlign(VAlign_Center)[SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(Label))]
		+ SHorizontalBox::Slot().AutoWidth()[SNew(SBox).WidthOverride(150)[SNew(SSpinBox<T>)
			.MinValue(Min).MaxValue(Max).MinSliderValue(Min).MaxSliderValue(Max)
			.Value_Lambda([this, Read]() { return Read(DisplaySettings); })
			.OnValueChanged_Lambda([this, Write](T Value) { Edit([Write, Value](FSMAGFShowcaseSettings& Copy) { Write(Copy, Value); }); })]]];
}
void SSMAGFShowcaseDebugPanel::AddGroup(const TSharedRef<SVerticalBox>& Rows, const TCHAR* Label, ESMAGFShowcaseGroup Group)
{
	Rows->AddSlot().AutoHeight().Padding(0, 18, 0, 6)[SNew(STextBlock).Text(FText::FromString(Label))];
	Rows->AddSlot().AutoHeight().Padding(0, 3)[SNew(SButton).Text(FText::FromString(TEXT("Reset Group")))
		.OnClicked_Lambda([this, Group]() { if (Subsystem.IsValid()) { Subsystem->ResetCurrentGroupToDefaults(Group); } return FReply::Handled(); })];
}

void SSMAGFShowcaseDebugPanel::BuildControls(const TSharedRef<SVerticalBox>& Rows)
{
	AddBool(Rows, TEXT("Enable Showcase Features"), [](const FSMAGFShowcaseSettings& S) { return S.Master.bShowcaseEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Master.bShowcaseEnabled = V; });
	AddNumber<float>(Rows, TEXT("Master Juice Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Master.MasterIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Master.MasterIntensity = V; });
	AddGroup(Rows, TEXT("Feeling Is Everything"), ESMAGFShowcaseGroup::Feeling);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bEnabled = V; });
	AddBool(Rows, TEXT("Fire Anticipation Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bFireAnticipationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bFireAnticipationEnabled = V; });
	AddBool(Rows, TEXT("Movement Acceleration Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bMovementAccelerationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bMovementAccelerationEnabled = V; });
	AddBool(Rows, TEXT("Recoil Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bRecoilEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bRecoilEnabled = V; });
	AddBool(Rows, TEXT("Impact Response Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bImpactResponseEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bImpactResponseEnabled = V; });
	AddBool(Rows, TEXT("Hit Stop Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bHitStopEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bHitStopEnabled = V; });
	AddBool(Rows, TEXT("Enemy Knockback Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bEnemyKnockbackEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bEnemyKnockbackEnabled = V; });
	AddBool(Rows, TEXT("Player Knockback Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bPlayerKnockbackEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bPlayerKnockbackEnabled = V; });
	AddBool(Rows, TEXT("Hit Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bHitReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bHitReactionEnabled = V; });
	AddNumber<float>(Rows, TEXT("Fire Anticipation Time"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.FireAnticipationTime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.FireAnticipationTime = V; });
	AddNumber<float>(Rows, TEXT("Anticipation Scale"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.AnticipationScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.AnticipationScale = V; });
	AddNumber<float>(Rows, TEXT("Movement Acceleration"), 0.0f, 10000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.MovementAcceleration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.MovementAcceleration = V; });
	AddNumber<float>(Rows, TEXT("Movement Deceleration"), 0.0f, 10000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.MovementDeceleration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.MovementDeceleration = V; });
	AddNumber<float>(Rows, TEXT("Recoil Distance"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilDistance = V; });
	AddNumber<float>(Rows, TEXT("Recoil Rotation"), 0.0f, 45.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilRotation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilRotation = V; });
	AddNumber<float>(Rows, TEXT("Recoil Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilDuration = V; });
	AddNumber<float>(Rows, TEXT("Recoil Recovery Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilRecoveryDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilRecoveryDuration = V; });
	AddNumber<float>(Rows, TEXT("Hit Stop Duration"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitStopDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitStopDuration = V; });
	AddNumber<float>(Rows, TEXT("Kill Hit Stop Duration"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.KillHitStopDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.KillHitStopDuration = V; });
	AddNumber<float>(Rows, TEXT("Hit Stop Time Scale"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitStopTimeScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitStopTimeScale = V; });
	AddNumber<float>(Rows, TEXT("Enemy Knockback Strength"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.EnemyKnockbackStrength; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.EnemyKnockbackStrength = V; });
	AddNumber<float>(Rows, TEXT("Enemy Knockback Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.EnemyKnockbackDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.EnemyKnockbackDuration = V; });
	AddNumber<float>(Rows, TEXT("Player Knockback Strength"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.PlayerKnockbackStrength; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.PlayerKnockbackStrength = V; });
	AddNumber<float>(Rows, TEXT("Player Knockback Recovery"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.PlayerKnockbackRecovery; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.PlayerKnockbackRecovery = V; });
	AddNumber<float>(Rows, TEXT("Hit Reaction Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitReactionDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitReactionDuration = V; });
	AddNumber<float>(Rows, TEXT("Hit Reaction Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitReactionIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitReactionIntensity = V; });
	AddNumber<float>(Rows, TEXT("Impact Response Distance"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.ImpactResponseDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.ImpactResponseDistance = V; });
	AddNumber<float>(Rows, TEXT("Impact Response Duration"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.ImpactResponseDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.ImpactResponseDuration = V; });
	AddGroup(Rows, TEXT("Move It or Lose It"), ESMAGFShowcaseGroup::Animation);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bEnabled = V; });
	AddBool(Rows, TEXT("Fire Anticipation Animation Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bFireAnticipationAnimationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bFireAnticipationAnimationEnabled = V; });
	AddBool(Rows, TEXT("Weapon Recoil Animation Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bWeaponRecoilAnimationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bWeaponRecoilAnimationEnabled = V; });
	AddBool(Rows, TEXT("Character Fire Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bCharacterFireReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bCharacterFireReactionEnabled = V; });
	AddBool(Rows, TEXT("Enemy Hit Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bEnemyHitReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bEnemyHitReactionEnabled = V; });
	AddBool(Rows, TEXT("Enemy Death Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bEnemyDeathReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bEnemyDeathReactionEnabled = V; });
	AddBool(Rows, TEXT("Squash And Stretch Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bSquashAndStretchEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bSquashAndStretchEnabled = V; });
	AddBool(Rows, TEXT("Overshoot Recovery Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bOvershootRecoveryEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bOvershootRecoveryEnabled = V; });
	AddBool(Rows, TEXT("Procedural Wobble Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bProceduralWobbleEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bProceduralWobbleEnabled = V; });
	AddNumber<float>(Rows, TEXT("Animation Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.AnimationIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.AnimationIntensity = V; });
	AddNumber<float>(Rows, TEXT("Squash Amount"), 0.0f, 0.9f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.SquashAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.SquashAmount = V; });
	AddNumber<float>(Rows, TEXT("Stretch Amount"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.StretchAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.StretchAmount = V; });
	AddNumber<float>(Rows, TEXT("Overshoot Amount"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.OvershootAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.OvershootAmount = V; });
	AddNumber<float>(Rows, TEXT("Wobble Amount"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.WobbleAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.WobbleAmount = V; });
	AddNumber<float>(Rows, TEXT("Wobble Frequency"), 0.0f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.WobbleFrequency; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.WobbleFrequency = V; });
	AddNumber<float>(Rows, TEXT("Reaction Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.ReactionDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.ReactionDuration = V; });
	AddNumber<float>(Rows, TEXT("Recovery Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.RecoveryDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.RecoveryDuration = V; });
Rows->AddSlot().AutoHeight().Padding(0, 6)[SNew(SComboButton)
		.ButtonContent()[SNew(STextBlock).Text_Lambda([this]() { return FText::Format(FText::FromString(TEXT("Animation Easing: {0}")), StaticEnum<ESMAGFEasingType>()->GetDisplayNameTextByValue(static_cast<int64>(DisplaySettings.Animation.Easing))); })]
		.OnGetMenuContent_Lambda([this]() -> TSharedRef<SWidget>
		{
			TSharedRef<SVerticalBox> Options = SNew(SVerticalBox);
			for (uint8 Index = 0; Index < static_cast<uint8>(ESMAGFEasingType::Count); ++Index)
			{
				const ESMAGFEasingType Value = static_cast<ESMAGFEasingType>(Index);
				Options->AddSlot().AutoHeight()[SNew(SButton).Text(StaticEnum<ESMAGFEasingType>()->GetDisplayNameTextByValue(Index))
					.OnClicked_Lambda([this, Value]() { Edit([Value](FSMAGFShowcaseSettings& S) { S.Animation.Easing = Value; }); return FReply::Handled(); })];
			}
			return Options;
		})];
	AddGroup(Rows, TEXT("Blood, Sweat & Pixelated Streaming"), ESMAGFShowcaseGroup::Audiovisual);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bEnabled = V; });
	AddBool(Rows, TEXT("Muzzle Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bMuzzleFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bMuzzleFlashEnabled = V; });
	AddBool(Rows, TEXT("Shot Light Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bShotLightFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bShotLightFlashEnabled = V; });
	AddBool(Rows, TEXT("Projectile Visual Scale Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bProjectileVisualScaleEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bProjectileVisualScaleEnabled = V; });
	AddBool(Rows, TEXT("Projectile Trail Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bProjectileTrailEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bProjectileTrailEnabled = V; });
	AddBool(Rows, TEXT("Shell Ejection Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bShellEjectionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bShellEjectionEnabled = V; });
	AddBool(Rows, TEXT("Fire Sound Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bFireSoundEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bFireSoundEnabled = V; });
	AddBool(Rows, TEXT("Layered Fire Sound Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bLayeredFireSoundEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bLayeredFireSoundEnabled = V; });
	AddBool(Rows, TEXT("Impact Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bImpactFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bImpactFlashEnabled = V; });
	AddBool(Rows, TEXT("Impact Effect Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bImpactEffectEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bImpactEffectEnabled = V; });
	AddBool(Rows, TEXT("Impact Sound Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bImpactSoundEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bImpactSoundEnabled = V; });
	AddBool(Rows, TEXT("Enemy Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bEnemyFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bEnemyFlashEnabled = V; });
	AddBool(Rows, TEXT("Death Effect Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bDeathEffectEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bDeathEffectEnabled = V; });
	AddBool(Rows, TEXT("Explosion Effect Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bExplosionEffectEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bExplosionEffectEnabled = V; });
	AddNumber<float>(Rows, TEXT("Muzzle Flash Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.MuzzleFlashScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.MuzzleFlashScale = V; });
	AddNumber<float>(Rows, TEXT("Muzzle Flash Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.MuzzleFlashDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.MuzzleFlashDuration = V; });
	AddNumber<float>(Rows, TEXT("Shot Light Intensity"), 0.0f, 100000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ShotLightIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ShotLightIntensity = V; });
	AddNumber<float>(Rows, TEXT("Shot Light Radius"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ShotLightRadius; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ShotLightRadius = V; });
	AddNumber<float>(Rows, TEXT("Projectile Visual Scale"), 0.01f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ProjectileVisualScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ProjectileVisualScale = V; });
	AddNumber<float>(Rows, TEXT("Trail Duration"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.TrailDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.TrailDuration = V; });
	AddNumber<float>(Rows, TEXT("Shell Lifetime"), 0.0f, 120.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ShellLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ShellLifetime = V; });
	AddNumber<float>(Rows, TEXT("Fire Sound Volume"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.FireSoundVolume; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.FireSoundVolume = V; });
	AddNumber<float>(Rows, TEXT("Fire Sound Pitch Variation"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.FireSoundPitchVariation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.FireSoundPitchVariation = V; });
	AddNumber<float>(Rows, TEXT("Impact Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ImpactScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ImpactScale = V; });
	AddNumber<float>(Rows, TEXT("Impact Duration"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ImpactDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ImpactDuration = V; });
	AddNumber<float>(Rows, TEXT("Impact Light Intensity"), 0.0f, 100000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ImpactLightIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ImpactLightIntensity = V; });
	AddNumber<float>(Rows, TEXT("Enemy Flash Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.EnemyFlashDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.EnemyFlashDuration = V; });
	AddNumber<float>(Rows, TEXT("Death Effect Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.DeathEffectScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.DeathEffectScale = V; });
	AddNumber<float>(Rows, TEXT("Explosion Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ExplosionScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ExplosionScale = V; });
	AddGroup(Rows, TEXT("Camera"), ESMAGFShowcaseGroup::Camera);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bEnabled = V; });
	AddBool(Rows, TEXT("Camera Follow Smoothing Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bCameraFollowSmoothingEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bCameraFollowSmoothingEnabled = V; });
	AddBool(Rows, TEXT("Aim Look Ahead Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bAimLookAheadEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bAimLookAheadEnabled = V; });
	AddBool(Rows, TEXT("Screen Shake Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bScreenShakeEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bScreenShakeEnabled = V; });
	AddBool(Rows, TEXT("Directional Camera Kick Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bDirectionalCameraKickEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bDirectionalCameraKickEnabled = V; });
	AddBool(Rows, TEXT("Kill Camera Emphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bKillCameraEmphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bKillCameraEmphasisEnabled = V; });
	AddNumber<float>(Rows, TEXT("Camera Follow Speed"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraFollowSpeed; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraFollowSpeed = V; });
	AddNumber<float>(Rows, TEXT("Aim Look Ahead Distance"), 0.0f, 1000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.AimLookAheadDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.AimLookAheadDistance = V; });
	AddNumber<float>(Rows, TEXT("Aim Look Ahead Interpolation"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.AimLookAheadInterpolation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.AimLookAheadInterpolation = V; });
	AddNumber<float>(Rows, TEXT("Shake Translation Amplitude"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeTranslationAmplitude; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeTranslationAmplitude = V; });
	AddNumber<float>(Rows, TEXT("Shake Rotation Amplitude"), 0.0f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeRotationAmplitude; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeRotationAmplitude = V; });
	AddNumber<float>(Rows, TEXT("Shake Frequency"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeFrequency; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeFrequency = V; });
	AddNumber<float>(Rows, TEXT("Shake Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeDuration = V; });
	AddNumber<float>(Rows, TEXT("Shake Falloff"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeFalloff; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeFalloff = V; });
	AddNumber<float>(Rows, TEXT("Camera Kick Distance"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickDistance = V; });
	AddNumber<float>(Rows, TEXT("Camera Kick Rotation"), 0.0f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickRotation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickRotation = V; });
	AddNumber<float>(Rows, TEXT("Camera Kick Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickDuration = V; });
	AddNumber<float>(Rows, TEXT("Camera Kick Recovery"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickRecovery; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickRecovery = V; });
	AddNumber<float>(Rows, TEXT("Kill Camera Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.KillCameraDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.KillCameraDuration = V; });
	AddGroup(Rows, TEXT("Permanence"), ESMAGFShowcaseGroup::Permanence);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bEnabled = V; });
	AddBool(Rows, TEXT("Keep Enemy Bodies Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepEnemyBodiesEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepEnemyBodiesEnabled = V; });
	AddBool(Rows, TEXT("Keep Shell Casings Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepShellCasingsEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepShellCasingsEnabled = V; });
	AddBool(Rows, TEXT("Keep Impact Evidence Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepImpactEvidenceEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepImpactEvidenceEnabled = V; });
	AddBool(Rows, TEXT("Keep Debris Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepDebrisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepDebrisEnabled = V; });
	AddBool(Rows, TEXT("Keep Aftermath Smoke Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepAftermathSmokeEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepAftermathSmokeEnabled = V; });
	AddNumber<float>(Rows, TEXT("Corpse Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.CorpseLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.CorpseLifetime = V; });
	AddNumber<float>(Rows, TEXT("Shell Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.ShellLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.ShellLifetime = V; });
	AddNumber<float>(Rows, TEXT("Debris Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.DebrisLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.DebrisLifetime = V; });
	AddNumber<float>(Rows, TEXT("Impact Evidence Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.ImpactEvidenceLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.ImpactEvidenceLifetime = V; });
	AddNumber<int32>(Rows, TEXT("Max Persistent Corpses"), 0, 1000, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.MaxPersistentCorpses; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.Permanence.MaxPersistentCorpses = V; });
	AddNumber<int32>(Rows, TEXT("Max Persistent Shells"), 0, 1000, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.MaxPersistentShells; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.Permanence.MaxPersistentShells = V; });
	AddNumber<int32>(Rows, TEXT("Max Persistent Debris"), 0, 1000, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.MaxPersistentDebris; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.Permanence.MaxPersistentDebris = V; });
	AddGroup(Rows, TEXT("Smoke & Mirrors"), ESMAGFShowcaseGroup::SmokeAndMirrors);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bEnabled = V; });
	AddBool(Rows, TEXT("Scripted Enemy Wave Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bScriptedEnemyWaveEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bScriptedEnemyWaveEnabled = V; });
	AddBool(Rows, TEXT("Forced Dramatic Kill Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bForcedDramaticKillEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bForcedDramaticKillEnabled = V; });
	AddBool(Rows, TEXT("Forced Explosion Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bForcedExplosionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bForcedExplosionEnabled = V; });
	AddBool(Rows, TEXT("Showcase Slow Motion Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bShowcaseSlowMotionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bShowcaseSlowMotionEnabled = V; });
	AddBool(Rows, TEXT("Predefined Showcase Spawns Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bPredefinedShowcaseSpawnsEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bPredefinedShowcaseSpawnsEnabled = V; });
	AddBool(Rows, TEXT("Automatic Encounter Reset Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bAutomaticEncounterResetEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bAutomaticEncounterResetEnabled = V; });
	AddBool(Rows, TEXT("Deterministic Random Seed Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bDeterministicRandomSeedEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bDeterministicRandomSeedEnabled = V; });
	AddBool(Rows, TEXT("Freeze Enemies Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bFreezeEnemiesEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bFreezeEnemiesEnabled = V; });
	AddBool(Rows, TEXT("Scripted Showcase Sequence Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bScriptedShowcaseSequenceEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bScriptedShowcaseSequenceEnabled = V; });
	AddNumber<int32>(Rows, TEXT("Wave Size"), 1, 100, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.WaveSize; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.SmokeAndMirrors.WaveSize = V; });
	AddNumber<float>(Rows, TEXT("Slow Motion Time Scale"), 0.01f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.SlowMotionTimeScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.SlowMotionTimeScale = V; });
	AddNumber<float>(Rows, TEXT("Slow Motion Duration"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.SlowMotionDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.SlowMotionDuration = V; });
	AddNumber<float>(Rows, TEXT("Encounter Reset Delay"), 0.0f, 120.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EncounterResetDelay; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EncounterResetDelay = V; });
	AddNumber<int32>(Rows, TEXT("Random Seed"), 0, 2147483647, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.RandomSeed; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.SmokeAndMirrors.RandomSeed = V; });
	AddNumber<float>(Rows, TEXT("Sequence Duration"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.SequenceDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.SequenceDuration = V; });
	AddGroup(Rows, TEXT("Focus Pocus"), ESMAGFShowcaseGroup::Focus);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bEnabled = V; });
	AddBool(Rows, TEXT("Aim Direction Framing Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bAimDirectionFramingEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bAimDirectionFramingEnabled = V; });
	AddBool(Rows, TEXT("Target Emphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bTargetEmphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bTargetEmphasisEnabled = V; });
	AddBool(Rows, TEXT("Contextual Camera Emphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bContextualCameraEmphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bContextualCameraEmphasisEnabled = V; });
	AddBool(Rows, TEXT("Contextual Lighting Pulse Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bContextualLightingPulseEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bContextualLightingPulseEnabled = V; });
	AddBool(Rows, TEXT("Contextual Post Process Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bContextualPostProcessEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bContextualPostProcessEnabled = V; });
	AddBool(Rows, TEXT("Background Deemphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bBackgroundDeemphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bBackgroundDeemphasisEnabled = V; });
	AddNumber<float>(Rows, TEXT("Focus Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusIntensity = V; });
	AddNumber<float>(Rows, TEXT("Focus Duration"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusDuration = V; });
	AddNumber<float>(Rows, TEXT("Focus Camera Bias"), 0.0f, 1000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusCameraBias; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusCameraBias = V; });
	AddNumber<float>(Rows, TEXT("Focus Vignette Amount"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusVignetteAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusVignetteAmount = V; });
	AddNumber<float>(Rows, TEXT("Focus Exposure Bias"), -5.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusExposureBias; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusExposureBias = V; });
	AddGroup(Rows, TEXT("Juice Responsibly"), ESMAGFShowcaseGroup::JuiceResponsibly);
	AddBool(Rows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.JuiceResponsibly.bEnabled = V; });
	AddBool(Rows, TEXT("Event Importance Scaling Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.bEventImportanceScalingEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.JuiceResponsibly.bEventImportanceScalingEnabled = V; });
	AddNumber<float>(Rows, TEXT("Minor Multiplier"), 0.0f, 4.0f, [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.MinorMultiplier; }, [](FSMAGFShowcaseSettings& S, float V) { S.JuiceResponsibly.MinorMultiplier = V; });
	AddNumber<float>(Rows, TEXT("Normal Multiplier"), 0.0f, 4.0f, [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.NormalMultiplier; }, [](FSMAGFShowcaseSettings& S, float V) { S.JuiceResponsibly.NormalMultiplier = V; });
	AddNumber<float>(Rows, TEXT("Major Multiplier"), 0.0f, 4.0f, [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.MajorMultiplier; }, [](FSMAGFShowcaseSettings& S, float V) { S.JuiceResponsibly.MajorMultiplier = V; });
	AddNumber<float>(Rows, TEXT("Critical Multiplier"), 0.0f, 4.0f, [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.CriticalMultiplier; }, [](FSMAGFShowcaseSettings& S, float V) { S.JuiceResponsibly.CriticalMultiplier = V; });
}
