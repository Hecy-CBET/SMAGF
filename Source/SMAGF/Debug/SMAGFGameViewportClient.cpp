#include "Debug/SMAGFGameViewportClient.h"
#include "Showcase/SMAGFShowcaseDeveloperSettings.h"
#include "SMAGF.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "HAL/PlatformTime.h"
#include "InputKeyEventArgs.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/PackageName.h"
#include "UnrealClient.h"
#include "UObject/Package.h"

bool USMAGFGameViewportClient::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (EventArgs.Key == EKeys::R)
	{
		if (EventArgs.Event == IE_Released)
		{
			bRPressed = false;
		}
		else if (EventArgs.Event == IE_Pressed && !bRPressed)
		{
			bRPressed = true;
			FViewport* InputViewport = EventArgs.Viewport;
			if (GetDefault<USMAGFShowcaseDeveloperSettings>()->bEnableQuickRestartShortcut
				&& InputViewport && InputViewport == Viewport && InputViewport->HasFocus()
				&& (InputViewport->KeyState(EKeys::LeftControl) || InputViewport->KeyState(EKeys::RightControl)))
			{
				RestartCurrentMap();
				return true;
			}
		}
	}
	return Super::InputKey(EventArgs);
}

void USMAGFGameViewportClient::LostFocus(FViewport* InViewport)
{
	bRPressed = false;
	Super::LostFocus(InViewport);
}

void USMAGFGameViewportClient::RefreshRestartGuard()
{
	if (!bMapRestartRequested)
	{
		return;
	}
	if (!RestartWorld.IsValid() || GetWorld() != RestartWorld.Get())
	{
		bMapRestartRequested = false;
		RestartWorld.Reset();
	}
	else if (FPlatformTime::Seconds() - RestartRequestedAt > 10.0)
	{
		// The viewport can outlive travel. Permit retry if travel failed to replace its world.
		UE_LOG(LogSMAGF, Warning, TEXT("SMAGF Quick Restart: travel did not replace the world; request guard cleared."));
		bMapRestartRequested = false;
		RestartWorld.Reset();
	}
}

void USMAGFGameViewportClient::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	RefreshRestartGuard();
}

bool USMAGFGameViewportClient::RestartCurrentMap()
{
	RefreshRestartGuard();
	UWorld* CurrentWorld = GetWorld();
	if (bMapRestartRequested || !CurrentWorld || !CurrentWorld->PersistentLevel
		|| (CurrentWorld->WorldType != EWorldType::Game && CurrentWorld->WorldType != EWorldType::PIE)
		|| !CurrentWorld->NextURL.IsEmpty())
	{
		return false;
	}

	// Retain the full package path (not an ambiguous short asset name) and strip PIE decoration.
	const FString MapPackage = UWorld::RemovePIEPrefix(CurrentWorld->PersistentLevel->GetOutermost()->GetName());
	if (!FPackageName::IsValidLongPackageName(MapPackage) || !FPackageName::DoesPackageExist(MapPackage))
	{
		UE_LOG(LogSMAGF, Warning, TEXT("SMAGF Quick Restart: cannot resolve persistent map '%s'."), *MapPackage);
		return false;
	}
	bMapRestartRequested = true;
	RestartWorld = CurrentWorld;
	RestartRequestedAt = FPlatformTime::Seconds();
	UE_LOG(LogSMAGF, Log, TEXT("SMAGF Quick Restart: %s"), *MapPackage);
	UGameplayStatics::OpenLevel(CurrentWorld, FName(*MapPackage), true);
	return true;
}
