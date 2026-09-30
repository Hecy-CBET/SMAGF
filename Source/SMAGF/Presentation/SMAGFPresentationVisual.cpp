#include "Presentation/SMAGFPresentationVisual.h"
#include "Presentation/SMAGFPermanenceSubsystem.h"
#include "Showcase/SMAGFShowcaseSubsystem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInterface.h"
#include "Engine/StaticMesh.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Presentation/SMAGFSurfaceImpact.h"
#include "Feedback/SMAGFReactionComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Components/DecalComponent.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

ASMAGFPresentationVisual::ASMAGFPresentationVisual()
{
	PrimaryActorTick.bCanEverTick = true;
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Visual"));
	SetRootComponent(Mesh);
	Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Mesh->SetGenerateOverlapEvents(false);
	Mesh->SetCanEverAffectNavigation(false);
	Mesh->SetCastShadow(false);
	Mesh->SetReceivesDecals(false); // Includes disappearance sphere, flashes, debris and shells.
	SetActorEnableCollision(false); SetCanBeDamaged(false);
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Shape(TEXT("/Engine/BasicShapes/Sphere.Sphere"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> Material(TEXT("/Game/Variant_TwinStick/M_Glow.M_Glow"));
	Mesh->SetStaticMesh(Shape.Object); Glow = Material.Object;
	static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder.Cylinder"));
	ShellMesh = Cylinder.Object;
	static ConstructorHelpers::FObjectFinder<UNiagaraSystem> ImpactAsset(TEXT("/Game/SMAGF/VFX/NS_ProjectileImpact_Circle.NS_ProjectileImpact_Circle"));
	static ConstructorHelpers::FObjectFinder<UMaterialInterface> DecalAsset(TEXT("/Game/FreeSurfaceDecals5Pack/Materials/MI_Decal_ScorchMark.MI_Decal_ScorchMark"));
	ImpactSystem=ImpactAsset.Object; ImpactDecalMaterial=DecalAsset.Object;
}

int32 ASMAGFPresentationVisual::SpawnContact(UWorld* World,FVector Point,FVector Normal,UPrimitiveComponent* Surface)
{
	const auto* S=World?World->GetSubsystem<USMAGFShowcaseSubsystem>():nullptr;
	if (!S) { return 0; }
	const auto Settings=S->GetSettings();
	const float Size=FMath::Clamp(Settings.Audiovisual.ImpactScale*S->GetMasterIntensity(),0.f,20.f);
	if (Size<=0.f) { return 0; }
	int32 NiagaraCount = 0;
	auto* P=World->GetSubsystem<USMAGFPermanenceSubsystem>();
	const float Roll=P?P->NextImpactRoll():0.f;
	if (auto* V=Spawn(World,Point+Normal*.2f,ESMAGFShowcaseFeature::ImpactEffect,Settings.Audiovisual.ImpactDuration,FVector::OneVector,FLinearColor::White))
	{
		V->bSurfacePresentation=true; V->Mesh->SetVisibility(false,false); V->SetActorScale3D(FVector::OneVector);
		// The project-owned Niagara emitter uses local-space SpriteFacing = +X.
		V->SetActorRotation(SMAGFSurfaceImpact::Basis(Normal,Roll));
		V->Niagara=NewObject<UNiagaraComponent>(V);
		V->Niagara->SetupAttachment(V->RootComponent); V->Niagara->SetAsset(V->ImpactSystem);
		V->Niagara->SetRelativeScale3D(FVector(Size)); V->Niagara->SetAutoActivate(false); V->Niagara->SetCanEverAffectNavigation(false);
		V->Niagara->RegisterComponent(); V->Niagara->Activate(true);
		NiagaraCount = 1;
	}
	if (auto* V=Spawn(World,Point+Normal*.2f,ESMAGFShowcaseFeature::KeepImpactEvidence,Settings.Permanence.ImpactEvidenceLifetime,FVector::OneVector,FLinearColor::White))
	{
		V->bSurfacePresentation=true; V->Mesh->SetVisibility(false,false); V->SetActorScale3D(FVector::OneVector);
		// Engine decal projection depth is local X; local -X points into the surface.
		V->SetActorRotation(SMAGFSurfaceImpact::Basis(Normal,Roll));
		V->Decal=NewObject<UDecalComponent>(V); V->Decal->SetupAttachment(V->RootComponent);
		V->Decal->SetDecalMaterial(V->ImpactDecalMaterial); V->Decal->DecalSize=FVector(2,24,24)*Size;
		V->Decal->RegisterComponent();
		if (IsValid(Surface) && Surface->Mobility == EComponentMobility::Movable)
		{
			V->Decal->AttachToComponent(Surface, FAttachmentTransformRules::KeepWorldTransform);
			V->bAttachedEvidence = true;
			V->EvidenceSurface = Surface;
			V->EvidenceVisual = Surface;
			// Collision proxies may have a separate visual-only reaction mesh.
			// Keep the actual hit component as parent, while tracking that visual's motion.
			if (auto* Reaction = Surface->GetOwner()->FindComponentByClass<USMAGFReactionComponent>())
			{
				if (auto* VisualSurface = Reaction->GetVisualComponent()) { V->EvidenceVisual = VisualSurface; }
			}
			V->EvidenceVisualLocal = V->Decal->GetComponentTransform().GetRelativeTransform(V->EvidenceVisual->GetComponentTransform());
			V->SetTickGroup(TG_PostUpdateWork);
		}
		if (P && P->Retain(V,ESMAGFShowcaseFeature::KeepImpactEvidence)) { V->SetPersistent(); }
		else { V->Destroy(); }
	}
	return NiagaraCount;
}

void ASMAGFPresentationVisual::TransferSurfaceEvidence(AActor* Source, USceneComponent* Destination)
{
	if (!IsValid(Source) || !IsValid(Destination)) { return; }
	for (TActorIterator<ASMAGFPresentationVisual> It(Source->GetWorld()); It; ++It)
	{
		auto* Evidence = *It;
		if (!Evidence->Decal || !Evidence->EvidenceSurface.IsValid() || Evidence->EvidenceSurface->GetOwner() != Source) { continue; }
		if (Evidence->EvidenceVisual.IsValid())
		{
			Evidence->Decal->SetWorldTransform(Evidence->EvidenceVisualLocal * Evidence->EvidenceVisual->GetComponentTransform());
		}
		Evidence->Decal->AttachToComponent(Destination, FAttachmentTransformRules::KeepWorldTransform);
		Evidence->EvidenceSurface = Destination;
		Evidence->EvidenceVisual = Destination;
		Evidence->EvidenceVisualLocal = Evidence->Decal->GetComponentTransform().GetRelativeTransform(Destination->GetComponentTransform());
		// Same world-owned actor, timer and permanence entry; no Retain/SetLifeSpan call.
	}
}
ASMAGFPresentationVisual* ASMAGFPresentationVisual::Spawn(UWorld* World, FVector Position, ESMAGFShowcaseFeature Kind,
	float Lifetime, FVector Scale, FLinearColor Color, FVector InVelocity, float Brightness, float Radius)
{
	const auto* S = World ? World->GetSubsystem<USMAGFShowcaseSubsystem>() : nullptr;
	if (!S || !S->IsFeatureEffectivelyEnabled(Kind) || Lifetime <= 0.f || S->GetMasterIntensity() <= 0.f) { return nullptr; }
	// Hard cap for transient bursts, independent of user persistence budgets.
	int32 Count = 0; ASMAGFPresentationVisual* Oldest = nullptr;
	for (TActorIterator<ASMAGFPresentationVisual> It(World); It; ++It)
	{
		if (!It->bPersistent) { ++Count; if (!Oldest || It->GetGameTimeSinceCreation() > Oldest->GetGameTimeSinceCreation()) { Oldest = *It; } }
	}
	if (Count >= 256 && Oldest) { Oldest->Destroy(); }
	const FTransform Transform(FRotator::ZeroRotator, Position);
	auto* V = World->SpawnActorDeferred<ASMAGFPresentationVisual>(StaticClass(), Transform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (!V) { return nullptr; }
	V->Feature = Kind; V->Duration = Lifetime; V->Start = Position; V->Velocity = InVelocity;
	FHitResult GroundHit;
	V->FloorZ = World->LineTraceSingleByObjectType(GroundHit, Position, Position-FVector(0,0,2000),
		FCollisionObjectQueryParams(ECC_WorldStatic)) ? GroundHit.ImpactPoint.Z+2.f : Position.Z-50.f;
	V->Gain = S->GetMasterIntensity(); V->InitialScale = Scale * V->Gain;
	if (Kind == ESMAGFShowcaseFeature::ShellEjection)
	{
		V->Mesh->SetStaticMesh(V->ShellMesh);
		// Cylinder length is local Z; original squeezed sphere length was local X.
		V->InitialScale = FVector(Scale.Y,Scale.Z,Scale.X) * (3.5f * V->Gain);
	}
	V->Mesh->SetRelativeScale3D(V->InitialScale);
	V->MID = UMaterialInstanceDynamic::Create(V->Glow, V);
	if (V->MID) { V->MID->SetVectorParameterValue(TEXT("Emissive Color"),Color); V->MID->SetScalarParameterValue(TEXT("Emissive Strength"), 6.f * V->Gain); V->Mesh->SetMaterial(0,V->MID); }
	if (Brightness > 0.f && Radius > 0.f)
	{
		V->Light = NewObject<UPointLightComponent>(V);
		V->Light->SetupAttachment(V->Mesh); V->Light->SetIntensity(Brightness * V->Gain);
		V->Light->SetAttenuationRadius(Radius); V->Light->SetLightColor(Color); V->Light->SetCastShadows(false); V->Light->RegisterComponent();
	}
	V->FinishSpawning(Transform); V->SetLifeSpan(Lifetime);
	if (Kind == ESMAGFShowcaseFeature::ShellEjection)
	{
		V->ShellPhysicalMaterial=NewObject<UPhysicalMaterial>(V);
		V->ShellPhysicalMaterial->Restitution=.3f;
		V->ShellPhysicalMaterial->Friction=.6f;
		V->Mesh->SetPhysMaterialOverride(V->ShellPhysicalMaterial);
		V->SetActorEnableCollision(true);
		V->Mesh->SetCollisionObjectType(ECC_WorldDynamic);
		V->Mesh->SetCollisionResponseToAllChannels(ECR_Ignore);
		V->Mesh->SetCollisionResponseToChannel(ECC_WorldStatic,ECR_Block);
		V->Mesh->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
		V->Mesh->SetEnableGravity(true);
		V->Mesh->SetLinearDamping(.3f); V->Mesh->SetAngularDamping(.8f);
		V->Mesh->SetSimulatePhysics(true);
		V->Mesh->SetPhysicsLinearVelocity(InVelocity + FVector(0,0,100));
		V->Mesh->SetPhysicsAngularVelocityInDegrees(FVector(210,330,150));
	}
	return V;
}
void ASMAGFPresentationVisual::BeginPlay()
{
	Super::BeginPlay(); Showcase = GetWorld()->GetSubsystem<USMAGFShowcaseSubsystem>();
	if (Showcase.IsValid()) { Handle = Showcase->OnSettingsChangedNative.AddUObject(this,&ThisClass::SettingsChanged); }
}
void ASMAGFPresentationVisual::SettingsChanged()
{
	if (!Showcase.IsValid() || !Showcase->IsFeatureEffectivelyEnabled(Feature) || Showcase->GetMasterIntensity() <= 0.f)
	{ SetActorHiddenInGame(true); if (Light) { Light->SetVisibility(false); } Destroy(); }
}
void ASMAGFPresentationVisual::Tick(float Dt)
{
	Super::Tick(Dt); Age += Dt;
	if (bSurfacePresentation)
	{
		if (bAttachedEvidence)
		{
			if (!EvidenceSurface.IsValid() || !EvidenceVisual.IsValid()) { Destroy(); return; }
			Decal->SetWorldTransform(EvidenceVisualLocal * EvidenceVisual->GetComponentTransform());
		}
		return;
	}
	const bool bShell = Feature == ESMAGFShowcaseFeature::ShellEjection;
	if (bShell) { return; } // Chaos owns the physical shell transform.
	const bool bDebris = Feature == ESMAGFShowcaseFeature::KeepDebris;
	if (bDebris)
	{
		const float T = FMath::Min(Age, .45f);
		FVector Position = Start + Velocity * T;
		Position.Z = FMath::Lerp(Start.Z, double(FloorZ), double(T/.45f)) + FMath::Sin(T/.45f*PI)*15.f;
		SetActorLocation(Position);
		SetActorRotation(FRotator(T*260.f,T*170.f,T*310.f));
	}
	else if (Feature == ESMAGFShowcaseFeature::KeepAftermathSmoke)
	{
		SetActorLocation(Start + FVector(0,0,Age*8.f));
		Mesh->SetRelativeScale3D(InitialScale * (1.f + Age*.25f));
		if (MID) { MID->SetScalarParameterValue(TEXT("Emissive Strength"), .08f); }
	}
	else if (!bPersistent)
	{
		const float Alpha = FMath::Clamp(Age / Duration,0.f,1.f);
		SetActorLocation(Start + Velocity * Age);
		Mesh->SetRelativeScale3D(InitialScale * (Feature == ESMAGFShowcaseFeature::ExplosionEffect ? (1.f+2.f*Alpha) : (1.f-Alpha)));
		if (MID) { MID->SetScalarParameterValue(TEXT("Emissive Strength"),6.f*Gain*(1.f-Alpha)); }
		if (Light) { Light->SetIntensity(Light->Intensity * FMath::Max(0.f,1.f-Dt/FMath::Max(Duration-Age,Dt))); }
	}
}
void ASMAGFPresentationVisual::EndPlay(const EEndPlayReason::Type Reason)
{
	if (Showcase.IsValid()) { Showcase->OnSettingsChangedNative.Remove(Handle); }
	Super::EndPlay(Reason);
}
