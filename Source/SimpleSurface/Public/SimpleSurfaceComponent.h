// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/DynamicMeshComponent.h"
#include "UObject/UObjectGlobals.h"

#include "SimpleSurfaceComponent.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UMaterialInstance;
class UTexture2D;
class UMeshComponent;

DECLARE_LOG_CATEGORY_EXTERN(LogSimpleSurface, Log, All);

using ComponentMaterialMap = TMap<TObjectPtr<UMeshComponent>, TMap<int32, TObjectPtr<UMaterialInterface>>>;

USTRUCT()
struct FMeshCatalogRecord
{
	GENERATED_BODY()

	FMeshCatalogRecord() = default;

	FMeshCatalogRecord(UMeshComponent& Component, const TArray<const TSoftClassPtr<UMaterialInterface>>& Ex)
	{
		ExcludedMaterialClasses = Ex;
		UpdateRecord(Component);
	}

	/**
	 * Updates this record to reflect the specified @see UMeshComponent.
	 */
	void UpdateRecord(UMeshComponent& Component)
	{
		MeshHash = GetMeshHash(&Component);
		IndexPath = GetIndexPath(Component);
		UpdateMaterialsBySlot(Component);
	}

	UPROPERTY()
	TArray<int32> IndexPath;
	
	/**
	 * Returns an array of indexes that represent the path to the component from the root component.
	 */
	static TArray<int32> GetIndexPath(UMeshComponent& MeshComponent)
	{
		TArray<int32> Result;
		USceneComponent* Current = &MeshComponent;
		while (Current)
		{
			if (auto Parent = Cast<USceneComponent>(Current->GetAttachParent()))
			{
				Result.Insert(Parent->GetAttachChildren().Find(Current), 0);
				Current = Parent;
			}
			else
			{
				break;
			}
		}
		
		return Result;
	}

	/**
	 * Uses @see IndexPath to locate a component on the specified actor.  Useful for remapping a record to a new actor, e.g. after duplication. 
	 */
	UMeshComponent* LocateComponent(const AActor &Actor)
	{
		// Locate the component corresponding to IndexPath.
		USceneComponent* Current = Actor.GetRootComponent();
		for (auto Index : IndexPath)
		{
			if (Current)
			{
				auto Children = Current->GetAttachChildren();
				if (Index < Children.Num())
				{
					Current = Children[Index];
				}
				else
				{
					Current = nullptr;
				}
			}
			else
			{
				break;
			}
		}

		return Cast<UMeshComponent>(Current);
	}

	/**
	 * Applies the materials captured by this record to the specified UMeshComponent.
	 */
	void ApplyMaterials(UMeshComponent& MeshComponent) const
	{
		for (int32 i = 0; i < MaterialsBySlot.Num(); ++i)
		{
			auto SoftMaterialPtr = MaterialsBySlot[i];
			if (const auto Material = SoftMaterialPtr.IsValid() ? SoftMaterialPtr.Get() : SoftMaterialPtr.LoadSynchronous())
			{
				MeshComponent.SetMaterial(i, Material);
			}
		}
	}

	/**
	 * Accumulates the materials used by the specified UMeshComponent into this record's MaterialsBySlot.
	 * Skips any materials matching the classes in ExcludedMaterialClasses.
	 */
	void UpdateMaterialsBySlot(const UMeshComponent& MeshComponent)
	{
		// Take care to update the slots one by one, don't just copy the array; because we don't want to capture
		// excluded materials.
		MaterialsBySlot.SetNum(MeshComponent.GetNumMaterials());
		for (auto i = 0; i < MeshComponent.GetNumMaterials(); i++)
		{
			auto Material = MeshComponent.GetMaterial(i);
			if (Material && !ExcludedMaterialClasses.Contains(Material->GetClass()))
			{
				MaterialsBySlot[i] = Material;
			}
		}
	}

	/**
	 * Returns true if the mesh presented by the specified UMeshComponent is the same as the mesh presented by the mesh component that this record was created from.
	 */
	bool MeshEquals(UMeshComponent& Component) const
	{
		return MeshHash == GetMeshHash(&Component);
	}

	bool operator==(const FMeshCatalogRecord& Other) const
	{
		return MeshHash == Other.MeshHash && MaterialsBySlot == Other.MaterialsBySlot;
	}

	/**
	 * A hash of the mesh presented by MeshComponent.  Used to determine if an existing MeshComponent's mesh changed.
	 */
	UPROPERTY()
	uint32 MeshHash;

	/**
	 * A map of material slot indexes to materials.
	 */
	UPROPERTY()
	TArray<TSoftObjectPtr<UMaterialInterface>> MaterialsBySlot;

	UPROPERTY()
	TArray<const TSoftClassPtr<UMaterialInterface>> ExcludedMaterialClasses;

	static uint32 GetMeshHash(UMeshComponent* MeshComponent)
	{
		if (!MeshComponent)
		{
			return 0;
		}
		
		auto HashValue = GetTypeHash(MeshComponent);

		if (UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(MeshComponent))
		{
			HashValue = HashCombine(HashValue, GetTypeHash(StaticMeshComponent->GetStaticMesh()));
		}
		else if (UDynamicMeshComponent* DynamicMeshComponent = Cast<UDynamicMeshComponent>(MeshComponent))
		{
			HashValue = HashCombine(HashValue, GetTypeHash(DynamicMeshComponent->GetDynamicMesh()->GetTriangleCount()));
		}
		else
		{
			HashValue = 0;
		}

		return HashValue;
	}
};
	
/**
 * Quickly apply simple, colorful shading to meshes.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIMPLESURFACE_API USimpleSurfaceComponent : public UActorComponent
{
private:
	GENERATED_BODY()

	ComponentMaterialMap TransientComponentMaterialMap;
	
public:
	USimpleSurfaceComponent(FObjectInitializer const& ObjectInitializer);

	/**
	 * Applies configured properties to the SimpleSurface material and applies the material to all mesh components. 
	 */
	void ApplyAll();

	/**
	 * Handle the component's being turned on.
	 */
	virtual void Activate(bool bReset) override;

	/**
	 * Handle the component's being turned off.
	 */
	virtual void Deactivate() override;

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

	UPROPERTY(DisplayName = "🖌️ Color", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, Setter = SetParameter_Color, meta = (HideAlphaChannel))
	FColor Color = FColor::FromHex("D84DC2");

	UPROPERTY(DisplayName = "☀️ Glow", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0.0f, ClampMax=1.0f), Setter = SetParameter_Glow)
	float Glow = 0.0f;

	UPROPERTY(DisplayName = "💎 Shininess / Roughness 🍞", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0.0f, ClampMax=1.0f), Setter = SetParameter_ShininessRoughness)
	float ShininessRoughness = 0.5f;

	UPROPERTY(DisplayName = "🕯️ Waxiness / Metalness 🔩", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0.0f, ClampMax=1.0f), Setter = SetParameter_WaxinessMetalness)
	float WaxinessMetalness = 0.5f;

	/**
	 * The intensity of the object's texture.  Setting this to zero effectively disables the texture.
	 */
	UPROPERTY(DisplayName = "🧱 Texture Intensity", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0.0f, ClampMax=1.0f, DisplayPriority = 10, DisplayAfter = Appearance), Setter = SetParameter_TextureIntensity)
	float TextureIntensity = 0.1f;

	/**
	 * The scale of the object's texture.
	 */
	UPROPERTY(DisplayName = "🧱 Texture Scale", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, meta=(ClampMin=0.0f, ClampMax=1.0f, DisplayPriority = 20, DisplayAfter = Appearance), Setter = SetParameter_TextureScale)
	float TextureScale = 1.0f;

	/**
	 * An optional texture to use as a normal map instead of the built-in texture.
	 */
	UPROPERTY(DisplayName = "🧱 Texture Override", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, Setter = SetParameter_Texture, meta = (DisplayPriority = 30, DisplayAfter = Appearance))
	TObjectPtr<UTexture> Texture;

	/**
	 * Initializes the component by setting up internal data structures used to monitor changes to components and materials.
	 */
	virtual void InitializeComponent() override;

	/**
	 * Monitors the actor's components and materials for changes and re-applies SimpleSurface if necessary.
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	virtual void OnRegister() override;

private:
	UPROPERTY(DuplicateTransient)
	TObjectPtr<UMaterialInstanceDynamic> SimpleSurfaceMaterial;

	UPROPERTY(DuplicateTransient)
	TObjectPtr<UMaterialInstance> BaseMaterial;

	/**
	 * Keeps a record of materials applied to mesh components, so they can be restored if the component is deleted or deactivated.
	 */
	UPROPERTY()
	TMap<TSoftObjectPtr<UMeshComponent>, FMeshCatalogRecord> CapturedMeshCatalog;
		
	int32 CapturedMeshComponentCount;
	
	void SetParameter_Color(const FColor& InColor);
	void SetParameter_Glow(const float& InGlow);
	void SetParameter_ShininessRoughness(const float& InValue);
	void SetParameter_WaxinessMetalness(const float& InValue);
	void SetParameter_Texture(UTexture* InTexture);
	void SetParameter_TextureIntensity(const float& InValue);
	void SetParameter_TextureScale(const float& InValue);

protected:
	/**
	 * Initializes the UMaterialInstanceDynamic used by this component, ensuring that the instance has this component as its outer.  Does NOT assign the material to any meshes.
	 */
	void InitializeSharedMID();

	void ApplyParametersToMaterial() const;

	/**
	 * Applies the SimpleSurface material to all meshes of the owning actor.
	 */
	void ApplyMaterialToMeshes() const;

	static TArray<int32> GetIndexPath(USceneComponent& Component);

	/**
	 * Updates this component's internal state to capture the actor's current mesh components and their assigned materials,
	 * so they can be restored later if the component is deleted or deactivated.
	 *
	 * This also works across sessions. :)
	 *
	 * @see TryRestoreMaterials
	 * 
	 * @remarks This function captures two data structures:
	 *   1. a transient map of mesh component references and their materials, used to reconcile changes to the actor's components while SimpleSurface is active
	 *   2. a map of mesh component "paths" and their materials, used when duplicating actors and their components  
	 */
	void UpdateMeshCatalog();

	/**
	 * Attempts to restore captured materials to their original state.
	 * If components or referenced materials are no longer valid, they are ignored.
	 */
	void TryRestoreMaterials();

	ComponentMaterialMap CreateComponentMaterialMap() const;
	void UpdateComponentMaterialMap(ComponentMaterialMap &InOutMap) const;
	
	/**
	 * Compares the current state of mesh components and materials to the last known state and returns true if a change
	 * occurred that warrants re-applying SimpleSurface.  Does not update any data if changes are found.
	 */
	bool MonitorForChanges() const;
};