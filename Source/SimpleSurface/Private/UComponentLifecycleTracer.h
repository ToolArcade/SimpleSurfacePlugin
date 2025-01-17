#pragma once
#include "Components/ActorComponent.h"
#include "Engine/EngineTypes.h"
#include "UObject/ObjectSaveContext.h"
#include "CoreMinimal.h"

#include "UComponentLifecycleTracer.generated.h"

DEFINE_LOG_CATEGORY_STATIC(LogComponentLifecycle, Log, All)

// Define a LOG_FUNCTION_AND_PARAMS macro that works like LOG_FUNCTION, but allows the caller to specify n optional arguments whose values are appended to the log.
#define LOG_FUNCTION_AND_PARAMS(FormatString, ...) UE_LOG(LogComponentLifecycle, Display, TEXT("[%p (%s)] %hs(%s)"), this, GetOwner() ? *GetOwner()->GetActorNameOrLabel() : TEXT("NoOwner"), __FUNCTION__, *FString::Printf(TEXT(FormatString), __VA_ARGS__))
#define LOG_FUNCTION_AND_NOTE(OptionalString, ...) UE_LOG(LogComponentLifecycle, Display, TEXT("[%p (%s)] %hs (Note: %s)"), this, GetOwner() ? *GetOwner()->GetActorNameOrLabel() : TEXT("NoOwner"), __FUNCTION__, *FString::Printf(TEXT(OptionalString), __VA_ARGS__))
#define LOG_FUNCTION UE_LOG(LogComponentLifecycle, Display, TEXT("[%p (%s)] %hs"), this, GetOwner() ? *GetOwner()->GetActorNameOrLabel() : TEXT("NoOwner"), __FUNCTION__)

#define LOG_FUNCSIG UE_LOG(LogComponentLifecycle, Display, TEXT("[%p (%s)] %hs"), this, GetOwner() ? *GetOwner()->GetActorNameOrLabel() : TEXT("NoOwner"), __FUNCSIG__)

/**
 * A troubleshooting tool used to inspect the lifecycle of a component by tracing its virtual function calls.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class SIMPLESURFACE_API UComponentLifecycleTracer : public UActorComponent
{
	GENERATED_BODY()

public:
	UComponentLifecycleTracer(FObjectInitializer const& ObjectInitializer)
	{
		LOG_FUNCTION;

		bAutoActivate = false;
		bWantsInitializeComponent = true;
	}

	virtual void PostEditImport() override
	{
		LOG_FUNCTION;
		Super::PostEditImport();
	}

	virtual void ExportCustomProperties(FOutputDevice& Out, uint32 Indent) override
	{
		LOG_FUNCTION;
		Super::ExportCustomProperties(Out, Indent);
	}

	virtual void ImportCustomProperties(const TCHAR* SourceText, FFeedbackContext* Warn) override
	{
		LOG_FUNCTION;
		Super::ImportCustomProperties(SourceText, Warn);
	}

	virtual void FinishDestroy() override
	{
		LOG_FUNCTION;
		Super::FinishDestroy();
	}

	virtual void InitializeComponent() override
	{
		LOG_FUNCTION;
		Super::InitializeComponent();
	}

	virtual void UninitializeComponent() override
	{
		LOG_FUNCTION;
		Super::UninitializeComponent();
	}

	virtual void BeginPlay() override
	{
		LOG_FUNCTION;
		Super::BeginPlay();
	}

	virtual bool Modify(bool bAlwaysMarkDirty = true) override
	{
		LOG_FUNCTION_AND_PARAMS("bAlwaysMarkDirty = %s", bAlwaysMarkDirty ? TEXT("true") : TEXT("false"));
		return Super::Modify(bAlwaysMarkDirty);
	}

	virtual void OnComponentCreated() override
	{
		LOG_FUNCTION;
		Super::OnComponentCreated();
	}

	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override
	{
		LOG_FUNCTION_AND_PARAMS("bDestroyingHierarchy = %s", bDestroyingHierarchy ? TEXT("true") : TEXT("false"));
		Super::OnComponentDestroyed(bDestroyingHierarchy);
	}

	virtual void OnRegister() override
	{
		LOG_FUNCTION_AND_NOTE("Owner is %p", GetOwner());
		Super::OnRegister();
	}

	virtual void OnUnregister() override
	{
		LOG_FUNCTION;
		Super::OnUnregister();
	}

	virtual void PostApplyToComponent() override
	{
		LOG_FUNCTION;
		Super::PostApplyToComponent();
	}

	virtual void PostDuplicate(bool bDuplicateForPIE) override
	{
		LOG_FUNCTION;
		Super::PostDuplicate(bDuplicateForPIE);
	}

	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override
	{
		LOG_FUNCTION;
		Super::PostDuplicate(DuplicateMode);
	}

	virtual void PostEditChangeChainProperty(FPropertyChangedChainEvent& PropertyChangedEvent) override
	{
		LOG_FUNCTION;
		Super::PostEditChangeChainProperty(PropertyChangedEvent);
	}

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override
	{
		LOG_FUNCTION;
		Super::PostEditChangeProperty(PropertyChangedEvent);
	}

	virtual void PostInitProperties() override
	{
		LOG_FUNCTION;
		Super::PostInitProperties();
	}

	virtual void PostLoad() override
	{
		LOG_FUNCTION;
		Super::PostLoad();
	}

	virtual void PostLoadSubobjects(FObjectInstancingGraph* OuterInstanceGraph) override
	{
		LOG_FUNCTION;
		Super::PostLoadSubobjects(OuterInstanceGraph);
	}

	virtual void PostReinitProperties() override
	{
		LOG_FUNCTION;
		Super::PostReinitProperties();
	}

	virtual void SetActive(bool bNewActive, bool bReset = false) override
	{
		LOG_FUNCTION_AND_PARAMS("bNewActive = %s, bReset = %s", bNewActive ? TEXT("true") : TEXT("false"),
		                        bReset ? TEXT("true") : TEXT("false"));
		Super::SetActive(bNewActive, bReset);
	}

	virtual void PreDuplicate(FObjectDuplicationParameters& DupParams) override
	{
		LOG_FUNCTION;
		Super::PreDuplicate(DupParams);
	}

	virtual void PreEditChange(FProperty* PropertyThatWillChange) override
	{
		LOG_FUNCTION;
		Super::PreEditChange(PropertyThatWillChange);
	}

	virtual void PostTransacted(const FTransactionObjectEvent& TransactionEvent) override
	{
		LOG_FUNCTION;
		Super::PostTransacted(TransactionEvent);
	}

	virtual void Serialize(FArchive& Ar) override
	{
		LOG_FUNCTION;
		try
		{
		    Super::Serialize(Ar);
		}
		catch (...)
		{
		    UE_LOG(LogComponentLifecycle, Error, TEXT("Caught an exception during serialization"));
		}
	}
	
	virtual void Serialize(FStructuredArchive::FRecord Record) override
	{
		LOG_FUNCTION;
		try
		{
			Super::Serialize(Record);
		}
		catch (...)
		{
			UE_LOG(LogComponentLifecycle, Error, TEXT("Caught an exception during serialization"));
		}
	}

	virtual void Activate(bool bReset = false) override
	{
		LOG_FUNCTION_AND_PARAMS("bReset = %s", bReset ? TEXT("true") : TEXT("false"));
		Super::Activate(bReset);
	}

	virtual void BeginDestroy() override
	{
		LOG_FUNCTION;
		Super::BeginDestroy();
	}

	virtual void BeginReplication() override
	{
		LOG_FUNCTION;
		Super::BeginReplication();
	}

	virtual void Deactivate() override
	{
		LOG_FUNCTION;
		Super::Deactivate();
	}

	virtual void DeferredRegister(UClass* UClassStaticClass, const TCHAR* PackageName, const TCHAR* Name) override
	{
		LOG_FUNCTION;
		Super::DeferredRegister(UClassStaticClass, PackageName, Name);
	}

	virtual void DestroyComponent(bool bPromoteChildren = false) override
	{
		LOG_FUNCTION_AND_PARAMS("bPromoteChildren = %s", bPromoteChildren ? TEXT("true") : TEXT("false"));
		Super::DestroyComponent(bPromoteChildren);
	}

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override
	{
		LOG_FUNCTION_AND_PARAMS("EndPlayReason = %s", *UEnum::GetValueAsString(EndPlayReason));
		Super::EndPlay(EndPlayReason);
	}

	virtual void EndReplication() override
	{
		LOG_FUNCTION;
		Super::EndReplication();
	}

	virtual void LoadedFromAnotherClass(const FName& OldClassName) override
	{
		LOG_FUNCTION_AND_PARAMS("OldClassName = %s", *OldClassName.ToString());
		Super::LoadedFromAnotherClass(OldClassName);
	}

	virtual void OnActorEnableCollisionChanged() override
	{
		LOG_FUNCTION;
		Super::OnActorEnableCollisionChanged();
	}

	virtual void OnActorVisibilityChanged() override
	{
		LOG_FUNCTION;
		Super::OnActorVisibilityChanged();
	}

	virtual void OnClusterMarkedAsPendingKill() override
	{
		LOG_FUNCTION;
		Super::OnClusterMarkedAsPendingKill();
	}

	virtual void OnCreatePhysicsState() override
	{
		LOG_FUNCTION;
		Super::OnCreatePhysicsState();
	}

	virtual void OnDestroyPhysicsState() override
	{
		LOG_FUNCTION;
		Super::OnDestroyPhysicsState();
	}

	virtual void PostCDOCompiled(const FPostCDOCompiledContext& Context) override
	{
		LOG_FUNCTION;
		Super::PostCDOCompiled(Context);
	}

	virtual void PostCDOContruct() override
	{
		LOG_FUNCTION;
		Super::PostCDOContruct();
	}

	virtual void PostEditUndo() override
	{
		LOG_FUNCTION;
		Super::PostEditUndo();
	}

	virtual void PostEditUndo(TSharedPtr<ITransactionObjectAnnotation> TransactionAnnotation) override
	{
		LOG_FUNCTION;
	}

	virtual void PostLinkerChange() override
	{
		LOG_FUNCTION;
		Super::PostLinkerChange();
	}

	virtual void PostReloadConfig(FProperty* PropertyThatWasLoaded) override
	{
		LOG_FUNCTION;
		Super::PostReloadConfig(PropertyThatWasLoaded);
	}

	virtual void PostRename(UObject* OldOuter, const FName OldName) override
	{
		LOG_FUNCTION_AND_PARAMS("OldOuter = %p, OldName = %s", OldOuter, *OldName.ToString());
		Super::PostRename(OldOuter, OldName);
	}

	virtual void PostSaveRoot(FObjectPostSaveRootContext ObjectSaveContext) override
	{
		LOG_FUNCTION;
		Super::PostSaveRoot(ObjectSaveContext);
	}

	virtual void PreEditUndo() override
	{
		LOG_FUNCTION;
		Super::PreEditUndo();
	}

	virtual void PreReplication(IRepChangedPropertyTracker& ChangedPropertyTracker) override
	{
		LOG_FUNCTION;
		Super::PreReplication(ChangedPropertyTracker);
	}

	virtual void PreSave(FObjectPreSaveContext SaveContext) override
	{
		LOG_FUNCTION;
		Super::PreSave(SaveContext);
	}

	virtual void PreSaveRoot(FObjectPreSaveRootContext ObjectSaveContext) override
	{
		LOG_FUNCTION;
		Super::PreSaveRoot(ObjectSaveContext);
	}

	virtual UClass* RegenerateClass(UClass* ClassToRegenerate, UObject* PreviousCDO) override
	{
		LOG_FUNCTION_AND_PARAMS("ClassToRegenerate = %p, PreviousCDO = %p", ClassToRegenerate, PreviousCDO);
		return Super::RegenerateClass(ClassToRegenerate, PreviousCDO);
	}

	virtual void RegisterComponentTickFunctions(bool bRegister) override
	{
		LOG_FUNCTION_AND_PARAMS("bRegister = %s", bRegister ? TEXT("true") : TEXT("false"));
		Super::RegisterComponentTickFunctions(bRegister);
	}

	virtual void RegisterDependencies() override
	{
		LOG_FUNCTION;
		Super::RegisterDependencies();
	}

	virtual bool ResolveSubobject(const TCHAR* SubObjectPath, UObject*& OutObject, bool bLoadIfExists) override
	{
		LOG_FUNCTION_AND_PARAMS("SubObjectPath = %s, bLoadIfExists = %s", SubObjectPath,
		                        bLoadIfExists ? TEXT("true") : TEXT("false"));
		return Super::ResolveSubobject(SubObjectPath, OutObject, bLoadIfExists);
	}

	virtual void UpdateComponentToWorld(EUpdateTransformFlags UpdateTransformFlags = EUpdateTransformFlags::None,
	                                    ETeleportType Teleport = ETeleportType::None) override
	{
		LOG_FUNCTION_AND_PARAMS("UpdateTransformFlags = %d, Teleport = %s", UpdateTransformFlags,
		                        *UEnum::GetValueAsString(Teleport));
		Super::UpdateComponentToWorld(UpdateTransformFlags, Teleport);
	}

	virtual ~UComponentLifecycleTracer() override
	{
		LOG_FUNCTION;
	}
};
