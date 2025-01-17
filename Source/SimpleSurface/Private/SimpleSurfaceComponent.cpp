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

DEFINE_LOG_CATEGORY(LogSimpleSurface);

void USimpleSurfaceComponent::DestroyComponent(const bool bPromoteChildren)
{
	TryRestoreMaterials();
	ClearCapturedMaterials();
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

void USimpleSurfaceComponent::SetParameter_PollingEnabled(const bool& bEnabled)
{
	PrimaryComponentTick.bCanEverTick = bEnabled;
	PrimaryComponentTick.bStartWithTickEnabled = bEnabled;
	PrimaryComponentTick.bTickEvenWhenPaused = bEnabled;
}

// Sets default values for this component's properties
USimpleSurfaceComponent::USimpleSurfaceComponent(FObjectInitializer const& ObjectInitializer)
	: Super(ObjectInitializer),
	MonitorBuffer_MeshComponents({}),
	MonitorBuffer_TempMaterials({}),
	MonitorBuffer_LastUsedMaterials({}),
	MonitorBuffer_CurrentlyUsedMaterials({})
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

void USimpleSurfaceComponent::ApplyAll() const
{
	if (SimpleSurfaceMaterial)
	{
		ApplyParametersToMaterial();
		ApplyMaterialToMeshes();
	}
}

void USimpleSurfaceComponent::Activate(bool bReset)
{
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
	UE_LOG(LogSimpleSurface, Verbose, TEXT("Initializing shared MID with outer %s (%p)"), *GetName(), this);

	// When duplicating actors, we must ensure that duplicated SimpleSurfaceComponents get their own instance of the SimpleSurfaceMaterial.
	if (!SimpleSurfaceMaterial || SimpleSurfaceMaterial.GetOuter() != this)
	{
		SimpleSurfaceMaterial = UMaterialInstanceDynamic::Create(BaseMaterial.Get(), this, TEXT("SimpleSurfaceMaterial"));
	}
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

ComponentMaterialMap USimpleSurfaceComponent::CreateComponentMaterialMap() const
{
	if (!GetOwner())
	{
		return {};
	}
	
	ComponentMaterialMap ResultMap;
	TArray<TObjectPtr<UMeshComponent>> AllMeshComponents;
	GetOwner()->GetComponents<UMeshComponent>(AllMeshComponents);
	
	for (auto MeshComponent : AllMeshComponents)
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
	for (auto NewComponent : NewComponents)
	{
		auto MaterialsBySlot = TMap<int32, TObjectPtr<UMaterialInterface>>();
		for (int32 i = 0; i < NewComponent->GetNumMaterials(); i++)
		{
			MaterialsBySlot.Add(i, NewComponent->GetMaterial(i));
		}
		InOutMap.Add(NewComponent, MaterialsBySlot);
	}

	// Remove old components
	for (auto MissingComponent : MissingComponents)
	{
		InOutMap.Remove(MissingComponent);
	}

	// Update existing components
	for (auto ExistingComponent : ExistingComponents)
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

void USimpleSurfaceComponent::SerializeComponentMaterialMap(ComponentMaterialMap &InMap, TArray<FCapturedMaterialSlotsEx> &OutArray)
{
	OutArray.Reset();
	for (auto& Pair : InMap)
	{
		auto& MeshComponent = *Pair.Key.Get();
		FCapturedMaterialSlotsEx CapturedMeshMaterials;
		CapturedMeshMaterials.ComponentPathAsChildIndexes = GetIndexPath(MeshComponent);
		CapturedMeshMaterials.MaterialsBySlot = Pair.Value;
		OutArray.Add(CapturedMeshMaterials);
	}
}

void USimpleSurfaceComponent::DeserializeComponentMaterialMap(TArray<FCapturedMaterialSlotsEx>& InArray,
	ComponentMaterialMap& OutMap) const
{
	for (auto CapturedMeshMaterials : InArray)
	{
		auto Component = GetOwner()->GetRootComponent();
		for (auto ChildIndex : CapturedMeshMaterials.ComponentPathAsChildIndexes)
		{
			if (!Component)
			{
				Component = GetOwner()->GetRootComponent();
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

		auto& MaterialsBySlot = OutMap.FindOrAdd(MeshComponent);
		MaterialsBySlot = CapturedMeshMaterials.MaterialsBySlot;
	}
}

void USimpleSurfaceComponent::CaptureMaterials()
{
	if (!GetOwner())
	{
		return;
	}

	UpdateComponentMaterialMap(TransientComponentMaterialMap);
	SerializeComponentMaterialMap(TransientComponentMaterialMap, CapturedMaterials);
}

void USimpleSurfaceComponent::TryRestoreMaterials()
{
	if (!GetOwner())
	{
		return;
	}

	for (auto& ComponentToMaterialsKvp : TransientComponentMaterialMap)
	{
		auto const &MeshComponent = ComponentToMaterialsKvp.Key;
		auto const &MaterialSlots = ComponentToMaterialsKvp.Value;

		if (MeshComponent)
		{
			for (auto& SlotToMaterialKvp : MaterialSlots)
			{
				MeshComponent->SetMaterial(SlotToMaterialKvp.Key, SlotToMaterialKvp.Value.Get());
			}
		}
	}
}

void USimpleSurfaceComponent::ClearCapturedMaterials()
{
	CapturedMaterials.Empty();
}

void USimpleSurfaceComponent::InitializeComponent()
{
	UE_LOG(LogSimpleSurface, Verbose, TEXT(__FUNCSIG__));
	
	Super::InitializeComponent();
}

bool USimpleSurfaceComponent::MonitorForChanges(bool bForceUpdate)
{
	if (!GetOwner())
	{
		return false;
	}
	
	bool bChangeOccurred = false;

	TArray<UMeshComponent*, TInlineAllocator<32>> TempComponents;
	GetOwner()->GetComponents<UMeshComponent>(TempComponents);
	int32 CurrentMeshComponentCount = TempComponents.Num();
	
	// Has the component list or any meshes changed?
	if (CurrentMeshComponentCount != CapturedMeshComponentCount)
	{
		// If so, re-capture the mesh components so they're fast to query.
		GetOwner()->GetComponents<UMeshComponent>(MonitorBuffer_MeshComponents);

		CapturedMeshComponentCount = CurrentMeshComponentCount;
		bChangeOccurred = true;
	}

	// Are there any materials in use that aren't SimpleSurface?
	for (auto Component : TempComponents)
	{
		if (!Component->HasOverrideMaterials())
		{
			continue;
		}

		for (int32 i = 0; i < Component->GetNumMaterials(); i++)
		{
			if (!Component->GetMaterial(i)->IsA(SimpleSurfaceMaterial.GetClass()))
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

void USimpleSurfaceComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	UE_LOG(LogSimpleSurface, Verbose, TEXT(__FUNCSIG__));
	bDirty = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);
}


void USimpleSurfaceComponent::OnRegister()
{
	InitializeSharedMID();

	if (!GetOwner())
	{
		return;
	}

	if (TransientComponentMaterialMap.Num() == 0)
	{
		if (CapturedMaterials.Num() > 0)
		{
			// If we have captured materials but no transient map, we can assume this follows a duplication, and we rebuild the transient map to support subsequent monitoring.
			DeserializeComponentMaterialMap(CapturedMaterials, TransientComponentMaterialMap);
		}
		else if (CapturedMaterials.Num() == 0)
		{
			// Otherwise we create the transient map from scratch, and let it be updated normally from now on.
			TransientComponentMaterialMap = CreateComponentMaterialMap();
		}
	}

	// Initialize/reset the buffers for subsequent monitoring.
	MonitorForChanges(true);
	
	// Make sure a serialization of the map is ready in case it's needed soon.
	SerializeComponentMaterialMap(TransientComponentMaterialMap, CapturedMaterials);

	// Calling ApplyAll() here ensures that all UMeshComponents on this actor that may already be using a SimpleSurfaceMaterial are using *this* component's instance of the SimpleSurfaceMaterial.
	// This is important following an actor duplication; we can't the duplicate's UMeshComponents referencing the original's SimpleSurfaceMaterial. 
	ApplyAll();
	
	Super::OnRegister();
}

void USimpleSurfaceComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                            FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bDirty)
	{
		UE_LOG(LogSimpleSurface, Verbose, TEXT("%hs: responding to dirty flag by applying parameters to materials."), __FUNCSIG__);
		ApplyParametersToMaterial();
		bDirty = false;
	}
	
	if (MonitorForChanges())
	{
		UE_LOG(LogSimpleSurface, Verbose, TEXT("%hs: Change in mesh components or materials detected.  Recapturing materials and re-applying surface."), __FUNCSIG__)

		// Re-capture the most up-to-date meshcomponent->materials maps.
		CaptureMaterials();

		// Re-apply SimpleSurface to all material slots.
		ApplyAll();
	}
}
