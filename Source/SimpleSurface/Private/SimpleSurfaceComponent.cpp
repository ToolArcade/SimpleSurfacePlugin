// Fill out your copyright notice in the Description page of Project Settings.


#include "SimpleSurfaceComponent.h"

#include "GameFramework/Actor.h"
#include "Components/MeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialInstance.h"
#include "Materials/MaterialInterface.h"

void USimpleSurfaceComponent::DestroyComponent(bool bPromoteChildren)
{
	TryRestoreMaterials();
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
USimpleSurfaceComponent::USimpleSurfaceComponent()
{
	bAutoActivate = true;
	bWantsInitializeComponent = true;

	// TODO: Use a soft reference instead?  Unclear whether this tightly binds the plugin to the material somehow...
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialFinder(TEXT("/SimpleSurface/Materials/MI_DreamsMaterial.MI_DreamsMaterial"));

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

	CaptureMaterials();

	if (SimpleSurfaceMaterial)
	{
		ApplyParametersToMaterial();
		ApplyMaterialToMeshes();
	}

	Super::Activate(bReset);
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

	TArray<UMeshComponent*> MeshComponents;
	GetOwner()->GetComponents<UMeshComponent>(MeshComponents);

	for (TObjectPtr<UMeshComponent> MeshComponent : MeshComponents)
	{
		FCapturedMaterialSlots& CapturedMeshMaterials = CapturedMaterials.FindOrAdd(MeshComponent);
		for (auto i = 0; i < MeshComponent->GetNumMaterials(); i++)
		{
			// To handle cases where an Undo is restoring the component, materials may have already been changed back to this component's material,
			// before the component itself is restored.  So don't attempt to capture the material if it's already the SimpleSurfaceMaterial.
			auto Material = MeshComponent->GetMaterial(i);
			if (Material != SimpleSurfaceMaterial)
			{
				CapturedMeshMaterials.SlotMaterialMap.Add(i, Material);
			}
		}
	}
}

/**
 * Attempts to restore captured materials to their original state.
 * If components or referenced materials are no longer valid, they are ignored.
 */
void USimpleSurfaceComponent::TryRestoreMaterials()
{
	for (auto& CapturedMeshMaterials : CapturedMaterials)
	{
		auto CapturedMaterialSlots = CapturedMeshMaterials.Value;

		if (auto MeshComponent = CapturedMeshMaterials.Key.Get())
		{
			for (auto i = 0; i < MeshComponent->GetNumMaterials(); i++)
			{
				if (CapturedMaterialSlots.SlotMaterialMap.Contains(i))
				{
					if (auto Material = CapturedMaterialSlots.SlotMaterialMap[i].Get())
					{
						MeshComponent->SetMaterial(i, Material);
					}
				}
			}
		}
	}
}

void USimpleSurfaceComponent::ClearOverrideMaterials() const
{
	TArray<UMeshComponent*> MeshComponents;
	GetOwner()->GetComponents<UMeshComponent>(MeshComponents);

	for (auto MeshComponent : MeshComponents)
	{
		MeshComponent->EmptyOverrideMaterials();
	}
}
