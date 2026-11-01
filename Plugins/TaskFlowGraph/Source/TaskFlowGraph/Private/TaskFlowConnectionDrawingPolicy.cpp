#include "TaskFlowConnectionDrawingPolicy.h"

#if WITH_EDITOR

#include "Framework/Application/SlateApplication.h"
#include "Nodes/TaskFlowNode.h"
#include "Styling/AppStyle.h"

FTaskFlowConnectionDrawingPolicy::FTaskFlowConnectionDrawingPolicy(int32 InBackLayerID, int32 InFrontLayerID, float InZoomFactor, const FSlateRect& InClippingRect, FSlateWindowElementList& InDrawElements, UEdGraph* InGraphObj)
	: FConnectionDrawingPolicy(InBackLayerID, InFrontLayerID, InZoomFactor, InClippingRect, InDrawElements)
{
	// [EN] The graph object is provided by the schema but the base policy no longer stores it; we read node state from the pins directly.
	// [CN] 基类不再保存图对象（由 schema 传入），节点状态直接从引脚读取，这里忽略该参数。
	(void)InGraphObj;
}

void FTaskFlowConnectionDrawingPolicy::DrawConnection(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params)
{
	// [EN] Draw the normal wire first.
	// [CN] 先绘制普通连线
	FConnectionDrawingPolicy::DrawConnection(LayerId, Start, End, Params);

	// [EN] The receiving end of a flow link is the input pin (AssociatedPin2). Animate only when that node is running or done.
	// [CN] 流程连线的接收端是输入引脚（AssociatedPin2）；仅当该节点正在执行或已执行成功时才播放动效
	const UEdGraphPin* TargetPin = Params.AssociatedPin2;
	if (TargetPin == nullptr)
	{
		return;
	}

	const UTaskFlowNode* TargetNode = Cast<UTaskFlowNode>(TargetPin->GetOwningNode());
	if (TargetNode == nullptr)
	{
		return;
	}

	const ETaskFlowNodeState State = TargetNode->GetNodeState();
	if (State == ETaskFlowNodeState::Executing || State == ETaskFlowNodeState::Succeeded)
	{
		DrawFlowDots(LayerId + 1, Start, End, Params);
	}
}

namespace
{
	/** [EN] Sample a point at the given arc-length distance along a polyline. [CN] 沿折线按弧长取点 */
	FVector2f SampleAtArcLength(const TArray<FVector2f>& Points, const TArray<float>& Cumulative, float Distance)
	{
		for (int32 i = 1; i < Cumulative.Num(); ++i)
		{
			if (Distance <= Cumulative[i] || i == Cumulative.Num() - 1)
			{
				const float SegLen = Cumulative[i] - Cumulative[i - 1];
				const float t = SegLen > KINDA_SMALL_NUMBER ? (Distance - Cumulative[i - 1]) / SegLen : 0.f;
				return FMath::Lerp(Points[i - 1], Points[i], FMath::Clamp(t, 0.f, 1.f));
			}
		}
		return Points.Last();
	}
}

void FTaskFlowConnectionDrawingPolicy::DrawFlowDots(int32 LayerId, const FVector2f& Start, const FVector2f& End, const FConnectionParams& Params) const
{
	// [EN] Rebuild the exact same cubic bezier the base policy uses, so dots sit on the wire.
	// [CN] 重建基类使用的三次贝塞尔曲线，让圆点精确落在线上
	const FVector2f P0 = Start;
	const FVector2f P1 = End;
	const FVector2f SplineTangent = ComputeSplineTangent(P0, P1);

	const FVector2f P0Tangent = (Params.StartTangent.IsNearlyZero()) ? ((Params.StartDirection == EGPD_Output) ? SplineTangent : -SplineTangent) : Params.StartTangent;
	const FVector2f P1Tangent = (Params.EndTangent.IsNearlyZero()) ? ((Params.EndDirection == EGPD_Input) ? SplineTangent : -SplineTangent) : Params.EndTangent;

	const int32 Samples = 48;
	TArray<FVector2f> Points;
	Points.Reserve(Samples + 1);
	TArray<float> Cumulative;
	Cumulative.Reserve(Samples + 1);

	Points.Add(P0);
	Cumulative.Add(0.f);
	float Length = 0.f;
	for (int32 i = 1; i <= Samples; ++i)
	{
		const float T = static_cast<float>(i) / static_cast<float>(Samples);
		const FVector2f Pt = FMath::CubicInterp(P0, P0Tangent, P1, P1Tangent, T);
		Length += (Pt - Points.Last()).Size();
		Points.Add(Pt);
		Cumulative.Add(Length);
	}

	if (Length < KINDA_SMALL_NUMBER)
	{
		return;
	}

	// [EN] Marching offset driven by wall-clock time so dots flow from source to target.
	// [CN] 用真实时间驱动偏移，让圆点从源头流向目标
	const double Time = FSlateApplication::Get().GetCurrentTime();
	const float Spacing = 22.f;
	const float Radius = 2.5f;
	const float Offset = FMath::Fmod(static_cast<float>(Time) * 40.f, Spacing);

	FLinearColor DotColor = Params.WireColor;
	if (const UTaskFlowNode* TargetNode = Cast<UTaskFlowNode>(Params.AssociatedPin2->GetOwningNode()))
	{
		DotColor = TargetNode->GetStateColor();
	}

	const FSlateBrush* DotBrush = FAppStyle::GetBrush("WhiteBrush");

	for (float d = Offset; d < Length; d += Spacing)
	{
		const FVector2f Pos = SampleAtArcLength(Points, Cumulative, d);
		FSlateDrawElement::MakeBox(
			DrawElementsList,
			LayerId,
			FPaintGeometry(FVector2f(Pos.X - Radius, Pos.Y - Radius), FVector2f(Radius * 2.f, Radius * 2.f), 1.f),
			DotBrush,
			ESlateDrawEffect::None,
			DotColor);
	}
}

#endif // WITH_EDITOR
