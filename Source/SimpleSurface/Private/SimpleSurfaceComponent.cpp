// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleSurfaceComponent.h"

#include "GameFramework/Actor.h"
#include "Components/MeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "FComponentIterator.h"

void USimpleSurfaceComponent::DestroyComponent(bool bPromoteChildren)
{
	TryRestoreMaterials();
	ClearCapturedMaterials();
	Super::DestroyComponent(bPromoteChildren);
}

void USimpleSurfaceComponent::SetParameter_Color(const FColor InColor)
{
	this->Color = InColor;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_Glow(const float InGlow)
{
	this->Glow = InGlow;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_ShininessRoughness(const float InValue)
{
	this->ShininessRoughness = InValue;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_WaxinessMetalness(const float InValue)
{
	this->WaxinessMetalness = InValue;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_Texture(UTexture* InTexture)
{
	this->Texture = InTexture;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_TextureIntensity(const float InValue)
{
	this->TextureIntensity = InValue;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_TextureScale(float InValue)
{
	this->TextureScale = InValue;
	ApplyParametersToMaterial();
}

// Sets default values for this component's properties
USimpleSurfaceComponent::USimpleSurfaceComponent(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bAutoActivate = true;
	bWantsInitializeComponent = true;

	// TODO: Use a soft reference instead?  Unclear whether this tightly binds the plugin to the material somehow...
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialFinder(
		TEXT("/SimpleSurface/Materials/MI_SimpleSurface.MI_SimpleSurface"));

	if (MaterialFinder.Succeeded() && !BaseMaterial)
	{
		BaseMaterial = MaterialFinder.Object;
	}

	if (!SimpleSurfaceMaterial)
	{
		InitializeSharedMID();
	}

	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}

void USimpleSurfaceComponent::Activate(bool bReset)
{
	ensure(SimpleSurfaceMaterial.Get());

	// If materials haven't been captured yet, capture them now, as is the case when the component is first activated.
	// If materials have been captured, it means the component is being reactivated after being deactivated, or is being activated following some other operation like Duplicate, so don't capture them again.
	if (CapturedMaterials.Num() == 0)
	{
		CaptureMaterials();
	}
	
	if (SimpleSurfaceMaterial)
	{
		ApplyParametersToMaterial();
		ApplyMaterialToMeshes();
	}

	Super::Activate(bReset);
}

void USimpleSurfaceComponent::Deactivate()
{
	TryRestoreMaterials();
	Super::Deactivate();
}

void USimpleSurfaceComponent::InitializeSharedMID()
{
	SimpleSurfaceMaterial = UMaterialInstanceDynamic::Create(BaseMaterial.Get(), GetOwner());
}

void USimpleSurfaceComponent::ApplyParametersToMaterial() const
{
	check(SimpleSurfaceMaterial.Get());
	SimpleSurfaceMaterial->SetVectorParameterValue("Color", Color);
	SimpleSurfaceMaterial->SetScalarParameterValue("Glow", Glow);
	SimpleSurfaceMaterial->SetScalarParameterValue("Waxiness / Metalness", WaxinessMetalness);
	SimpleSurfaceMaterial->SetScalarParameterValue("Shininess / Roughness", ShininessRoughness);

	SimpleSurfaceMaterial->SetTextureParameterValue("Texture", Texture.Get());
	SimpleSurfaceMaterial->SetScalarParameterValue("Texture Intensity", TextureIntensity);
	SimpleSurfaceMaterial->SetScalarParameterValue("Texture Scale", TextureScale);
}

void USimpleSurfaceComponent::ApplyMaterialToMeshes() const
{
	if (!GetOwner())
	{
		return;
	}

	TArray<UMeshComponent*> MeshComponents;
	GetOwner()->GetComponents<UMeshComponent>(MeshComponents);

	check(SimpleSurfaceMaterial.Get());

	for (auto MeshComponent : MeshComponents)
	{
		for (auto i = 0; i < MeshComponent->GetNumMaterials(); i++)
		{
			MeshComponent->SetMaterial(i, SimpleSurfaceMaterial.Get());
		}
	}
}

/**
 * Captures the state of all mesh components' overridden materials so they may be restored later.
 * This is useful in cases where the user has overridden a mesh's materials before_applying the SimpleSurfaceComponent: subsequently deleting the
 * component will restore those manual overrides.
 *
 * This also works across sessions. :)
 */
void USimpleSurfaceComponent::CaptureMaterials()
{
	if (!GetOwner())
	{
		return;
	}

	AActor* MyActor = GetOwner();
	FComponentIterator ComponentIterator(MyActor);

	for (const FComponentIterator::FComponentInfo& ComponentInfo : ComponentIterator.GetComponents())
	{
		UActorComponent* Component = ComponentInfo.Component;
		if (auto MeshComponent = Cast<UMeshComponent>(Component))
		{
			FCapturedMaterialSlotsEx CapturedMeshMaterials;
			CapturedMeshMaterials.ComponentPathAsChildIndexes = ComponentInfo.ChildIndices;

			for (auto i = 0; i < MeshComponent->GetNumMaterials(); i++)
			{
				// To handle cases where an Undo is restoring the component, materials may have already been changed back to this component's material,
				// before the component itself is restored.  So don't attempt to capture the material if it's already the SimpleSurfaceMaterial.
				auto Material = MeshComponent->GetMaterial(i);
				if (Material != SimpleSurfaceMaterial)
				{
					CapturedMeshMaterials.MaterialsBySlot.Add(i, Material);
				}
			}

			CapturedMaterials.Add(CapturedMeshMaterials);
		}
	}
}

/**
 * Attempts to restore captured materials to their original state.
 * If components or referenced materials are no longer valid, they are ignored.
 */
void USimpleSurfaceComponent::TryRestoreMaterials()
{
	if (!GetOwner())
	{
		return;
	}

	AActor* MyActor = GetOwner();

	for (const FCapturedMaterialSlotsEx& CapturedMeshMaterials : CapturedMaterials)
	{
		// Acquire the component by traversing the child collections by indicies.
		USceneComponent* Component = MyActor->GetRootComponent();

		for (int32 ChildIndex : CapturedMeshMaterials.ComponentPathAsChildIndexes)
		{
			if (!Component)
			{
				Component = MyActor->GetRootComponent();
			}
			else
			{
				Component = Component->GetChildComponent(ChildIndex);
			}
		}

		if (!Component)
		{
			continue;
		}

		auto MeshComponent = Cast<UMeshComponent>(Component);
		if (!MeshComponent)
		{
			continue;
		}

		// Having identified the mesh component for this entry, restore its materials slot by slot.
		for (const auto& SlotMaterialPair : CapturedMeshMaterials.MaterialsBySlot)
		{
			const int32 Slot = SlotMaterialPair.Key;
			if (const auto Material = SlotMaterialPair.Value.Get())
			{
				MeshComponent->SetMaterial(Slot, Material);
			}
		}
	}
}

void USimpleSurfaceComponent::ClearCapturedMaterials()
{
	CapturedMaterials.Empty();
}
