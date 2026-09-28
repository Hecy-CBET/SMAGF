#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "SMAGFGameViewportClient.generated.h"

/** Project-wide runtime shortcut, independent of map-specific gameplay classes. */
UCLASS()
class SMAGF_API USMAGFGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()
public:
	virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
	virtual void LostFocus(FViewport* InViewport) override;
	virtual void Tick(float DeltaTime) override;

	/** Full current persistent-map reload. Future UI can call this without simulating keys. */
	UFUNCTION(BlueprintCallable, Category="SMAGF|Debug")
	bool RestartCurrentMap();

private:
	void RefreshRestartGuard();
	bool bRPressed = false;
	bool bMapRestartRequested = false;
	TWeakObjectPtr<UWorld> RestartWorld;
	double RestartRequestedAt = 0.0;
};
