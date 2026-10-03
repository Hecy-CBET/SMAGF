#include "Debug/SMAGFShowcaseLayerIndicator.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Brushes/SlateColorBrush.h"
#include "Styling/CoreStyle.h"

void SSMAGFShowcaseLayerIndicator::Construct(const FArguments& Args)
{
	Subsystem = Args._Subsystem;
	SetVisibility(EVisibility::HitTestInvisible);
	static const FSlateColorBrush Background(FLinearColor::White);
	ChildSlot.HAlign(HAlign_Center).VAlign(VAlign_Top).Padding(0,14,0,0)
	[
		SNew(SBorder).BorderImage(&Background).BorderBackgroundColor(FLinearColor(.045f,.045f,.045f,.7f)).Padding(FMargin(12,6))
		[SAssignNew(Text,STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Bold",28)).ColorAndOpacity(FLinearColor::White)]
	];
	Refresh();
	if (Subsystem.IsValid()) { Handle=Subsystem->OnPresetChangedNative.AddSP(this,&SSMAGFShowcaseLayerIndicator::Refresh); }
}
void SSMAGFShowcaseLayerIndicator::Refresh()
{
	const auto P=Subsystem.IsValid()?Subsystem->GetShowcasePreset():ESMAGFShowcasePreset::Baseline;
	FString Value;
	switch(P)
	{
	case ESMAGFShowcasePreset::SmokeAndMirrors: Value=TEXT("SMOKE & MIRRORS"); break;
	case ESMAGFShowcasePreset::JuiceResponsibly: Value=TEXT("JUICE RESPONSIBLY"); break;
	case ESMAGFShowcasePreset::FullShowcase: Value=TEXT("FULL SHOWCASE"); break;
	default: Value=StaticEnum<ESMAGFShowcasePreset>()->GetNameStringByValue(int64(P)).ToUpper(); break;
	}
	Label=FText::FromString(Value); Text->SetText(Label);
}
void SSMAGFShowcaseLayerIndicator::Disconnect()
{
	if (Subsystem.IsValid()) { Subsystem->OnPresetChangedNative.Remove(Handle); }
	Subsystem.Reset();
}
SSMAGFShowcaseLayerIndicator::~SSMAGFShowcaseLayerIndicator() { Disconnect(); }
