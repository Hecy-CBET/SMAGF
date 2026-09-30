#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Showcase/SMAGFShowcaseTypes.h"

class ASMAGFPlayerController;
class USMAGFShowcaseSubsystem;
class SVerticalBox;

/** Event-refreshed read model only. All mutation goes through the world subsystem. */
class SSMAGFShowcaseDebugPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSMAGFShowcaseDebugPanel) {}
		SLATE_ARGUMENT(ASMAGFPlayerController*, Controller)
		SLATE_ARGUMENT(USMAGFShowcaseSubsystem*, Subsystem)
	SLATE_END_ARGS()
	void Construct(const FArguments& Args);
	virtual ~SSMAGFShowcaseDebugPanel() override;
	void Disconnect();
	virtual bool SupportsKeyboardFocus() const override { return true; }
	virtual FReply OnPreviewKeyDown(const FGeometry& Geometry, const FKeyEvent& Event) override;
private:
	void Refresh();
	void Edit(TFunction<void(FSMAGFShowcaseSettings&)> Mutation);
	void BuildControls(const TSharedRef<SVerticalBox>& Rows);
	void AddBool(const TSharedRef<SVerticalBox>& Rows, const TCHAR* Label,
		TFunction<bool(const FSMAGFShowcaseSettings&)> Read, TFunction<void(FSMAGFShowcaseSettings&, bool)> Write);
	template<typename T> void AddNumber(const TSharedRef<SVerticalBox>& Rows, const TCHAR* Label, T Min, T Max,
		TFunction<T(const FSMAGFShowcaseSettings&)> Read, TFunction<void(FSMAGFShowcaseSettings&, T)> Write);
	TSharedRef<SVerticalBox> AddGroup(const TSharedRef<SVerticalBox>& Rows, const TCHAR* Label, ESMAGFShowcaseGroup Group);
	FText PresetText() const;
	TWeakObjectPtr<ASMAGFPlayerController> Controller;
	TWeakObjectPtr<USMAGFShowcaseSubsystem> Subsystem;
	FSMAGFShowcaseSettings DisplaySettings;
	ESMAGFShowcasePreset DisplayPreset = ESMAGFShowcasePreset::Baseline;
	ESMAGFShowcasePreset DisplayLastNamed = ESMAGFShowcasePreset::Baseline;
	bool bHasCustom = false;
	bool bRefreshing = false;
	FDelegateHandle SettingsHandle;
	FDelegateHandle PresetHandle;
};
