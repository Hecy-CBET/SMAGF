#include "SMAGFPlayerController.h"
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

ASMAGFPlayerController::ASMAGFPlayerController()
{
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
	CancelHeldInput();
	RemoveOwnedMapping();
	Super::EndPlay(EndPlayReason);
}

void ASMAGFPlayerController::FlushPressedKeys()
{
	Super::FlushPressedKeys();
	CancelHeldInput();
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
	MoveIntent = Value.Get<FVector2D>();
}

void ASMAGFPlayerController::OnMoveReleased()
{
	MoveIntent = FVector2D::ZeroVector;
}

void ASMAGFPlayerController::OnFireStarted()
{
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
	if (DeprojectMousePositionToWorld(RayOrigin, RayDirection) && !RayOrigin.ContainsNaN()
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
