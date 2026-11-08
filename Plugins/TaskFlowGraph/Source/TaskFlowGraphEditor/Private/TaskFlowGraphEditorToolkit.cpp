#include "TaskFlowGraphEditorToolkit.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphUtilities.h"
#include "DesktopPlatformModule.h"
#include "Editor.h"
#include "Framework/Application/SlateApplication.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/Docking/TabManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "HAL/PlatformApplicationMisc.h"
#include "IDesktopPlatform.h"
#include "IDetailsView.h"
#include "Math/Vector2D.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Containers/Ticker.h"
#include "PropertyEditorModule.h"
#include "SGraphPanel.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Subsystems/AssetEditorSubsystem.h"
#include "Widgets/Docking/SDockTab.h"

#include "Nodes/TaskFlowNode.h"
#include "Nodes/TaskFlowNode_SubFlow.h"
#include "TaskFlowAsset.h"
#include "TaskFlowGraph.h"
#include "TaskFlowGraphSerializer.h"
#include "TaskFlowTypes.h"

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

	ExtendToolbar();
	RegenerateMenusAndToolbars();
}

void FTaskFlowGraphEditor::CreateInternalWidgets()
{
	check(TaskFlowAsset != nullptr);

	UTaskFlowGraph* Graph = TaskFlowAsset->CreateOrLoadGraph();
	check(Graph != nullptr);

	SGraphEditor::FGraphEditorEvents GraphEvents;
	GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FTaskFlowGraphEditor::OnSelectedNodesChanged);
	GraphEvents.OnNodeDoubleClicked = FSingleNodeEvent::CreateSP(this, &FTaskFlowGraphEditor::OnNodeDoubleClicked);

	// Bind the Delete key to delete selected nodes.
	FGenericCommands::Register();
	GraphEditorCommands = MakeShared<FUICommandList>();
	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CanDeleteSelectedNodes));

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Copy,
		FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CopySelectedNodes),
		FCanExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CanCopyNodes));

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Cut,
		FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CutSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CanCutNodes));

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Paste,
		FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::PasteNodes),
		FCanExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CanPasteNodes));

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Duplicate,
		FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::DuplicateNodes),
		FCanExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::CanDuplicateNodes));

	GraphEditor = SNew(SGraphEditor)
		.GraphToEdit(Graph)
		.IsEditable(true)
		.AdditionalCommands(GraphEditorCommands)
		.Appearance(this, &FTaskFlowGraphEditor::GetGraphAppearance)
		.GraphEvents(GraphEvents);

	// Subscribe to node state changes to refresh node colors at runtime (gray/yellow/green).
	NodeStateChangedHandle = UTaskFlowNode::OnNodeStateChanged().AddRaw(this, &FTaskFlowGraphEditor::OnNodeStateChanged);

	// Subscribe to PIE end to reset nodes to gray after stopping a run.
	PieEndHandle = FEditorDelegates::EndPIE.AddRaw(this, &FTaskFlowGraphEditor::OnPIEEnded);

	// Subscribe to PIE begin to start the connection-flow animation.
	PieBeginHandle = FEditorDelegates::BeginPIE.AddRaw(this, &FTaskFlowGraphEditor::OnPIEBegan);

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
	return LOCTEXT("TaskFlowGraphEditor_AppLabel", "Task Flow Graph Editor");
}

FString FTaskFlowGraphEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("TaskFlowGraphEditor_WorldCentricTabPrefix", "Task Flow ").ToString();
}

FLinearColor FTaskFlowGraphEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.79f, 0.56f, 0.24f);
}

void FTaskFlowGraphEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_TaskFlowGraphEditor", "Task Flow Graph Editor"));
	TSharedRef<FWorkspaceItem> WorkspaceMenuCategoryRef = WorkspaceMenuCategory.ToSharedRef();

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(GraphTabId, FOnSpawnTab::CreateSP(this, &FTaskFlowGraphEditor::SpawnTab_Graph))
		.SetDisplayName(LOCTEXT("GraphTab_Title", "Graph"))
		.SetGroup(WorkspaceMenuCategoryRef)
		.SetIcon(FSlateIcon(FAppStyle::GetAppStyleSetName(), "GraphEditor.EventGraph_16x"));

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FTaskFlowGraphEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab_Title", "Details"))
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
	FEditorDelegates::BeginPIE.Remove(PieBeginHandle);
	StopAnimation();

	FAssetEditorToolkit::OnClose();

	GraphEditor.Reset();
	DetailsView.Reset();
	TaskFlowAsset = nullptr;
}

TSharedRef<SDockTab> FTaskFlowGraphEditor::SpawnTab_Graph(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == GraphTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("GraphTab_Label", "Graph"))
		[
			GraphEditor.ToSharedRef()
		];
}

TSharedRef<SDockTab> FTaskFlowGraphEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	check(Args.GetTabId() == DetailsTabId);

	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTab_Label", "Details"))
		[
			DetailsView.ToSharedRef()
		];
}

FGraphAppearanceInfo FTaskFlowGraphEditor::GetGraphAppearance() const
{
	FGraphAppearanceInfo Appearance;
	Appearance.CornerText = LOCTEXT("TaskFlowGraph_CornerText", "Task Flow");
	Appearance.InstructionText = LOCTEXT("TaskFlowGraph_InstructionText", "Right-click to add nodes, then drag pins to connect them: link an output pin to the next node's input pin");

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

	// Collect deletable nodes first to avoid modifying the selection set while iterating.
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

	const FScopedTransaction Transaction(LOCTEXT("DeleteTaskFlowNodes", "Delete Task Flow Nodes"));
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

void FTaskFlowGraphEditor::CopySelectedNodes()
{
	if (!GraphEditor.IsValid())
	{
		return;
	}

	FGraphPanelSelectionSet NodesToCopy;
	for (UObject* Object : GraphEditor->GetSelectedNodes())
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node != nullptr && Node->CanDuplicateNode())
		{
			Node->PrepareForCopying();
			NodesToCopy.Add(Node);
		}
	}

	if (NodesToCopy.Num() == 0)
	{
		return;
	}

	FString ExportedText;
	FEdGraphUtilities::ExportNodesToText(NodesToCopy, ExportedText);
	FPlatformApplicationMisc::ClipboardCopy(*ExportedText);
}

bool FTaskFlowGraphEditor::CanCopyNodes() const
{
	if (!GraphEditor.IsValid())
	{
		return false;
	}

	for (UObject* Object : GraphEditor->GetSelectedNodes())
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Object);
		if (Node != nullptr && Node->CanDuplicateNode())
		{
			return true;
		}
	}

	return false;
}

void FTaskFlowGraphEditor::CutSelectedNodes()
{
	CopySelectedNodes();

	if (!GraphEditor.IsValid())
	{
		return;
	}

	UEdGraph* Graph = GraphEditor->GetCurrentGraph();
	if (Graph == nullptr)
	{
		return;
	}

	TArray<UEdGraphNode*> NodesToDelete;
	for (UObject* Object : GraphEditor->GetSelectedNodes())
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

	const FScopedTransaction Transaction(FGenericCommands::Get().Cut->GetDescription());
	Graph->Modify();
	for (UEdGraphNode* Node : NodesToDelete)
	{
		Node->DestroyNode();
	}
}

bool FTaskFlowGraphEditor::CanCutNodes() const
{
	return CanCopyNodes() && CanDeleteSelectedNodes();
}

void FTaskFlowGraphEditor::PasteNodes()
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

	const FVector2f PasteLocation = GraphEditor->GetPasteLocation2f();

	FString TextToImport;
	FPlatformApplicationMisc::ClipboardPaste(TextToImport);
	if (TextToImport.IsEmpty())
	{
		return;
	}

	const FScopedTransaction Transaction(FGenericCommands::Get().Paste->GetDescription());
	Graph->Modify();

	// Clear the selection; pasted nodes will be selected again.
	GraphEditor->ClearSelectionSet();

	TSet<UEdGraphNode*> PastedNodes;
	FEdGraphUtilities::ImportNodesFromText(Graph, TextToImport, PastedNodes);
	if (PastedNodes.Num() == 0)
	{
		return;
	}

	// Compute the average position and translate the whole group to the paste location.
	FVector2f AvgNodePosition(0.f, 0.f);
	for (UEdGraphNode* Node : PastedNodes)
	{
		AvgNodePosition.X += static_cast<float>(Node->NodePosX);
		AvgNodePosition.Y += static_cast<float>(Node->NodePosY);
	}
	AvgNodePosition /= static_cast<float>(PastedNodes.Num());

	for (UEdGraphNode* Node : PastedNodes)
	{
		Node->NodePosX = static_cast<int32>(static_cast<float>(Node->NodePosX) - AvgNodePosition.X + PasteLocation.X);
		Node->NodePosY = static_cast<int32>(static_cast<float>(Node->NodePosY) - AvgNodePosition.Y + PasteLocation.Y);
		Node->SnapToGrid(16);
		Node->CreateNewGuid();
		GraphEditor->SetNodeSelection(Node, true);
	}

	Graph->NotifyGraphChanged();
}

bool FTaskFlowGraphEditor::CanPasteNodes() const
{
	if (!GraphEditor.IsValid())
	{
		return false;
	}

	UEdGraph* Graph = GraphEditor->GetCurrentGraph();
	if (Graph == nullptr)
	{
		return false;
	}

	FString ClipboardContent;
	FPlatformApplicationMisc::ClipboardPaste(ClipboardContent);

	return FEdGraphUtilities::CanImportNodesFromText(Graph, ClipboardContent);
}

void FTaskFlowGraphEditor::DuplicateNodes()
{
	CopySelectedNodes();
	PasteNodes();
}

bool FTaskFlowGraphEditor::CanDuplicateNodes() const
{
	return CanCopyNodes();
}

void FTaskFlowGraphEditor::OnNodeDoubleClicked(UEdGraphNode* Node)
{
	UTaskFlowNode_SubFlow* SubFlowNode = Cast<UTaskFlowNode_SubFlow>(Node);
	if (SubFlowNode == nullptr)
	{
		return;
	}

	UTaskFlowAsset* Asset = SubFlowNode->SubFlowAsset.LoadSynchronous();
	if (Asset == nullptr)
	{
		return;
	}

	// OpenEditorForAsset goes through UAssetDefinition_TaskFlow::OpenAssets, which focuses an
	// already-open editor instead of opening a duplicate one.
	if (UAssetEditorSubsystem* AssetEditorSubsystem = GEditor->GetEditorSubsystem<UAssetEditorSubsystem>())
	{
		AssetEditorSubsystem->OpenEditorForAsset(Asset);
	}
}

void FTaskFlowGraphEditor::OnNodeStateChanged(UTaskFlowNode* Node)
{
	if (GraphEditor.IsValid() && Node != nullptr)
	{
		GraphEditor->RefreshNode(*Node);

		// When a node starts executing, kick off the per-frame animation so the flow dots keep moving.
		if (Node->GetNodeState() == ETaskFlowNodeState::Executing)
		{
			StartAnimation();
		}
	}
}

void FTaskFlowGraphEditor::OnPIEBegan(bool bIsSimulating)
{
	// Begin animating connection flows while the game is running.
	StartAnimation();
}

void FTaskFlowGraphEditor::StartAnimation()
{
	if (bIsAnimating)
	{
		return;
	}

	bIsAnimating = true;
	AnimTickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateSP(this, &FTaskFlowGraphEditor::TickAnimate));
}

void FTaskFlowGraphEditor::StopAnimation()
{
	if (!bIsAnimating)
	{
		return;
	}

	bIsAnimating = false;
	if (AnimTickerHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(AnimTickerHandle);
		AnimTickerHandle.Reset();
	}
}

bool FTaskFlowGraphEditor::TickAnimate(float /*DeltaTime*/)
{
	if (GraphEditor.IsValid())
	{
		if (SGraphPanel* Panel = GraphEditor->GetGraphPanel())
		{
			// Force a repaint every frame so the flow dots keep animating.
			Panel->Invalidate(EInvalidateWidget::Paint);
		}
	}

	return bIsAnimating;
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

	// After a run stops, reset all nodes to NotExecuted (gray) and stop the flow animation.
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (UTaskFlowNode* TaskNode = Cast<UTaskFlowNode>(Node))
		{
			TaskNode->SetNodeState(ETaskFlowNodeState::NotExecuted);
		}
	}

	StopAnimation();

	if (SGraphPanel* Panel = GraphEditor->GetGraphPanel())
	{
		// Repaint once so the flow dots disappear immediately.
		Panel->Invalidate(EInvalidateWidget::Paint);
	}
}

void FTaskFlowGraphEditor::ExtendToolbar()
{
	TSharedPtr<FExtender> ToolbarExtender = MakeShareable(new FExtender);
	ToolbarExtender->AddToolBarExtension(
		TEXT("Asset"),
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& ToolbarBuilder)
		{
			ToolbarBuilder.BeginSection(TEXT("TaskFlowImportExport"));
			{
				ToolbarBuilder.AddToolBarButton(
					FUIAction(FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::ExportToJson)),
					NAME_None,
					LOCTEXT("Export_Label", "Export"),
					LOCTEXT("Export_Tooltip", "Export nodes and links to a Json file"),
					FSlateIcon(FAppStyle::Get().GetStyleSetName(), "Icons.Save"));

				ToolbarBuilder.AddToolBarButton(
					FUIAction(FExecuteAction::CreateSP(this, &FTaskFlowGraphEditor::ImportFromJson)),
					NAME_None,
					LOCTEXT("Import_Label", "Import"),
					LOCTEXT("Import_Tooltip", "Import nodes and links from a Json file"),
					FSlateIcon(FAppStyle::Get().GetStyleSetName(), "Icons.Open"));
			}
			ToolbarBuilder.EndSection();
		})
	);

	AddToolbarExtender(ToolbarExtender);
}

void FTaskFlowGraphEditor::ExportToJson()
{
	UEdGraph* Graph = GraphEditor.IsValid() ? GraphEditor->GetCurrentGraph() : nullptr;
	UTaskFlowGraph* TaskGraph = Cast<UTaskFlowGraph>(Graph);
	if (TaskGraph == nullptr)
	{
		return;
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform == nullptr)
	{
		return;
	}

	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	TArray<FString> OutFiles;
	const bool bSaved = DesktopPlatform->SaveFileDialog(
		ParentWindowHandle,
		LOCTEXT("ExportJson_DialogTitle", "Export Task Flow to Json").ToString(),
		FPaths::ProjectContentDir(),
		TEXT("TaskFlow.json"),
		TEXT("Json Files (*.json)|*.json"),
		EFileDialogFlags::None,
		OutFiles);

	if (!bSaved || OutFiles.Num() == 0)
	{
		return;
	}

	FString Json;
	if (TaskFlowGraphSerializer::ExportGraphToJson(TaskGraph, Json))
	{
		FFileHelper::SaveStringToFile(Json, *OutFiles[0], FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM);
	}
}

void FTaskFlowGraphEditor::ImportFromJson()
{
	UEdGraph* Graph = GraphEditor.IsValid() ? GraphEditor->GetCurrentGraph() : nullptr;
	UTaskFlowGraph* TaskGraph = Cast<UTaskFlowGraph>(Graph);
	if (TaskGraph == nullptr)
	{
		return;
	}

	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform == nullptr)
	{
		return;
	}

	const void* ParentWindowHandle = FSlateApplication::Get().FindBestParentWindowHandleForDialogs(nullptr);

	TArray<FString> OutFiles;
	const bool bOpened = DesktopPlatform->OpenFileDialog(
		ParentWindowHandle,
		LOCTEXT("ImportJson_DialogTitle", "Import Task Flow from Json").ToString(),
		FPaths::ProjectContentDir(),
		TEXT(""),
		TEXT("Json Files (*.json)|*.json"),
		EFileDialogFlags::None,
		OutFiles);

	if (!bOpened || OutFiles.Num() == 0)
	{
		return;
	}

	FString Json;
	if (!FFileHelper::LoadFileToString(Json, *OutFiles[0]))
	{
		return;
	}

	FString Error;
	const FScopedTransaction Transaction(LOCTEXT("ImportTaskFlowNodes", "Import Task Flow"));
	TaskGraph->Modify();

	if (!TaskFlowGraphSerializer::ImportGraphFromJson(TaskGraph, Json, Error))
	{
		UE_LOG(LogTaskFlow, Warning, TEXT("[TaskFlow] Import failed: %s"), *Error);
	}

	TaskGraph->NotifyGraphChanged();
}

#undef LOCTEXT_NAMESPACE
