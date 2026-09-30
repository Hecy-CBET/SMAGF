#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Showcase/SMAGFShowcaseTypes.h"
#include "SMAGFPresentationVisual.generated.h"

/** Bounded presentation ownership; only shells simulate physics. Never deals damage. */
UCLASS(Transient, NotBlueprintable)
class SMAGF_API ASMAGFPresentationVisual : public AActor
{
	GENERATED_BODY()
public:
	ASMAGFPresentationVisual();
	static int32 SpawnContact(UWorld* World, FVector Point, FVector Normal, UPrimitiveComponent* Surface);
	static void TransferSurfaceEvidence(AActor* Source, USceneComponent* Destination);
	static ASMAGFPresentationVisual* Spawn(UWorld* World, FVector Position, ESMAGFShowcaseFeature Feature,
		float Duration, FVector Scale, FLinearColor Color, FVector Velocity = FVector::ZeroVector, float LightIntensity = 0.f, float Radius = 0.f);
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type Reason) override;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SMAGF|Presentation") ESMAGFShowcaseFeature Feature;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="SMAGF|Presentation") bool bPersistent = false;
	void SetPersistent() { bPersistent = true; }
protected:
	virtual void BeginPlay() override;
private:
	UPROPERTY() TObjectPtr<class UStaticMeshComponent> Mesh;
	UPROPERTY() TObjectPtr<class UPointLightComponent> Light;
	UPROPERTY() TObjectPtr<class UMaterialInterface> Glow;
	UPROPERTY() TObjectPtr<class UMaterialInstanceDynamic> MID;
	UPROPERTY() TObjectPtr<class UStaticMesh> ShellMesh;
	UPROPERTY() TObjectPtr<class UPhysicalMaterial> ShellPhysicalMaterial;
	UPROPERTY() TObjectPtr<class UNiagaraSystem> ImpactSystem;
	UPROPERTY() TObjectPtr<class UMaterialInterface> ImpactDecalMaterial;
	UPROPERTY() TObjectPtr<class UNiagaraComponent> Niagara;
	UPROPERTY() TObjectPtr<class UDecalComponent> Decal;
	TWeakObjectPtr<USceneComponent> EvidenceSurface;
	TWeakObjectPtr<USceneComponent> EvidenceVisual;
	FTransform EvidenceVisualLocal;
	bool bAttachedEvidence = false;
	bool bSurfacePresentation = false;
	TWeakObjectPtr<class USMAGFShowcaseSubsystem> Showcase;
	FDelegateHandle Handle;
	FVector Start, InitialScale, Velocity;
	float Duration = 0.f, Age = 0.f, Gain = 1.f;
	float FloorZ = 0.f;
	void SettingsChanged();
};
