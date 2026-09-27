#include "Combat/SMAGFHealthComponent.h"

USMAGFHealthComponent::USMAGFHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void USMAGFHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	MaxHealth = FMath::IsFinite(MaxHealth) ? FMath::Max(1.f, MaxHealth) : 100.f;
	CurrentHealth = MaxHealth;
	bDead = false;
}

float USMAGFHealthComponent::ApplyDamage(float Damage)
{
	if (bDead || !FMath::IsFinite(Damage) || Damage <= 0.f)
	{
		return 0.f;
	}
	const float Applied = FMath::Min(Damage, CurrentHealth);
	CurrentHealth = FMath::Max(0.f, CurrentHealth - Applied);
	// Commit the transition before delegates can re-enter the damage path.
	const bool bDied = CurrentHealth <= 0.f;
	bDead = bDied;
	OnHealthChanged.Broadcast(this, CurrentHealth, Applied);
	if (bDied)
	{
		OnDeath.Broadcast(this);
	}
	return Applied;
}
