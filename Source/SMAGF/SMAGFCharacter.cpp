#include "SMAGFCharacter.h"
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
	const FRotator Basis(0.f, CameraBoom->GetComponentRotation().Yaw, 0.f);
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
