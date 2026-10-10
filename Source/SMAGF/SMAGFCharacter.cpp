#include "SMAGFCharacter.h"
#include "SMAGFPlayerController.h"
#include "SMAGFGameMode.h"
#include "Components/SceneComponent.h"
#include "Engine/World.h"
#include "Presentation/SMAGFEventPresentationComponent.h"
#include "SMAGF.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Combat/SMAGFWeaponComponent.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"

ASMAGFCharacter::ASMAGFCharacter()
{
	CreateDefaultSubobject<USMAGFEventPresentationComponent>(TEXT("Presentation"));
	PrimaryActorTick.bCanEverTick = false;
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.f);
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->MaxWalkSpeed = 600.f;
	GetCharacterMovement()->MaxAcceleration = 1000.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 1000.f;
	// Walking handles elevation; only the requested movement direction is planar.
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 1400.f;
	CameraBoom->SetRelativeRotation(FRotator(-50.f, 45.f, 0.f));
	CameraBoom->bDoCollisionTest = false;
	CameraBoom->bEnableCameraLag = false;
	CameraBoom->bEnableCameraRotationLag = false;
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));
	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;
	TopDownCameraComponent->FieldOfView = 55.f;
	FlashlightAnchor = CreateDefaultSubobject<USceneComponent>(TEXT("FlashlightAnchor"));
	FlashlightAnchor->SetupAttachment(RootComponent);
	FlashlightAnchor->SetUsingAbsoluteRotation(true);
	FlashlightAnchor->SetRelativeLocation(FVector(0.f, 0.f, BaseEyeHeight));
	FlashlightAnchor->SetCanEverAffectNavigation(false);
	Weapon = CreateDefaultSubobject<USMAGFWeaponComponent>(TEXT("Weapon"));
	Reaction = CreateDefaultSubobject<USMAGFReactionComponent>(TEXT("Reaction"));

	static ConstructorHelpers::FObjectFinder<USkeletalMesh> Manny(TEXT("/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple"));
	static ConstructorHelpers::FClassFinder<UAnimInstance> Animation(TEXT("/Game/Characters/Mannequins/Anims/Unarmed/ABP_Unarmed"));
	GetMesh()->SetSkeletalMesh(Manny.Object);
	GetMesh()->SetAnimInstanceClass(Animation.Class);
	GetMesh()->SetRelativeLocationAndRotation(FVector(0.f, 0.f, -96.f), FRotator(0.f, -90.f, 0.f));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ASMAGFCharacter::BeginPlay()
{
	Super::BeginPlay();
	Reaction->SetVisualComponent(GetMesh());
	if (!GetMesh()->GetSkeletalMeshAsset() || !GetMesh()->GetAnimClass())
	{
		UE_LOG(LogSMAGF, Error, TEXT("SMAGF baseline requires SKM_Manny_Simple and ABP_Unarmed; a required visual reference is missing."));
	}
}

void ASMAGFCharacter::MoveInCameraPlane(const FVector2D& Intent)
{
	if (Intent.ContainsNaN())
	{
		return;
	}
	const FVector2D Clamped = Intent.GetClampedToMaxSize(1.f);
	const auto* Player = Cast<ASMAGFPlayerController>(GetController());
	const FRotator Basis(0.f, Player && Player->IsFirstPersonPerspective() ? Player->GetControlRotation().Yaw : CameraBoom->GetComponentRotation().Yaw, 0.f);
	AddMovementInput(Basis.Vector(), Clamped.X);
	AddMovementInput(FRotationMatrix(Basis).GetUnitAxis(EAxis::Y), Clamped.Y);
}

void ASMAGFCharacter::ApplyAimDirection(const FVector& Direction)
{
	if (!Direction.ContainsNaN() && !Direction.IsNearlyZero())
	{
		SetActorRotation(FRotator(0.f, Direction.Rotation().Yaw, 0.f));
	}
}

void ASMAGFCharacter::InitializeHorrorFlashlight()
{
	if (bFlashlightInitializationAttempted || !GetController()) { return; }
	const auto* Mode = GetWorld()->GetAuthGameMode<ASMAGFGameMode>();
	if (!Mode) { return; }
	bFlashlightInitializationAttempted = true;
	if (!Mode->IsHorrorMode()) { return; }
	const TSubclassOf<AActor> FlashlightClass = Mode->GetHorrorFlashlightClass();
	if (!FlashlightClass) { return; } // Intentionally unbound until the MCP asset pass.
	FActorSpawnParameters Params;
	Params.Owner = this;
	Params.Instigator = this;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	Params.ObjectFlags |= RF_Transient;
	HorrorFlashlight = GetWorld()->SpawnActor<AActor>(FlashlightClass, GetPawnViewLocation(), FRotator::ZeroRotator, Params);
	if (IsValid(HorrorFlashlight))
	{
		if (!HorrorFlashlight->GetRootComponent() || !HorrorFlashlight->AttachToComponent(
			FlashlightAnchor, FAttachmentTransformRules::SnapToTargetIncludingScale))
		{
			HorrorFlashlight->Destroy();
			HorrorFlashlight = nullptr;
		}
	}
}

void ASMAGFCharacter::UpdateFlashlightTransform(const FRotator& ViewRotation)
{
	if (!IsValid(HorrorFlashlight) || ViewRotation.ContainsNaN()) { return; }
	FlashlightAnchor->SetWorldLocationAndRotation(GetPawnViewLocation(), FRotator(ViewRotation.Pitch, ViewRotation.Yaw, 0.f));
}

void ASMAGFCharacter::EndPlay(const EEndPlayReason::Type Reason)
{
	if (IsValid(HorrorFlashlight)) { HorrorFlashlight->Destroy(); }
	HorrorFlashlight = nullptr;
	Super::EndPlay(Reason);
}
