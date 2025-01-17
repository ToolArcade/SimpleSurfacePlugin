// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
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
struct FCapturedMaterialSlotsEx
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<int32> ComponentPathAsChildIndexes;

	UPROPERTY()
	TMap<int32, TObjectPtr<UMaterialInterface>> MaterialsBySlot;
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
	void ApplyAll() const;

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

	bool bDirty;

	/**
	 * If enabled, the component will monitor its actor and reapply the surface when components are added, removed, and changed.
	 */
	UPROPERTY(DisplayName = "⏰ Apply to new components and meshes", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite, Setter = SetParameter_PollingEnabled, AdvancedDisplay);

	bool bPollingEnabled = true;

	/**
	 * Initializes the component by setting up internal data structures used to monitor changes to components and materials.
	 */
	virtual void InitializeComponent() override;

	/**
	 * Monitors the actor's components and materials for changes and re-applies SimpleSurface if necessary.
	 */
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	/**
	 * Responds to property changes by marking internal state for update.
	 */
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	virtual void OnRegister() override;

private:
	UPROPERTY(DuplicateTransient)
	TObjectPtr<UMaterialInstanceDynamic> SimpleSurfaceMaterial;

	UPROPERTY(DuplicateTransient)
	TObjectPtr<UMaterialInstance> BaseMaterial;

	/**
	 * Stores soft references to captured materials so they may be restored if the component is deleted, even across Editor sessions.
	 */
	UPROPERTY()
	TArray<FCapturedMaterialSlotsEx> CapturedMaterials;

	int32 CapturedMeshComponentCount;

	/// All mesh components owned by this component's owner.  Preallocated for monitoring mesh components and their materials.  This collection should NOT be copied between or shared between instances.
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer -- Would prefer to use TWeakObjectPtr but it doesn't play nice with GetComponents() 
	TArray<UMeshComponent*> MonitorBuffer_MeshComponents;

	/// Preallocated space for gathering mesh components' materials.  This collection should NOT be copied between or shared between instances.
	// ReSharper disable once CppUE4ProbableMemoryIssuesWithUObjectsInContainer -- Would prefer to use TWeakObjectPtr but it doesn't play nice with GetComponents() 
	TArray<UMaterialInterface*> MonitorBuffer_TempMaterials;
	
	/// Preallocated array used for monitoring materials.  This collection should NOT be copied between or shared between instances.
	TArray<TWeakObjectPtr<UMaterialInterface>> MonitorBuffer_LastUsedMaterials;

	/// Preallocated array used for monitoring materials.  This collection should NOT be copied between or shared between instances.
	TArray<TWeakObjectPtr<UMaterialInterface>> MonitorBuffer_CurrentlyUsedMaterials;
	
	void SetParameter_Color(const FColor& InColor);
	void SetParameter_Glow(const float& InGlow);
	void SetParameter_ShininessRoughness(const float& InValue);
	void SetParameter_WaxinessMetalness(const float& InValue);
	void SetParameter_Texture(UTexture* InTexture);
	void SetParameter_TextureIntensity(const float& InValue);
	void SetParameter_TextureScale(const float& InValue);
	void SetParameter_PollingEnabled(const bool& bEnable);

protected:
	/**
	 * Initializes the UMaterialInstanceDynamic used by this component, ensuring that the instance has this component as its outer.  Does NOT assign the material to any meshes.
	 */
	void InitializeSharedMID();

	void ApplyParametersToMaterial() const;

	void ApplyMaterialToMeshes() const;

	static TArray<int32> GetIndexPath(USceneComponent& Component);
	static void SerializeComponentMaterialMap(ComponentMaterialMap &InMap, TArray<FCapturedMaterialSlotsEx> &OutArray);
	void DeserializeComponentMaterialMap(TArray<FCapturedMaterialSlotsEx> &InArray, ComponentMaterialMap &OutMap) const;

	/**
	 * Updates this component's internal state to capture the actor's current mesh components and their assigned materials,
	 * so they can be restored later if the component is deleted or deactivated.
	 *
	 * This also works across sessions. :)
	 *
	 * @remarks This function captures two data structures:
	 *   1. a transient map of mesh component references and their materials, used to reconcile changes to the actor's components while SimpleSurface is active
	 *   2. a map of mesh component "paths" and their materials, used when duplicating actors and their components  
	 */
	void CaptureMaterials();

	/**
	 * Attempts to restore captured materials to their original state.
	 * If components or referenced materials are no longer valid, they are ignored.
	 */
	void TryRestoreMaterials();

	void ClearCapturedMaterials();

	ComponentMaterialMap CreateComponentMaterialMap() const;
	void UpdateComponentMaterialMap(ComponentMaterialMap &InOutMap) const;
	
	/**
	 * Compares the current state of mesh components and materials to the last known state and returns true if a change
	 * occurred that warrants re-applying SimpleSurface.  Updates the data in the process.
	 */
	bool MonitorForChanges(bool bForceUpdate = false);
};