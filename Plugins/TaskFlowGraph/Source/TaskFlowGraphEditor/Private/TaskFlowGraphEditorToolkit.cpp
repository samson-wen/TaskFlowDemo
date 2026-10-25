#include "TaskFlowGraphEditorToolkit.h"

#include "EdGraph/EdGraphNode.h"
#include "Editor.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"
#include "IDetailsView.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Widgets/Docking/SDockTab.h"

#include "Nodes/TaskFlowNode.h"
#include "TaskFlowAsset.h"
#include "TaskFlowGraph.h"

#define LOCTEXT_NAMESPACE "TaskFlowGraphEditorToolkit"

static const FName TaskFlowGraphEditorAppIdentifier(TEXT("TaskFlowGraphEditorApp"));

const FName FTaskFlowGraphEditor::GraphTabId(TEXT("TaskFlowGraphEditor_Graph"));
const FName FTaskFlowGraphEditor::DetailsTabId(TEXT("TaskFlowGraphEditor_Details"));

void FTaskFlowGraphEditor::InitTaskFlowGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UTaskFlowAsset* InAsset)
{
	TaskFlowAsset = InAsset;

	CreateInternalWidgets();

	const TSharedRef<FTabManager::FLayout> StandaloneDefaultLayout = FTabManager::NewLayout("Standalone_TaskFlowGraphEditor_Layout_v1")
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->SetSizeCoefficient(0.9f)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.7f)
					->AddTab(GraphTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.3f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

	const bool bCreateDefaultStandaloneMenu = true;
	const bool bCreateDefaultToolbar = true;

	InitAssetEditor(Mode, InitToolkitHost, TaskFlowGraphEditorAppIdentifier, StandaloneDefaultLayout, bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, InAsset);
}

void FTaskFlowGraphEditor::CreateInternalWidgets()
{
	check(TaskFlowAsset != nullptr);

	UTaskFlowGraph* Graph = TaskFlowAsset->CreateOrLoadGraph();
	check(Graph != nullptr);

	SGraphEditor::FGraphEditorEvents GraphEvents;
	GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FTaskFlowGraphEditor::OnSelectedNodesChanged);

	// 绑定 Delete 键：删除选中节点
	FGenericCommands::Register();
	GraphEditorCommands = MakeShared<FUICommandList>();
	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CanDeleteSelectedNodes));

	GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(Graph)
		.IsEditable(true)
		.AdditionalCommands(GraphEditorCommands)
		.Appearance(this, &FTaskFlowGraphEditor::GetGraphAppearance)
		.GraphEvents(GraphEvents);

	// 订阅节点状态变化，运行时刷新节点颜色（灰/黄/绿）
	NodeStateChangedHandle = UTaskFlowNode::OnNodeStateChanged().AddRaw(this, &FTaskFlowGraphEditor::OnNodeStateChanged);

	// 订阅 PIE 结束，停止运行时把节点重置为灰色
	PieEndHandle = FEditorDelegates::EndPIE.AddRaw(this, &FTaskFlowGraphEditor::OnPIEEnded);

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.bAllowSearch = true;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	DetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(TaskFlowAsset);
}

FName FTaskFlowGraphEditor::GetToolkitFName() const
{
	return FName(TEXT("TaskFlowGraphEditor"));
}

FText FTaskFlowGraphEditor::GetBaseToolkitName() const
{
	return LOCTEXT("TaskFlowGraphEditor_AppLabel", "任务流程编辑器");
}

FString FTaskFlowGraphEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("TaskFlowGraphEditor_WorldCentricTabPrefix", "任务流程 ").ToString();
}

FLinearColor FTaskFlowGraphEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.79f, 0.56f, 0.24f);
}

void FTaskFlowGraphEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_TaskFlowGraphEditor", "任务流程编辑器"));
	TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(GraphTabId, FOnSpawnTab::CreateSP(this, &FTaskFlowGraphEditor::SpawnTab_Graph))
		.SetDisplayName(LOCTEXT("GraphTab_Title", "流程图"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FTaskFlowGraphEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab_Title", "细节"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "LevelEditor.Tabs.Details"));
}

void FTaskFlowGraphEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(GraphTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
}

void FTaskFlowGraphEditor::OnClose()
{
	UTaskFlowNode::OnNodeStateChanged().Remove(NodeStateChangedHandle);
	FEditorDelegates::EndPIE.Remove(PieEndHandle);

	FAssetEditorToolkit::OnClose();

	GraphEditor.Reset();
	DetailsView.Reset();
	TaskFlowAsset = nullptr;
}

TSharedRef<SDockTab> FTaskFlowGraphEditor::SpawnTab_Graph(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == GraphTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("GraphTab_Label", "流程图"))
		[
			GraphEditor.ToSharedRef()
		];
}

TSharedRef<SDockTab> FTaskFlowGraphEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DetailsTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Label", "细节"))
		[
			DetailsView.ToSharedRef()
		];
}

FGraphAppearanceInfo FTaskFlowGraphEditor::GetGraphAppearance() const
{
	FGraphAppearanceInfo Appearance;
	Appearance.CornerText = LOCTEXT("TaskFlowGraph_CornerText", "任务流程");
	Appearance.InstructionText = LOCTEXT("TaskFlowGraph_InstructionText", "右键添加节点，拖动引脚连线：从「完成」连到下一个节点的「进入」");

	return Appearance;
}

void FTaskFlowGraphEditor::OnSelectedNodesChanged(const FGraphPanelSelectionSet& SelectedNodes)
{
	if (!DetailsView.IsValid())
	{
		return;
	}

	TArray<UObject*> Objects;
	Objects.Reserve(SelectedNodes.Num());
	for (UObject* Object : SelectedNodes)
	{
		if (Object != nullptr)
		{
			Objects.Add(Object);
		}
	}

	if (Objects.Num() > 0)
	{
		DetailsView->SetObjects(Objects);
	}
	else if (TaskFlowAsset != nullptr)
	{
		DetailsView->SetObject(TaskFlowAsset);
	}
}

void FTaskFlowGraphEditor::DeleteSelectedNodes()
{
	if (!GraphEditor.IsValid())
	{
		return;
	}

	UEdGraph* Graph = GraphEditor->GetCurrentGraph();
	if (Graph == nullptr)
	{
		return;
	}

	// 先收集可删除的节点，避免边遍历选择集边修改
	TArray<UEdGraphNode*> NodesToDelete;
	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
	for (UObject* Object : SelectedNodes)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			NodesToDelete.Add(Node);
		}
	}

	if (NodesToDelete.Num() == 0)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteTaskFlowNodes", "删除任务节点"));
	Graph->Modify();

	for (UEdGraphNode* Node : NodesToDelete)
	{
		Node->DestroyNode();
	}
}

bool FTaskFlowGraphEditor::CanDeleteSelectedNodes() const
{
	if (!GraphEditor.IsValid())
	{
		return false;
	}

	const FGraphPanelSelectionSet SelectedNodes = GraphEditor->GetSelectedNodes();
	for (UObject* Object : SelectedNodes)
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FTaskFlowGraphEditor::OnNodeStateChanged(UTaskFlowNode* Node)
{
	if (GraphEditor.IsValid() && Node != nullptr)
	{
		GraphEditor->RefreshNode(*Node);
	}
}

void FTaskFlowGraphEditor::OnPIEEnded(bool bIsSimulating)
{
	if (!GraphEditor.IsValid())
	{
		return;
	}

	UEdGraph* Graph = GraphEditor->GetCurrentGraph();
	if (Graph == nullptr)
	{
		return;
	}

	// 停止运行后，把所有节点重置为未执行（灰色）
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (UTaskFlowNode* TaskNode = Cast<UTaskFlowNode>(Node))
		{
			TaskNode->SetNodeState(ETaskFlowNodeState::NotExecuted);
		}
	}
}

#undef LOCTEXT_NAMESPACE
