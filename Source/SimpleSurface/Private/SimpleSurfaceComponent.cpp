// Copyright 2025, Jeff Stewart
// Email: object01@gmail.com
// All rights reserved.
//
// This software is provided "as is," without warranty of any kind,
// express or implied, including but not limited to the warranties
// of merchantability, fitness for a particular purpose, and
// noninfringement. In no event shall the author be liable for any
// claim, damages, or other liability, whether in an action of
// contract, tort, or otherwise, arising from, out of, or in
// connection with the software or the use or other dealings in
// the software.

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

#if defined(__FUNCSIG__)  // Microsoft Visual C++ Compiler
	#define FUNC_SIGNATURE __FUNCSIG__
#elif defined(__PRETTY_FUNCTION__)  // GCC or Clang
	#define FUNC_SIGNATURE __PRETTY_FUNCTION__
#else
	#define FUNC_SIGNATURE __func__  // Standard C++ fallback (less detailed)
#endif

DEFINE_LOG_CATEGORY(LogSimpleSurface);

void USimpleSurfaceComponent::DestroyComponent(const bool bPromoteChildren)
{
	TryRestoreMaterials();
	Super::DestroyComponent(bPromoteChildren);
}

void USimpleSurfaceComponent::SetParameter_Color(const FColor& InColor)
{
	this->Color = InColor;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_Glow(const float& InGlow)
{
	this->Glow = InGlow;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_ShininessRoughness(const float& InValue)
{
	this->ShininessRoughness = InValue;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_WaxinessMetalness(const float& InValue)
{
	this->WaxinessMetalness = InValue;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_Texture(UTexture* InTexture)
{
	this->Texture = InTexture;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_TextureIntensity(const float& InValue)
{
	this->TextureIntensity = InValue;
	ApplyParametersToMaterial();
}

void USimpleSurfaceComponent::SetParameter_TextureScale(const float& InValue)
{
	this->TextureScale = InValue;
	ApplyParametersToMaterial();
}

// Sets default values for this component's properties
USimpleSurfaceComponent::USimpleSurfaceComponent(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer),
	CapturedMeshComponentCount(0)
{
	PrimaryComponentTick.bCanEverTick = true;
	bTickInEditor = true;

	bAutoActivate = true;
	bWantsInitializeComponent = true;

	// TODO: Use a soft reference instead?  Unclear whether this tightly binds the plugin to the material somehow...
	static ConstructorHelpers::FObjectFinder<UMaterialInstance> MaterialFinder(
		TEXT("/SimpleSurface/Materials/MI_SimpleSurface.MI_SimpleSurface"));

	if (MaterialFinder.Succeeded() && !BaseMaterial)
	{
		BaseMaterial = MaterialFinder.Object;
	}
}

void USimpleSurfaceComponent::ApplyAll()
{
	if (SimpleSurfaceMaterial)
	{
		ApplyParametersToMaterial();
		ApplyMaterialToMeshes();
	}
}

void USimpleSurfaceComponent::Activate(bool bReset)
{
	UpdateMeshCatalog();
	ApplyAll();
	Super::Activate(bReset);
}

void USimpleSurfaceComponent::Deactivate()
{
	TryRestoreMaterials();
	Super::Deactivate();
}

void USimpleSurfaceComponent::InitializeSharedMID()
{
	UE_LOG(LogSimpleSurface, Verbose, TEXT("Initializing shared MID with outer %s (%p)"), *GetName(), this)

	// When duplicating actors, we must ensure that duplicated SimpleSurfaceComponents get their own instance of the SimpleSurfaceMaterial.
	if (!SimpleSurfaceMaterial || SimpleSurfaceMaterial.GetOuter() != this)
	{
		SimpleSurfaceMaterial = UMaterialInstanceDynamic::Create(BaseMaterial.Get(), this, TEXT("SimpleSurfaceMaterial"));
	}
}

void USimpleSurfaceComponent::ApplyParametersToMaterial() const
{
	check(SimpleSurfaceMaterial.Get())
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

	check(SimpleSurfaceMaterial.Get())

	for (auto MeshComponent : MeshComponents)
	{
		for (auto i = 0; i < MeshComponent->GetNumMaterials(); i++)
		{
			// To avoid spurious edits that will prompt the user to save their file even if they haven't changed anything, only change materials when necessary.
			auto Material = MeshComponent->GetMaterial(i);
			if (Material != SimpleSurfaceMaterial.Get())
			{
				// Ensure undo/redo capture for all components whose materials we're changing.
				MeshComponent->Modify();
			
				MeshComponent->SetMaterial(i, SimpleSurfaceMaterial.Get());
			}
		}
	}
}

ComponentMaterialMap USimpleSurfaceComponent::CreateComponentMaterialMap() const
{
	if (!GetOwner())
	{
		return {};
	}
	
	ComponentMaterialMap ResultMap;
	TArray<TObjectPtr<UMeshComponent>> AllMeshComponents;
	GetOwner()->GetComponents<UMeshComponent>(AllMeshComponents);
	
	for (const auto& MeshComponent : AllMeshComponents)
	{
		TMap<int32, TObjectPtr<UMaterialInterface>> MaterialMap;
		for (int32 i = 0; i < MeshComponent->GetNumMaterials(); i++)
		{
			TObjectPtr<UMaterialInterface> MatInterface = MeshComponent->GetMaterial(i);
			MaterialMap.Add(i, MatInterface);
		}
		ResultMap.Add(MeshComponent, MaterialMap);
	}

	return ResultMap;
}

void USimpleSurfaceComponent::UpdateComponentMaterialMap(ComponentMaterialMap& InOutMap) const
{
	if (!GetOwner())
	{
		return;		
	}

	TArray<UMeshComponent*, TInlineAllocator<32>> CurrentComponentsArray;
	GetOwner()->GetComponents<UMeshComponent>(CurrentComponentsArray);
	TSet<TObjectPtr<UMeshComponent>> CurrentComponentsSet;
	Algo::Transform(CurrentComponentsArray, CurrentComponentsSet, [](UMeshComponent* x) { return TObjectPtr(*x); });

	TSet<TObjectPtr<UMeshComponent>> OldKeys;
	InOutMap.GetKeys(OldKeys);

	auto NewComponents = CurrentComponentsSet.Difference(OldKeys);
	auto MissingComponents = OldKeys.Difference(CurrentComponentsSet);
	auto ExistingComponents = CurrentComponentsSet.Intersect(OldKeys);
	
	// Add new components
	for (const auto& NewComponent : NewComponents)
	{
		auto MaterialsBySlot = TMap<int32, TObjectPtr<UMaterialInterface>>();
		for (int32 i = 0; i < NewComponent->GetNumMaterials(); i++)
		{
			MaterialsBySlot.Add(i, NewComponent->GetMaterial(i));
		}
		InOutMap.Add(NewComponent, MaterialsBySlot);
	}

	// Remove old components
	for (const auto& MissingComponent : MissingComponents)
	{
		InOutMap.Remove(MissingComponent);
	}

	// Update existing components
	for (const auto& ExistingComponent : ExistingComponents)
	{
		auto& MaterialsBySlot = InOutMap[ExistingComponent];
		MaterialsBySlot.Reset();
		for (int32 i = 0; i < ExistingComponent->GetNumMaterials(); i++)
		{
			UMaterialInterface* ExistingMaterial = ExistingComponent->GetMaterial(i);
			if (!ExistingMaterial->IsA(SimpleSurfaceMaterial.GetClass()))
			{
				MaterialsBySlot.Add(i, ExistingMaterial);
			}
		}
	}
}

TArray<int32> USimpleSurfaceComponent::GetIndexPath(USceneComponent& Component)
{
	TArray<int32> IndexPath;
	auto Parent = Component.GetAttachParent();
	while (Parent)
	{
		auto Index = Parent->GetAttachChildren().Find(&Component);
		if (Index == INDEX_NONE)
		{
			break;
		}
		IndexPath.Insert(Index, 0);
		Parent = Parent->GetAttachParent();
	}
	return IndexPath;	
}

void USimpleSurfaceComponent::UpdateMeshCatalog()
{
	if (!GetOwner() || !SimpleSurfaceMaterial)
	{
		return;
	}	
	
	// Update our records of all mesh components' current materials.
	TArray<TObjectPtr<UMeshComponent>> AllMeshComponents;
	GetOwner()->GetComponents<UMeshComponent>(AllMeshComponents);
	CapturedMeshComponentCount = AllMeshComponents.Num();
	for (const auto& MeshComponent : AllMeshComponents)
	{
		if (!MeshComponent.Get())
		{
			CapturedMeshCatalog.Remove(MeshComponent);
			continue;
		}

		if (auto FoundCatalogRecord = CapturedMeshCatalog.Find(MeshComponent))
		{
			FoundCatalogRecord->UpdateRecord(*MeshComponent);			
		}
		else
		{
			CapturedMeshCatalog.Add(MeshComponent, FMeshCatalogRecord(*MeshComponent, { SimpleSurfaceMaterial.GetClass() }));
		}
	}
}

void USimpleSurfaceComponent::TryRestoreMaterials()
{
	if (!GetOwner())
	{
		return;
	}

	TSet<TSoftObjectPtr<UMeshComponent>> MarkedForRemoval;
	
	for (auto& ComponentToCatalogRecordKvp : CapturedMeshCatalog)
	{
		auto const &MeshComponent = ComponentToCatalogRecordKvp.Key;
		auto const &CatalogRecord = ComponentToCatalogRecordKvp.Value;

		// Now restore captured materials.
		if (auto SafeComponent = MeshComponent.Get())
		{
			// Ensure undo/redo capture for all components whose materials we're reverting.
			MeshComponent->Modify();
			
			// Start by clearing all override materials, including SimpleSurface.
			MeshComponent->EmptyOverrideMaterials();

			CatalogRecord.ApplyMaterials(*SafeComponent);
		}
		else
		{
			// No point keeping the record if the MeshComponent no longer exists.
			MarkedForRemoval.Add(MeshComponent);
		}
	}

	for (auto& Component : MarkedForRemoval)
	{
		CapturedMeshCatalog.Remove(Component);
	}
}

void USimpleSurfaceComponent::InitializeComponent()
{
	UE_LOG(LogSimpleSurface, Verbose, TEXT(FUNC_SIGNATURE))

	Super::InitializeComponent();
}

bool USimpleSurfaceComponent::MonitorForChanges() const
{
	if (!GetOwner())
	{
		return false;
	}
	
	bool bChangeOccurred = false;

	TArray<UMeshComponent*, TInlineAllocator<32>> CurrentMeshComponents;
	GetOwner()->GetComponents<UMeshComponent>(CurrentMeshComponents);
	int32 CurrentMeshComponentCount = CurrentMeshComponents.Num();
	
	// Has the number of mesh components changed?
	if (CurrentMeshComponentCount != CapturedMeshComponentCount)
	{
		bChangeOccurred = true;
	}

	// Have any of the components' meshes changed?
	for (auto& ComponentToCatalogRecordKvp : CapturedMeshCatalog)
	{
		auto const &MeshComponent = ComponentToCatalogRecordKvp.Key;
		auto const &CatalogRecord = ComponentToCatalogRecordKvp.Value;

		auto SafeMeshComponent = MeshComponent.Get();
		if (!CatalogRecord.MeshEquals(*SafeMeshComponent))
		{
			bChangeOccurred = true;
			break;
		}
	}

	// Are there any materials in use that aren't SimpleSurface?
	// This indicates that a mesh has changed, and the new mesh has more material slots than the old mesh.
	for (auto Component : CurrentMeshComponents)
	{
		if (Component->GetNumMaterials() == 0)
		{
			continue;
		}

		for (int32 i = 0; i < Component->GetNumMaterials(); i++)
		{
			auto Material = Component->GetMaterial(i);
			if (Material && !Material->IsA(SimpleSurfaceMaterial.GetClass()))
			{
				bChangeOccurred = true;
				break;
			}
		}

		if (bChangeOccurred)
		{
			break;
		}
	}
	
	return bChangeOccurred;
}

void USimpleSurfaceComponent::OnRegister()
{
	InitializeSharedMID();

	if (!GetOwner())
	{
		return;
	}

	// Initialize the mesh catalog.
	UpdateMeshCatalog();

	// Calling ApplyAll() here ensures that all UMeshComponents on this actor that may already be using a SimpleSurfaceMaterial are using *this* component's instance of the SimpleSurfaceMaterial.
	// This is important following an actor duplication; we can't the duplicate's UMeshComponents referencing the original's SimpleSurfaceMaterial. 
	ApplyAll();
	
	Super::OnRegister();
}

void USimpleSurfaceComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ApplyParametersToMaterial();
	
	if (MonitorForChanges())
	{
		UE_LOG(LogSimpleSurface, Verbose, TEXT("%hs: Change in mesh components or materials detected.  Recapturing materials and re-applying surface."), FUNC_SIGNATURE)

		// Re-capture the most up-to-date component->materials maps.
		UpdateMeshCatalog();

		// Re-apply SimpleSurface to all material slots.
		ApplyAll();
	}
}
