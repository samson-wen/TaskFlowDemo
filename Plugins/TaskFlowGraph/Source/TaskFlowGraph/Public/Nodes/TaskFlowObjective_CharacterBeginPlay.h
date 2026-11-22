#pragma once

#include "CoreMinimal.h"
#include "Nodes/TaskFlowNode_Objective.h"
#include "TaskFlowObjective_CharacterBeginPlay.generated.h"

class ACharacter;
class APawn;

/**
 * Generic Objective that observes a Character beginning play through engine delegates, then runs
 * its Actions with that Character as the flow context. No custom task-flow event name or manual
 * broadcasting is needed.
 *
 * Detection: on execution it subscribes to APawn::OnPawnBeginPlay, a static engine delegate that
 * fires for every Pawn's BeginPlay. The callback keeps only ACharacter instances (and optionally
 * the CharacterClass filter), injects the Character into the Runner context and starts Actions.
 *
 * Why not "OnActorSpawned -> then OnPawnBeginPlay": AActor::FinishSpawning calls
 * PostActorConstruction (which dispatches BeginPlay) BEFORE UWorld::OnActorFinishedSpawning
 * broadcasts OnActorSpawned (Actor.cpp:4420-4425 in UE 5.7). Arming a BeginPlay listener from an
 * OnActorSpawned callback is therefore always too late for that same Character. Subscribing the
 * static Pawn delegate directly also covers Characters pre-placed in the level.
 *
 * Before starting Actions the node calls Runner->SetContext(Character), so UTaskFlowAction::
 * GetContext() and UTaskFlowNode::GetContext() return the triggering Character inside Actions.
 * The delegate registration is released on completion and in OnFlowEnded.
 */
UCLASS(Blueprintable, meta = (DisplayName = "Character Begin Play Objective", TaskFlowCategory = "Task"))
class TASKFLOWGRAPH_API UTaskFlowObjective_CharacterBeginPlay : public UTaskFlowNode_Objective
{
	GENERATED_BODY()

public:
	UTaskFlowObjective_CharacterBeginPlay();

	//~ Begin UEdGraphNode Interface
	virtual FText GetTooltipText() const override;
	//~ End UEdGraphNode Interface

	virtual void Execute_Implementation(UTaskFlowRunner* Runner) override;

protected:
	virtual void OnFlowEnded() override;
	virtual bool ShouldDeferActionsUntilEvent() const override;

public:
	/** Optional class filter. nullptr matches any ACharacter. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Begin Play")
	TSubclassOf<ACharacter> CharacterClass;

private:
	/** APawn::OnPawnBeginPlay callback: filter the Character, inject context and start Actions. */
	void HandlePawnBeginPlay(APawn* Pawn);

	/** Release the static Pawn delegate registration. */
	void UnsubscribeFromBeginPlay();

	FDelegateHandle PawnBeginPlayHandle;
};
