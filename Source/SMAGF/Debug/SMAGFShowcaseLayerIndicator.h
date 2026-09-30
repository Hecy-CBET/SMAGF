#pragma once
#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

class USMAGFShowcaseSubsystem;
class STextBlock;

/** Passive, event-refreshed label. Owns no gameplay or input state. */
class SSMAGFShowcaseLayerIndicator : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSMAGFShowcaseLayerIndicator) {}
		SLATE_ARGUMENT(USMAGFShowcaseSubsystem*, Subsystem)
	SLATE_END_ARGS()
	void Construct(const FArguments& Args);
	virtual ~SSMAGFShowcaseLayerIndicator() override;
	void Disconnect();
	FText GetLabel() const { return Label; }
private:
	void Refresh();
	FText Label;
	TSharedPtr<STextBlock> Text;
	TWeakObjectPtr<USMAGFShowcaseSubsystem> Subsystem;
	FDelegateHandle Handle;
};
