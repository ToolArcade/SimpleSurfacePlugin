// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "SimpleSurfaceComponent.generated.h"

class UMaterialInterface;
class UMaterialInstanceDynamic;
class UMaterialInstance;
class UTexture2D;
class UMeshComponent;

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

	UPROPERTY(DuplicateTransient)
	TObjectPtr<UMaterialInstanceDynamic> SimpleSurfaceMaterial;

	UPROPERTY(DuplicateTransient)
	TObjectPtr<UMaterialInstance> BaseMaterial;

	/**
	 * Stores soft references to captured materials so they may be restored if the component is deleted, even across Editor sessions.
	 */
	UPROPERTY()
	TArray<FCapturedMaterialSlotsEx> CapturedMaterials;
	
	UFUNCTION()
	void SetParameter_Color(const FColor InColor);

	UFUNCTION()
	void SetParameter_Glow(const float InGlow);

	UFUNCTION()
	void SetParameter_ShininessRoughness(const float InValue);

	UFUNCTION()
	void SetParameter_WaxinessMetalness(const float InValue);

	UFUNCTION()
	void SetParameter_Texture(UTexture* InTexture);

	UFUNCTION()
	void SetParameter_TextureIntensity(float InValue);

	UFUNCTION()
	void SetParameter_TextureScale(float InValue);

public:
	// Sets default values for this component's properties
	USimpleSurfaceComponent(FObjectInitializer const& ObjectInitializer);

	virtual void Activate(bool bReset) override;
	virtual void Deactivate() override;

	virtual void DestroyComponent(bool bPromoteChildren = false) override;

	UPROPERTY(DisplayName = "🖌️ Color", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite,
		Setter = SetParameter_Color,
		meta = (HideAlphaChannel))
	FColor Color = FColor::FromHex("D84DC2");

	UPROPERTY(DisplayName = "☀️ Glow", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite,
		meta=(ClampMin=0.0f, ClampMax=1.0f),
		Setter = SetParameter_Glow)
	float Glow = 0.0f;

	UPROPERTY(DisplayName = "💎 Shininess / Roughness 🍞", Category = "🎨 Simple Surface", EditAnywhere,
		BlueprintReadWrite,
		meta=(ClampMin=0.0f, ClampMax=1.0f), Setter = SetParameter_ShininessRoughness)
	float ShininessRoughness = 0.5f;

	UPROPERTY(DisplayName = "🕯️ Waxiness / Metalness 🔩", Category = "🎨 Simple Surface", EditAnywhere,
		BlueprintReadWrite,
		meta=(ClampMin=0.0f, ClampMax=1.0f), Setter = SetParameter_WaxinessMetalness)
	float WaxinessMetalness = 0.5f;

	/**
	 * The intensity of the object's texture.  Setting this to zero effectively disables the texture.
	 */
	UPROPERTY(DisplayName = "🧱 Texture Intensity", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite,
		meta=(ClampMin=0.0f, ClampMax=1.0f, DisplayPriority = 10, DisplayAfter = Appearance),
		Setter = SetParameter_TextureIntensity)
	float TextureIntensity = 0.1f;

	/**
	 * The scale of the object's texture.
	 */
	UPROPERTY(DisplayName = "🧱 Texture Scale", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite,
		meta=(ClampMin=0.0f, ClampMax=1.0f, DisplayPriority = 20, DisplayAfter = Appearance),
		Setter = SetParameter_TextureScale)
	float TextureScale = 1.0f;

	/**
	 * An optional texture to use as a normal map instead of the built-in texture.
	 */
	UPROPERTY(DisplayName = "🧱 Texture Override", Category = "🎨 Simple Surface", EditAnywhere, BlueprintReadWrite,
		Setter = SetParameter_Texture, meta = (DisplayPriority = 30, DisplayAfter = Appearance))
	TObjectPtr<UTexture> Texture;

protected:
	void InitializeSharedMID();

	void ApplyParametersToMaterial() const;

	void ApplyMaterialToMeshes() const;

	void CaptureMaterials();
	void TryRestoreMaterials();
	void ClearCapturedMaterials();
};
