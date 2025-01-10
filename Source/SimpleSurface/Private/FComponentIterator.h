#pragma once

/**
 * Iterates over an actor's components and exposes a collection whose elements represent each component and a list of
 * child indices that may be used to located it.
 *
 * Useful for capturing component-related data in a way that can be copied between objects without risking cross-references.  
 */
class FComponentIterator
{
public:
	FComponentIterator(const AActor* InActor)
	{
		if (InActor)
		{
			Components.Emplace(InActor->GetRootComponent(), TArray<int32>());
			TraverseComponents(InActor->GetRootComponent());
		}
	}

	struct FComponentInfo
	{
		UActorComponent* Component;
		TArray<int32> ChildIndices;
	};

	const TArray<FComponentInfo>& GetComponents() const { return Components; }

private:
	void TraverseComponents(USceneComponent* Component, TArray<int32> IndicesStack = {})
	{
		if (!Component)
		{
			return;
		}

		int32 Depth = IndicesStack.Num();
		IndicesStack.Reserve(Depth + 1);

		for (int32 i = 0; i < Component->GetNumChildrenComponents(); ++i)
		{
			IndicesStack[Depth] = i;
			Components.Add({Component, IndicesStack});

			TArray<USceneComponent*> Children;
			Component->GetChildrenComponents(true, Children);
			if (Children.Num() > 0)
			{
				TraverseComponents(Children[i], IndicesStack);
			}
		}
	}

	TArray<FComponentInfo> Components;
};
