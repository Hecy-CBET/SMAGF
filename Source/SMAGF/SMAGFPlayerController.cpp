#include "SMAGFPlayerController.h"
#include "Debug/SMAGFShowcaseLayerIndicator.h"
#include "Camera/SMAGFPlayerCameraManager.h"
#include "SMAGF.h"
#include "SMAGFCharacter.h"
#include "Combat/SMAGFWeaponComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "InputAction.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "InputModifiers.h"
#include "Debug/SMAGFShowcaseDebugPanel.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Engine/GameViewportClient.h"
#include "Engine/World.h"
#include "Framework/Application/SlateApplication.h"
#include "UnrealClient.h"
#include "InputKeyEventArgs.h"

ASMAGFPlayerController::ASMAGFPlayerController()
{
	PlayerCameraManagerClass = ASMAGFPlayerCameraManager::StaticClass();
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
}

void ASMAGFPlayerController::CreateInputObjects()
{
	if (BaselineMapping)
	{
		return;
	}
	BaselineMapping = NewObject<UInputMappingContext>(this, TEXT("IMC_SMAGFBaseline"), RF_Transient);
	MoveAction = NewObject<UInputAction>(this, TEXT("IA_SMAGFMove"), RF_Transient);
	MoveAction->ValueType = EInputActionValueType::Axis2D;
	MoveAction->AccumulationBehavior = EInputActionAccumulationBehavior::Cumulative;
	FireAction = NewObject<UInputAction>(this, TEXT("IA_SMAGFFire"), RF_Transient);
	FireAction->ValueType = EInputActionValueType::Boolean;
	BaselineMapping->MapKey(MoveAction, EKeys::W);
	BaselineMapping->MapKey(MoveAction, EKeys::S).Modifiers.Add(NewObject<UInputModifierNegate>(BaselineMapping));
	for (const FKey Key : { EKeys::D, EKeys::A })
	{
		FEnhancedActionKeyMapping& Mapping = BaselineMapping->MapKey(MoveAction, Key);
		UInputModifierSwizzleAxis* Swizzle = NewObject<UInputModifierSwizzleAxis>(BaselineMapping);
		Swizzle->Order = EInputAxisSwizzle::YXZ;
		Mapping.Modifiers.Add(Swizzle);
		if (Key == EKeys::A)
		{
			Mapping.Modifiers.Add(NewObject<UInputModifierNegate>(BaselineMapping));
		}
	}
	BaselineMapping->MapKey(FireAction, EKeys::LeftMouseButton);
}

void ASMAGFPlayerController::AddOwnedMapping()
{
	if (!IsLocalController() || !BaselineMapping)
	{
		return;
	}
	if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
	{
		if (MappingSubsystem.Get() != Subsystem)
		{
			RemoveOwnedMapping();
		}
		if (!Subsystem->HasMappingContext(BaselineMapping))
		{
			Subsystem->AddMappingContext(BaselineMapping, 0);
		}
		MappingSubsystem = Subsystem;
	}
}

void ASMAGFPlayerController::RemoveOwnedMapping()
{
	if (MappingSubsystem.IsValid() && BaselineMapping)
	{
		MappingSubsystem->RemoveMappingContext(BaselineMapping);
	}
	MappingSubsystem.Reset();
}

void ASMAGFPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	if (!IsLocalController())
	{
		return;
	}
	CreateInputObjects();
	AddOwnedMapping();
	FInputModeGameAndUI Mode;
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
	InputComponent->BindKey(EKeys::PageUp, IE_Pressed, this, &ASMAGFPlayerController::PreviousShowcaseLayer);
	InputComponent->BindKey(EKeys::PageDown, IE_Pressed, this, &ASMAGFPlayerController::NextShowcaseLayer);
	InputComponent->BindKey(EKeys::Home, IE_Pressed, this, &ASMAGFPlayerController::BaselineShowcaseLayer);
	InputComponent->BindKey(EKeys::End, IE_Pressed, this, &ASMAGFPlayerController::FullShowcaseLayer);
	if (UEnhancedInputComponent* Input = Cast<UEnhancedInputComponent>(InputComponent))
	{
		Input->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASMAGFPlayerController::OnMove);
		Input->BindAction(MoveAction, ETriggerEvent::Completed, this, &ASMAGFPlayerController::OnMoveReleased);
		Input->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ASMAGFPlayerController::OnMoveReleased);
		Input->BindAction(FireAction, ETriggerEvent::Started, this, &ASMAGFPlayerController::OnFireStarted);
		Input->BindAction(FireAction, ETriggerEvent::Completed, this, &ASMAGFPlayerController::OnFireReleased);
		Input->BindAction(FireAction, ETriggerEvent::Canceled, this, &ASMAGFPlayerController::OnFireReleased);
	}
	else
	{
		UE_LOG(LogSMAGF, Error, TEXT("SMAGF requires an EnhancedInputComponent."));
	}
}

void ASMAGFPlayerController::OnPossess(APawn* InPawn)
{
	CancelHeldInput();
	Super::OnPossess(InPawn);
	LastAimDirection = InPawn->GetActorForwardVector().GetSafeNormal2D();
	CreateInputObjects();
	AddOwnedMapping();
	if (ASMAGFCharacter* ControlledCharacter = Cast<ASMAGFCharacter>(InPawn))
	{
		ControlledCharacter->GetWeapon()->AddTickPrerequisiteActor(this);
	}
}

void ASMAGFPlayerController::OnUnPossess()
{
	RemoveShowcasePanel(true);
	CancelHeldInput();
	if (ASMAGFCharacter* ControlledCharacter = Cast<ASMAGFCharacter>(GetPawn()))
	{
		ControlledCharacter->GetWeapon()->RemoveTickPrerequisiteActor(this);
	}
	RemoveOwnedMapping();
	Super::OnUnPossess();
}

void ASMAGFPlayerController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (LayerIndicator.IsValid())
	{
		LayerIndicator->Disconnect();
		if (GetWorld() && GetWorld()->GetGameViewport()) { GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(LayerIndicator.ToSharedRef()); }
		LayerIndicator.Reset();
	}
	RemoveShowcasePanel(false);
	CancelHeldInput();
	RemoveOwnedMapping();
	Super::EndPlay(EndPlayReason);
}

void ASMAGFPlayerController::BeginPlay()
{
	Super::BeginPlay();
	if (IsLocalController() && GetWorld()->GetGameViewport() && FSlateApplication::IsInitialized())
	{
		LayerIndicator=SNew(SSMAGFShowcaseLayerIndicator).Subsystem(GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>());
		GetWorld()->GetGameViewport()->AddViewportWidgetContent(LayerIndicator.ToSharedRef(),10);
	}
}
FText ASMAGFPlayerController::GetShowcaseLayerLabel() const
{
	return LayerIndicator.IsValid() ? LayerIndicator->GetLabel() : FText::GetEmpty();
}

void ASMAGFPlayerController::FlushPressedKeys()
{
	Super::FlushPressedKeys();
	CancelHeldInput();
	bRequireFireRelease = true;
	bRequireMoveRelease = true;
	// Viewport-to-Slate focus transfers also flush keys. They must not close UI.
}

void ASMAGFPlayerController::CancelHeldInput()
{
	MoveIntent = FVector2D::ZeroVector;
	OnFireReleased();
	if (ASMAGFCharacter* ControlledCharacter = Cast<ASMAGFCharacter>(GetPawn()))
	{
		ControlledCharacter->ConsumeMovementInputVector();
		ControlledCharacter->GetCharacterMovement()->StopMovementImmediately();
	}
}

void ASMAGFPlayerController::OnMove(const FInputActionValue& Value)
{
	if (IsShowcasePanelOpen() || bRequireMoveRelease) { return; }
	MoveIntent = Value.Get<FVector2D>();
}

void ASMAGFPlayerController::OnMoveReleased()
{
	MoveIntent = FVector2D::ZeroVector;
}

void ASMAGFPlayerController::OnFireStarted()
{
	if (IsShowcasePanelOpen() || bRequireFireRelease) { return; }
	UpdateAim();
	if (ASMAGFCharacter* ControlledCharacter = Cast<ASMAGFCharacter>(GetPawn()))
	{
		ControlledCharacter->GetWeapon()->StartFire();
	}
}

void ASMAGFPlayerController::OnFireReleased()
{
	if (ASMAGFCharacter* ControlledCharacter = Cast<ASMAGFCharacter>(GetPawn()))
	{
		ControlledCharacter->GetWeapon()->StopFire();
	}
}

void ASMAGFPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	if (IsLocalController() && GetWorld() && GetWorld()->GetGameViewport())
	{
		if (FViewport* Viewport = GetWorld()->GetGameViewport()->Viewport)
		{
			// Use physical viewport state: PlayerInput may have just been flushed.
			// Slate also tracks presses captured by UI controls outside SViewport.
			const bool bMouseHeld = FSlateApplication::IsInitialized()
				? FSlateApplication::Get().GetPressedMouseButtons().Contains(EKeys::LeftMouseButton)
				: Viewport->KeyState(EKeys::LeftMouseButton);
			if (!bMouseHeld) { bRequireFireRelease = false; }
			if (!Viewport->KeyState(EKeys::W) && !Viewport->KeyState(EKeys::A)
				&& !Viewport->KeyState(EKeys::S) && !Viewport->KeyState(EKeys::D)) { bRequireMoveRelease = false; }
		}
	}
	if (IsShowcasePanelOpen()) { return; }
	if (IsLocalController())
	{
		UpdateAim();
		if (ASMAGFCharacter* ControlledCharacter = Cast<ASMAGFCharacter>(GetPawn()))
		{
			ControlledCharacter->MoveInCameraPlane(MoveIntent);
		}
	}
}

void ASMAGFPlayerController::UpdateAim()
{
	ASMAGFCharacter* ControlledCharacter = Cast<ASMAGFCharacter>(GetPawn());
	if (!ControlledCharacter)
	{
		return;
	}
	FVector RayOrigin, RayDirection;
	const FVector Muzzle = ControlledCharacter->GetWeapon()->GetMuzzleLocation();
	float MouseX = 0.f, MouseY = 0.f;
	const auto* Camera = Cast<ASMAGFPlayerCameraManager>(PlayerCameraManager);
	const bool bRay = Camera && GetMousePosition(MouseX, MouseY)
		&& Camera->DeprojectLogicalScreen(FVector2D(MouseX, MouseY), RayOrigin, RayDirection);
	if (bRay && !RayOrigin.ContainsNaN()
		&& !RayDirection.ContainsNaN() && !Muzzle.ContainsNaN() && FMath::Abs(RayDirection.Z) > KINDA_SMALL_NUMBER)
	{
		const double Distance = (Muzzle.Z - RayOrigin.Z) / RayDirection.Z;
		if (FMath::IsFinite(Distance) && Distance > 0.0)
		{
			const FVector AimPoint = RayOrigin + RayDirection * Distance;
			// Aim from the yaw pivot: the forward muzzle offset then lies on the same ray.
			const FVector Direction = (AimPoint - ControlledCharacter->GetActorLocation()).GetSafeNormal2D();
			if (!AimPoint.ContainsNaN() && !Direction.ContainsNaN() && !Direction.IsNearlyZero())
			{
				LastAimDirection = Direction;
			}
		}
	}
	ControlledCharacter->ApplyAimDirection(LastAimDirection);
}

void ASMAGFPlayerController::ToggleShowcasePanel()
{
	if (IsShowcasePanelOpen()) { CloseShowcasePanel(); return; }
	if (!IsLocalController() || !GetWorld() || !FSlateApplication::IsInitialized()) { return; }
	UGameViewportClient* Viewport = GetWorld()->GetGameViewport();
	USMAGFShowcaseSubsystem* Subsystem = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (!Viewport || !Subsystem) { return; }
	CancelHeldInput();
	bRequireFireRelease = true;
	bRequireMoveRelease = true;
	ShowcasePanel = SNew(SSMAGFShowcaseDebugPanel).Controller(this).Subsystem(Subsystem);
	Viewport->AddViewportWidgetContent(ShowcasePanel.ToSharedRef(), 100);
	bShowMouseCursor = true;
	FInputModeGameAndUI Mode;
	Mode.SetWidgetToFocus(ShowcasePanel);
	Mode.SetHideCursorDuringCapture(false);
	Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(Mode);
	FSlateApplication::Get().SetKeyboardFocus(ShowcasePanel);
}

void ASMAGFPlayerController::CloseShowcasePanel()
{
	RemoveShowcasePanel(true);
}

void ASMAGFPlayerController::RemoveShowcasePanel(bool bRestoreInput)
{
	if (!ShowcasePanel.IsValid()) { return; }
	ShowcasePanel->Disconnect();
	if (GetWorld() && GetWorld()->GetGameViewport())
	{
		GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(ShowcasePanel.ToSharedRef());
	}
	ShowcasePanel.Reset();
	CancelHeldInput();
	bRequireFireRelease = true;
	bRequireMoveRelease = true;
	if (bRestoreInput && IsLocalController())
	{
		// Restore the approved Phase 1 GameAndUI mode with its visible aim cursor.
		bShowMouseCursor = true;
		FInputModeGameAndUI Mode;
		Mode.SetHideCursorDuringCapture(false);
		Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		SetInputMode(Mode);
		if (FSlateApplication::IsInitialized()) { FSlateApplication::Get().SetAllUserFocusToGameViewport(); }
	}
}

bool ASMAGFPlayerController::HandleShowcaseKey(FKey Key)
{
	USMAGFShowcaseSubsystem* Subsystem = GetWorld() ? GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>() : nullptr;
	if (!Subsystem) { return false; }
	if (Key == EKeys::PageUp) { Subsystem->PreviousShowcasePreset(); }
	else if (Key == EKeys::PageDown) { Subsystem->NextShowcasePreset(); }
	else if (Key == EKeys::Home) { Subsystem->SetShowcasePreset(ESMAGFShowcasePreset::Baseline); }
	else if (Key == EKeys::End) { Subsystem->SetShowcasePreset(ESMAGFShowcasePreset::FullShowcase); }
	else { return false; }
	return true;
}

bool ASMAGFPlayerController::InputKey(const FInputKeyEventArgs& EventArgs)
{
	if (IsLocalController() && EventArgs.Key == EKeys::F
		&& (EventArgs.Event == IE_Pressed || EventArgs.Event == IE_Repeat))
	{
		const bool bControlDown = FSlateApplication::IsInitialized()
			? FSlateApplication::Get().GetModifierKeys().IsControlDown()
			: IsInputKeyDown(EKeys::LeftControl) || IsInputKeyDown(EKeys::RightControl);
		if (HandleShowcaseToggleKey(EventArgs.Key, bControlDown, EventArgs.Event == IE_Repeat))
		{
			return true;
		}
	}
	return Super::InputKey(EventArgs);
}

bool ASMAGFPlayerController::HandleShowcaseToggleKey(FKey Key, bool bControlDown, bool bIsRepeat)
{
	if (Key != EKeys::F || !bControlDown) { return false; }
	// Both routes consume Ctrl+F. Native pressed/repeat edges need no persistent latch.
	if (!bIsRepeat) { ToggleShowcasePanel(); }
	return true;
}

void ASMAGFPlayerController::PreviousShowcaseLayer() { HandleShowcaseKey(EKeys::PageUp); }
void ASMAGFPlayerController::NextShowcaseLayer() { HandleShowcaseKey(EKeys::PageDown); }
void ASMAGFPlayerController::BaselineShowcaseLayer() { HandleShowcaseKey(EKeys::Home); }
void ASMAGFPlayerController::FullShowcaseLayer() { HandleShowcaseKey(EKeys::End); }
