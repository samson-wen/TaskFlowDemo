#pragma once

// [EN] The custom connection drawing policy relies on FConnectionDrawingPolicy, which lives in the
// [CN] 自定义连线绘制策略依赖 FConnectionDrawingPolicy，它属于编辑器模块 GraphEditor；
// [EN] editor-only GraphEditor module, so the whole type is only compiled in editor builds.
// [CN] 因此整个类型只在编辑器构建下编译。
#if WITH_EDITOR

#include "ConnectionDrawingPolicy.h"

class UTaskFlowNode;

/**
 * [EN] Custom connection drawing policy for the task flow graph.
 * [CN] 任务流图自定义的连线绘制策略。
 *
 * [EN] When the node at the receiving (input) end of a link is currently executing or has
 * [CN] 当一条连线的"接收端"（输入引脚）节点正处于执行中或已执行成功时，
 * [EN] already succeeded, it draws an animated "marching dots" flow along the wire (in the same
 * [CN] 会在连线上绘制流动的小圆点动效（与行为树的连接线动画类似），
 * [EN] spirit as the Behavior Tree editor), so the runtime execution path is visually obvious.
 * [CN] 让运行时的执行路径一目了然。
 */
class FTaskFlowConnectionDrawingPolicy : public FConnectionDrawingPolicy
{
public:
	FTaskFlowConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj);

	//~ Begin FConnectionDrawingPolicy Interface
	virtual void DrawConnection(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params) override;
	//~ End FConnectionDrawingPolicy Interface

private:
	/** [EN] Draw the animated flowing dots along the wire described by Start->End. [CN] 沿 Start->End 描述的连线绘制流动小圆点 */
	void DrawFlowDots(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params) const;
};

#endif // WITH_EDITOR
