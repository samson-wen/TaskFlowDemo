#pragma once

// The custom connection drawing policy relies on FConnectionDrawingPolicy, which lives in the
// editor-only GraphEditor module, so the whole type is only compiled in editor builds.
#if WITH_EDITOR

#include "ConnectionDrawingPolicy.h"

class UTaskFlowNode;

/**
 * Custom connection drawing policy for the task flow graph.
 *
 * When the node at the receiving (input) end of a link is currently executing or has
 * already succeeded, it draws an animated "marching dots" flow along the wire (in the same
 * spirit as the Behavior Tree editor), so the runtime execution path is visually obvious.
 */
class FTaskFlowConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FTaskFlowConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);

	//~ Begin FConnectionDrawingPolicy Interface
	virtual void DrawConnection(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params) override;
	//~ End FConnectionDrawingPolicy Interface

private:
	/** Draw the animated flowing dots along the wire described by Start->End. */
	void DrawFlowDots(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params) const;
};

#endif // WITH_EDITOR
