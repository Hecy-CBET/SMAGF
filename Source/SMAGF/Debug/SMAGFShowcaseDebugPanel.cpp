#include "Debug/SMAGFShowcaseDebugPanel.h"
#include "Debug/SMAGFGameViewportClient.h"
#include "Debug/SMAGFDebugUISettings.h"
#include "Brushes/SlateColorBrush.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Feedback/SMAGFFeedbackSubsystem.h"
#include "SMAGFPlayerController.h"
#include "SMAGFGameMode.h"
#include "Engine/World.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "Styling/CoreStyle.h"
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
	case ESMAGFShowcasePreset::JuiceResponsibly: return FText::FromString(TEXT("Juice Responsibly"));
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
	Rows->AddSlot().AutoHeight().Padding(0, 4)[SNew(STextBlock).Text_Lambda([this]()
	{
		const auto* World = Controller.IsValid() ? Controller->GetWorld() : nullptr;
		const auto* Mode = World ? World->GetAuthGameMode<ASMAGFGameMode>() : nullptr;
		return FText::Format(FText::FromString(TEXT("Enemy Mode: {0} (Ctrl+G)")), Mode
			? StaticEnum<ESMAGFEnemyDebugMode>()->GetDisplayNameTextByValue(static_cast<int64>(Mode->GetEnemyDebugMode()))
			: FText::FromString(TEXT("Unavailable")));
	})];
	Rows->AddSlot().AutoHeight().Padding(0, 4)[SNew(STextBlock).AutoWrapText(true).Text(FText::FromString(TEXT("Ctrl+F closes; Ctrl+T restores missing enemies; PageUp/PageDown or [ / ] change layers. Expand a group to tune its features.")))];
	TSharedRef<SWrapBox> Presets = SNew(SWrapBox).UseAllottedSize(true);
	for (const ESMAGFShowcasePreset Preset : SMAGFOrderedPresets())
	{
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
	if (Key == EKeys::PageUp || Key == EKeys::PageDown || Key == EKeys::LeftBracket || Key == EKeys::RightBracket || Key == EKeys::Home || Key == EKeys::End)
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
TSharedRef<SVerticalBox> SSMAGFShowcaseDebugPanel::AddGroup(const TSharedRef<SVerticalBox>& Rows, const TCHAR* Label, ESMAGFShowcaseGroup Group)
{
	TSharedRef<SVerticalBox> Body=SNew(SVerticalBox);
	Rows->AddSlot().AutoHeight().Padding(0,8)[SNew(SExpandableArea).InitiallyCollapsed(true)
		.HeaderContent()[SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",19)).Text(FText::FromString(Label))]
		.BodyContent()[Body]];
	Body->AddSlot().AutoHeight().Padding(0, 3)[SNew(SButton).Text(FText::FromString(TEXT("Reset Group")))
		.OnClicked_Lambda([this, Group]() { if (Subsystem.IsValid()) { Subsystem->ResetCurrentGroupToDefaults(Group); } return FReply::Handled(); })];
	return Body;
}

void SSMAGFShowcaseDebugPanel::BuildControls(const TSharedRef<SVerticalBox>& Rows)
{
	AddBool(Rows, TEXT("Enable Showcase Features"), [](const FSMAGFShowcaseSettings& S) { return S.Master.bShowcaseEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Master.bShowcaseEnabled = V; });
	AddNumber<float>(Rows, TEXT("Master Juice Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Master.MasterIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Master.MasterIntensity = V; });
	TSharedRef<SVerticalBox> GroupRows = AddGroup(Rows, TEXT("Feeling Is Everything"), ESMAGFShowcaseGroup::Feeling);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bEnabled = V; });
	AddBool(GroupRows, TEXT("Fire Anticipation Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bFireAnticipationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bFireAnticipationEnabled = V; });
	AddBool(GroupRows, TEXT("Movement Acceleration Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bMovementAccelerationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bMovementAccelerationEnabled = V; });
	AddBool(GroupRows, TEXT("Recoil Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bRecoilEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bRecoilEnabled = V; });
	AddBool(GroupRows, TEXT("Impact Response Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bImpactResponseEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bImpactResponseEnabled = V; });
	AddBool(GroupRows, TEXT("Hit Stop Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bHitStopEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bHitStopEnabled = V; });
	AddBool(GroupRows, TEXT("Enemy Knockback Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bEnemyKnockbackEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bEnemyKnockbackEnabled = V; });
	AddBool(GroupRows, TEXT("Player Knockback Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bPlayerKnockbackEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bPlayerKnockbackEnabled = V; });
	AddBool(GroupRows, TEXT("Hit Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bHitReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bHitReactionEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Fire Anticipation Time"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.FireAnticipationTime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.FireAnticipationTime = V; });
	AddNumber<float>(GroupRows, TEXT("Anticipation Scale"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.AnticipationScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.AnticipationScale = V; });
	AddNumber<float>(GroupRows, TEXT("Movement Acceleration"), 0.0f, 10000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.MovementAcceleration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.MovementAcceleration = V; });
	AddNumber<float>(GroupRows, TEXT("Movement Deceleration"), 0.0f, 10000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.MovementDeceleration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.MovementDeceleration = V; });
	AddNumber<float>(GroupRows, TEXT("Recoil Distance"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilDistance = V; });
	AddNumber<float>(GroupRows, TEXT("Recoil Rotation"), 0.0f, 45.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilRotation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilRotation = V; });
	AddNumber<float>(GroupRows, TEXT("Recoil Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Recoil Recovery Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.RecoilRecoveryDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.RecoilRecoveryDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Hit Stop Duration"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitStopDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitStopDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Kill Hit Stop Duration"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.KillHitStopDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.KillHitStopDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Hit Stop Time Scale"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitStopTimeScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitStopTimeScale = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Knockback Strength"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.EnemyKnockbackStrength; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.EnemyKnockbackStrength = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Knockback Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.EnemyKnockbackDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.EnemyKnockbackDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Player Knockback Strength"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.PlayerKnockbackStrength; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.PlayerKnockbackStrength = V; });
	AddNumber<float>(GroupRows, TEXT("Player Knockback Recovery"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.PlayerKnockbackRecovery; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.PlayerKnockbackRecovery = V; });
	AddNumber<float>(GroupRows, TEXT("Hit Reaction Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitReactionDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitReactionDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Hit Reaction Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.HitReactionIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.HitReactionIntensity = V; });
	AddNumber<float>(GroupRows, TEXT("Impact Response Distance"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.ImpactResponseDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.ImpactResponseDistance = V; });
	AddNumber<float>(GroupRows, TEXT("Impact Response Duration"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.ImpactResponseDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.ImpactResponseDuration = V; });
	AddBool(GroupRows, TEXT("Projectile Readability Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Feeling.bProjectileReadabilityEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Feeling.bProjectileReadabilityEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Projectile Visual Scale"), 0.5f, 3.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.ProjectileVisualScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.ProjectileVisualScale = V; });
	AddNumber<float>(GroupRows, TEXT("Projectile Glow Fade Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Feeling.ProjectileGlowFadeDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Feeling.ProjectileGlowFadeDuration = V; });
	GroupRows = AddGroup(Rows, TEXT("Move It or Lose It"), ESMAGFShowcaseGroup::Animation);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bEnabled = V; });
	AddBool(GroupRows, TEXT("Fire Anticipation Animation Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bFireAnticipationAnimationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bFireAnticipationAnimationEnabled = V; });
	AddBool(GroupRows, TEXT("Weapon Recoil Animation Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bWeaponRecoilAnimationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bWeaponRecoilAnimationEnabled = V; });
	AddBool(GroupRows, TEXT("Character Fire Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bCharacterFireReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bCharacterFireReactionEnabled = V; });
	AddBool(GroupRows, TEXT("Enemy Hit Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bEnemyHitReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bEnemyHitReactionEnabled = V; });
	AddBool(GroupRows, TEXT("Enemy Death Reaction Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bEnemyDeathReactionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bEnemyDeathReactionEnabled = V; });
	AddBool(GroupRows, TEXT("Squash And Stretch Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bSquashAndStretchEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bSquashAndStretchEnabled = V; });
	AddBool(GroupRows, TEXT("Overshoot Recovery Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bOvershootRecoveryEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bOvershootRecoveryEnabled = V; });
	AddBool(GroupRows, TEXT("Procedural Wobble Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bProceduralWobbleEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bProceduralWobbleEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Animation Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.AnimationIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.AnimationIntensity = V; });
	AddNumber<float>(GroupRows, TEXT("Squash Amount"), 0.0f, 0.9f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.SquashAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.SquashAmount = V; });
	AddNumber<float>(GroupRows, TEXT("Stretch Amount"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.StretchAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.StretchAmount = V; });
	AddNumber<float>(GroupRows, TEXT("Overshoot Amount"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.OvershootAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.OvershootAmount = V; });
	AddNumber<float>(GroupRows, TEXT("Wobble Amount"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.WobbleAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.WobbleAmount = V; });
	AddNumber<float>(GroupRows, TEXT("Wobble Frequency"), 0.0f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.WobbleFrequency; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.WobbleFrequency = V; });
	AddNumber<float>(GroupRows, TEXT("Reaction Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.ReactionDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.ReactionDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Recovery Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.RecoveryDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.RecoveryDuration = V; });
GroupRows->AddSlot().AutoHeight().Padding(0, 6)[SNew(SComboButton)
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
	AddBool(GroupRows, TEXT("Projectile Animation Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Animation.bProjectileAnimationEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Animation.bProjectileAnimationEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Projectile Flight Stretch"), 1.0f, 4.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.ProjectileFlightStretch; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.ProjectileFlightStretch = V; });
	AddNumber<float>(GroupRows, TEXT("Projectile Flight Growth"), 1.0f, 3.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.ProjectileFlightGrowth; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.ProjectileFlightGrowth = V; });
	AddNumber<float>(GroupRows, TEXT("Projectile Impact Squash"), 0.1f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.ProjectileImpactSquash; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.ProjectileImpactSquash = V; });
	AddNumber<float>(GroupRows, TEXT("Projectile Impact Animation Duration"), 0.0f, 0.5f, [](const FSMAGFShowcaseSettings& S) { return S.Animation.ProjectileImpactAnimationDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Animation.ProjectileImpactAnimationDuration = V; });
	GroupRows = AddGroup(Rows, TEXT("Blood, Sweat & Pixelated Streaming"), ESMAGFShowcaseGroup::Audiovisual);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bEnabled = V; });
	AddBool(GroupRows, TEXT("Muzzle Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bMuzzleFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bMuzzleFlashEnabled = V; });
	AddBool(GroupRows, TEXT("Shot Light Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bShotLightFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bShotLightFlashEnabled = V; });
	AddBool(GroupRows, TEXT("Projectile Visual Scale Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bProjectileVisualScaleEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bProjectileVisualScaleEnabled = V; });
	AddBool(GroupRows, TEXT("Projectile Trail Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bProjectileTrailEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bProjectileTrailEnabled = V; });
	AddBool(GroupRows, TEXT("Shell Ejection Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bShellEjectionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bShellEjectionEnabled = V; });
	AddBool(GroupRows, TEXT("Fire Sound Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bFireSoundEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bFireSoundEnabled = V; });
	AddBool(GroupRows, TEXT("Layered Fire Sound Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bLayeredFireSoundEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bLayeredFireSoundEnabled = V; });
	AddBool(GroupRows, TEXT("Impact Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bImpactFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bImpactFlashEnabled = V; });
	AddBool(GroupRows, TEXT("Impact Effect Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bImpactEffectEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bImpactEffectEnabled = V; });
	AddBool(GroupRows, TEXT("Impact Sound Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bImpactSoundEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bImpactSoundEnabled = V; });
	AddBool(GroupRows, TEXT("Enemy Flash Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bEnemyFlashEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bEnemyFlashEnabled = V; });
	AddBool(GroupRows, TEXT("Death Effect Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bDeathEffectEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bDeathEffectEnabled = V; });
	AddBool(GroupRows, TEXT("Explosion Effect Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.bExplosionEffectEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Audiovisual.bExplosionEffectEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Muzzle Flash Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.MuzzleFlashScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.MuzzleFlashScale = V; });
	AddNumber<float>(GroupRows, TEXT("Muzzle Flash Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.MuzzleFlashDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.MuzzleFlashDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Shot Light Intensity"), 0.0f, 100000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ShotLightIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ShotLightIntensity = V; });
	AddNumber<float>(GroupRows, TEXT("Shot Light Radius"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ShotLightRadius; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ShotLightRadius = V; });
	AddNumber<float>(GroupRows, TEXT("Projectile Visual Scale"), 0.01f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ProjectileVisualScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ProjectileVisualScale = V; });
	AddNumber<float>(GroupRows, TEXT("Trail Duration"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.TrailDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.TrailDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Shell Lifetime"), 0.0f, 120.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ShellLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ShellLifetime = V; });
	AddNumber<float>(GroupRows, TEXT("Fire Sound Volume"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.FireSoundVolume; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.FireSoundVolume = V; });
	AddNumber<float>(GroupRows, TEXT("Fire Sound Pitch Variation"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.FireSoundPitchVariation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.FireSoundPitchVariation = V; });
	AddNumber<float>(GroupRows, TEXT("Impact Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ImpactScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ImpactScale = V; });
	AddNumber<float>(GroupRows, TEXT("Impact Duration"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ImpactDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ImpactDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Impact Light Intensity"), 0.0f, 100000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ImpactLightIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ImpactLightIntensity = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Flash Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.EnemyFlashDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.EnemyFlashDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Death Effect Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.DeathEffectScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.DeathEffectScale = V; });
	AddNumber<float>(GroupRows, TEXT("Explosion Scale"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Audiovisual.ExplosionScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.Audiovisual.ExplosionScale = V; });
	GroupRows = AddGroup(Rows, TEXT("Camera"), ESMAGFShowcaseGroup::Camera);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bEnabled = V; });
	AddBool(GroupRows, TEXT("Camera Follow Smoothing Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bCameraFollowSmoothingEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bCameraFollowSmoothingEnabled = V; });
	AddBool(GroupRows, TEXT("Aim Look Ahead Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bAimLookAheadEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bAimLookAheadEnabled = V; });
	AddBool(GroupRows, TEXT("Screen Shake Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bScreenShakeEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bScreenShakeEnabled = V; });
	AddBool(GroupRows, TEXT("Directional Camera Kick Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bDirectionalCameraKickEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bDirectionalCameraKickEnabled = V; });
	AddBool(GroupRows, TEXT("Kill Camera Emphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Camera.bKillCameraEmphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Camera.bKillCameraEmphasisEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Camera Follow Speed"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraFollowSpeed; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraFollowSpeed = V; });
	AddNumber<float>(GroupRows, TEXT("Aim Look Ahead Distance"), 0.0f, 1000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.AimLookAheadDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.AimLookAheadDistance = V; });
	AddNumber<float>(GroupRows, TEXT("Aim Look Ahead Interpolation"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.AimLookAheadInterpolation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.AimLookAheadInterpolation = V; });
	AddNumber<float>(GroupRows, TEXT("Shake Translation Amplitude"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeTranslationAmplitude; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeTranslationAmplitude = V; });
	AddNumber<float>(GroupRows, TEXT("Shake Rotation Amplitude"), 0.0f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeRotationAmplitude; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeRotationAmplitude = V; });
	AddNumber<float>(GroupRows, TEXT("Shake Frequency"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeFrequency; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeFrequency = V; });
	AddNumber<float>(GroupRows, TEXT("Shake Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Shake Falloff"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.ShakeFalloff; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.ShakeFalloff = V; });
	AddNumber<float>(GroupRows, TEXT("Camera Kick Distance"), 0.0f, 100.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickDistance; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickDistance = V; });
	AddNumber<float>(GroupRows, TEXT("Camera Kick Rotation"), 0.0f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickRotation; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickRotation = V; });
	AddNumber<float>(GroupRows, TEXT("Camera Kick Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Camera Kick Recovery"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.CameraKickRecovery; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.CameraKickRecovery = V; });
	AddNumber<float>(GroupRows, TEXT("Kill Camera Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Camera.KillCameraDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Camera.KillCameraDuration = V; });
	GroupRows = AddGroup(Rows, TEXT("Permanence"), ESMAGFShowcaseGroup::Permanence);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bEnabled = V; });
	AddBool(GroupRows, TEXT("Keep Enemy Bodies Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepEnemyBodiesEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepEnemyBodiesEnabled = V; });
	AddBool(GroupRows, TEXT("Keep Shell Casings Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepShellCasingsEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepShellCasingsEnabled = V; });
	AddBool(GroupRows, TEXT("Keep Impact Evidence Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepImpactEvidenceEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepImpactEvidenceEnabled = V; });
	AddBool(GroupRows, TEXT("Keep Debris Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepDebrisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepDebrisEnabled = V; });
	AddBool(GroupRows, TEXT("Keep Aftermath Smoke Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Permanence.bKeepAftermathSmokeEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Permanence.bKeepAftermathSmokeEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Corpse Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.CorpseLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.CorpseLifetime = V; });
	AddNumber<float>(GroupRows, TEXT("Corpse Dissolve Duration"), 0.0f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.CorpseDissolveDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.CorpseDissolveDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Shell Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.ShellLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.ShellLifetime = V; });
	AddNumber<float>(GroupRows, TEXT("Debris Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.DebrisLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.DebrisLifetime = V; });
	AddNumber<float>(GroupRows, TEXT("Impact Evidence Lifetime"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.ImpactEvidenceLifetime; }, [](FSMAGFShowcaseSettings& S, float V) { S.Permanence.ImpactEvidenceLifetime = V; });
	AddNumber<int32>(GroupRows, TEXT("Max Persistent Corpses"), 0, 1000, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.MaxPersistentCorpses; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.Permanence.MaxPersistentCorpses = V; });
	AddNumber<int32>(GroupRows, TEXT("Max Persistent Shells"), 0, 1000, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.MaxPersistentShells; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.Permanence.MaxPersistentShells = V; });
	AddNumber<int32>(GroupRows, TEXT("Max Persistent Debris"), 0, 1000, [](const FSMAGFShowcaseSettings& S) { return S.Permanence.MaxPersistentDebris; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.Permanence.MaxPersistentDebris = V; });
	GroupRows = AddGroup(Rows, TEXT("Smoke & Mirrors"), ESMAGFShowcaseGroup::SmokeAndMirrors);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bEnabled = V; });
	AddBool(GroupRows, TEXT("Enemy Movement Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bEnemyMovementEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bEnemyMovementEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Attack Start Range"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyAttackStartRange; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyAttackStartRange = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Move Speed Ratio"), 0.1f, 1.5f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyMoveSpeedRatio; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyMoveSpeedRatio = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Move Speed Deviation"), 0.0f, 0.5f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyMoveSpeedDeviation; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyMoveSpeedDeviation = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Move Acceleration"), 1.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyMoveAcceleration; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyMoveAcceleration = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Move Deceleration"), 1.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyMoveDeceleration; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyMoveDeceleration = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Range Recheck Interval"), 0.1f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyRangeRecheckInterval; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyRangeRecheckInterval = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Range Recheck Jitter"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyRangeRecheckJitter; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyRangeRecheckJitter = V; });
	AddBool(GroupRows, TEXT("Enemy Threat Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bEnemyThreatEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bEnemyThreatEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Attack Exit Range"), 0.0f, 5000.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyEngagementRange; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyEngagementRange = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Telegraph Duration"), 0.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyTelegraphDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyTelegraphDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Projectile Speed"), 100.0f, 3000.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyProjectileSpeed; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyProjectileSpeed = V; });
	AddNumber<float>(GroupRows, TEXT("Enemy Attack Cooldown"), 0.1f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EnemyAttackCooldown; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EnemyAttackCooldown = V; });
	AddBool(GroupRows, TEXT("Scripted Enemy Wave Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bScriptedEnemyWaveEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bScriptedEnemyWaveEnabled = V; });
	AddBool(GroupRows, TEXT("Forced Dramatic Kill Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bForcedDramaticKillEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bForcedDramaticKillEnabled = V; });
	AddBool(GroupRows, TEXT("Forced Explosion Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bForcedExplosionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bForcedExplosionEnabled = V; });
	AddBool(GroupRows, TEXT("Showcase Slow Motion Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bShowcaseSlowMotionEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bShowcaseSlowMotionEnabled = V; });
	AddBool(GroupRows, TEXT("Predefined Showcase Spawns Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bPredefinedShowcaseSpawnsEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bPredefinedShowcaseSpawnsEnabled = V; });
	AddBool(GroupRows, TEXT("Automatic Encounter Reset Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bAutomaticEncounterResetEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bAutomaticEncounterResetEnabled = V; });
	AddBool(GroupRows, TEXT("Deterministic Random Seed Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bDeterministicRandomSeedEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bDeterministicRandomSeedEnabled = V; });
	AddBool(GroupRows, TEXT("Freeze Enemies Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bFreezeEnemiesEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bFreezeEnemiesEnabled = V; });
	AddBool(GroupRows, TEXT("Scripted Showcase Sequence Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.bScriptedShowcaseSequenceEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.SmokeAndMirrors.bScriptedShowcaseSequenceEnabled = V; });
	AddNumber<int32>(GroupRows, TEXT("Wave Size"), 1, 100, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.WaveSize; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.SmokeAndMirrors.WaveSize = V; });
	AddNumber<float>(GroupRows, TEXT("Slow Motion Time Scale"), 0.01f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.SlowMotionTimeScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.SlowMotionTimeScale = V; });
	AddNumber<float>(GroupRows, TEXT("Slow Motion Duration"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.SlowMotionDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.SlowMotionDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Encounter Reset Delay"), 0.0f, 120.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.EncounterResetDelay; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.EncounterResetDelay = V; });
	AddNumber<int32>(GroupRows, TEXT("Random Seed"), 0, 2147483647, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.RandomSeed; }, [](FSMAGFShowcaseSettings& S, int32 V) { S.SmokeAndMirrors.RandomSeed = V; });
	AddNumber<float>(GroupRows, TEXT("Sequence Duration"), 0.0f, 300.0f, [](const FSMAGFShowcaseSettings& S) { return S.SmokeAndMirrors.SequenceDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.SmokeAndMirrors.SequenceDuration = V; });
	GroupRows = AddGroup(Rows, TEXT("Focus Pocus"), ESMAGFShowcaseGroup::Focus);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bEnabled = V; });
	AddBool(GroupRows, TEXT("Aim Direction Framing Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bAimDirectionFramingEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bAimDirectionFramingEnabled = V; });
	AddBool(GroupRows, TEXT("Target Emphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bTargetEmphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bTargetEmphasisEnabled = V; });
	AddBool(GroupRows, TEXT("Contextual Camera Emphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bContextualCameraEmphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bContextualCameraEmphasisEnabled = V; });
	AddBool(GroupRows, TEXT("Contextual Lighting Pulse Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bContextualLightingPulseEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bContextualLightingPulseEnabled = V; });
	AddBool(GroupRows, TEXT("Contextual Post Process Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bContextualPostProcessEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bContextualPostProcessEnabled = V; });
	AddBool(GroupRows, TEXT("Background Deemphasis Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.Focus.bBackgroundDeemphasisEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.Focus.bBackgroundDeemphasisEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Intensity"), 0.0f, 2.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusIntensity; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusIntensity = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Release Grace"), 0.0f, 10.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusDuration; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusDuration = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Camera Bias"), 0.0f, 1000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusCameraBias; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusCameraBias = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Vignette Amount"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusVignetteAmount; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusVignetteAmount = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Exposure Bias"), -5.0f, 5.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusExposureBias; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusExposureBias = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Framing Weight"), 0.0f, 1.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusFramingWeight; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusFramingWeight = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Max Camera Offset"), 0.0f, 1000.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusMaxCameraOffset; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusMaxCameraOffset = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Camera Interp Speed"), 0.1f, 30.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusCameraInterpSpeed; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusCameraInterpSpeed = V; });
	AddNumber<float>(GroupRows, TEXT("Focus FOV Reduction"), 0.0f, 15.0f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusFOVReduction; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusFOVReduction = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Acquire Radius (viewport fraction)"), .01f, .50f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusAcquireRadius; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusAcquireRadius = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Release Radius (viewport fraction)"), .01f, .75f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusReleaseRadius; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusReleaseRadius = V; });
	AddNumber<float>(GroupRows, TEXT("Focus Switch Advantage (viewport fraction)"), 0.f, .25f, [](const FSMAGFShowcaseSettings& S) { return S.Focus.FocusSwitchAdvantage; }, [](FSMAGFShowcaseSettings& S, float V) { S.Focus.FocusSwitchAdvantage = V; });
	GroupRows = AddGroup(Rows, TEXT("Juice Responsibly"), ESMAGFShowcaseGroup::JuiceResponsibly);
	AddBool(GroupRows, TEXT("Group Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.bEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.JuiceResponsibly.bEnabled = V; });
	AddBool(GroupRows, TEXT("Presentation Pressure Enabled"), [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.bEventImportanceScalingEnabled; }, [](FSMAGFShowcaseSettings& S, bool V) { S.JuiceResponsibly.bEventImportanceScalingEnabled = V; });
	AddNumber<float>(GroupRows, TEXT("Budget Strength"), 0.f, 1.f, [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.JuiceBudgetStrength; }, [](FSMAGFShowcaseSettings& S, float V) { S.JuiceResponsibly.JuiceBudgetStrength = V; });
	AddNumber<float>(GroupRows, TEXT("Pressure Decay"), .05f, 10.f, [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.JuicePressureDecay; }, [](FSMAGFShowcaseSettings& S, float V) { S.JuiceResponsibly.JuicePressureDecay = V; });
	AddNumber<float>(GroupRows, TEXT("Minimum Response Scale"), 0.f, 1.f, [](const FSMAGFShowcaseSettings& S) { return S.JuiceResponsibly.JuiceMinimumScale; }, [](FSMAGFShowcaseSettings& S, float V) { S.JuiceResponsibly.JuiceMinimumScale = V; });
	GroupRows->AddSlot().AutoHeight().Padding(0, 4)
	[SNew(STextBlock).Text_Lambda([this]()
	{
		const auto* World = Subsystem.IsValid() ? Subsystem->GetWorld() : nullptr;
		const auto* Feedback = World ? World->GetSubsystem<USMAGFFeedbackSubsystem>() : nullptr;
		if (!Feedback) { return FText::FromString(TEXT("Presentation pressure: unavailable")); }
		return FText::FromString(FString::Printf(TEXT("Presentation pressure: %.3f | Last scale: %.3f | Last importance: %s"),
			Feedback->GetPresentationPressure(), Feedback->GetLastPresentationScale(),
			*StaticEnum<ESMAGFEventImportance>()->GetNameStringByValue(static_cast<int64>(Feedback->GetLastImportance()))));
	})];
}
