#pragma once

#include "CoreMinimal.h"
#include "Delegates/IDelegateInstance.h"
#include "Framework/Docking/TabManager.h"
#include "GraphEditor.h"
#include "Toolkits/AssetEditorToolkit.h"

class FUICommandList;
class IDetailsView;
class SGraphEditor;
class UTaskFlowAsset;
class UTaskFlowNode;

/** Task Flow asset editor: the flow Graph on the left, the selected node's properties on the right. */
class FTaskFlowGraphEditor : public FAssetEditorToolkit
{
public:
	void InitTaskFlowGraphEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UTaskFlowAsset* InAsset);

	//~ Begin IToolkit Interface
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	//~ End IToolkit Interface

	//~ Begin FAssetEditorToolkit Interface
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void OnClose() override;
	//~ End FAssetEditorToolkit Interface

private:
	TSharedRef<SDockTab> SpawnTab_Graph(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	FGraphAppearanceInfo GetGraphAppearance() const;
	void OnSelectedNodesChanged(const FGraphPanelSelectionSet& SelectedNodes);

	/** Double-clicking a Sub Flow node opens its bound TaskFlowAsset in a task flow editor. */
	void OnNodeDoubleClicked(UEdGraphNode* Node);

	void CreateInternalWidgets();

	/** Delete the currently selected nodes (Delete key). */
	void DeleteSelectedNodes();
	bool CanDeleteSelectedNodes() const;

	/** Copy the currently selected nodes (Ctrl+C). */
	void CopySelectedNodes();
	bool CanCopyNodes() const;

	/** Cut the currently selected nodes (Ctrl+X). */
	void CutSelectedNodes();
	bool CanCutNodes() const;

	/** Paste nodes from the clipboard (Ctrl+V). */
	void PasteNodes();
	bool CanPasteNodes() const;

	/** Copy and paste (Ctrl+D). */
	void DuplicateNodes();
	bool CanDuplicateNodes() const;

	/** Export the graph to a Json file. */
	void ExportToJson();

	/** Import the graph from a Json file. */
	void ImportFromJson();

	/** Extend the toolbar (add export/import buttons). */
	void ExtendToolbar();

	/** Refresh the node's display color when its state changes. */
	void OnNodeStateChanged(UTaskFlowNode* Node);

	/** Reset all nodes to gray when PIE ends. */
	void OnPIEEnded(bool bIsSimulating);

	/** Begin PIE: start animating connection flows. */
	void OnPIEBegan(bool bIsSimulating);

	/** Start the per-frame animation ticker (called on PIE begin or when a node starts executing). */
	void StartAnimation();

	/** Stop the animation ticker. */
	void StopAnimation();

	/** Per-frame ticker that repaints the graph panel so the flow dots keep animating. */
	bool TickAnimate(float DeltaTime);

private:
	UTaskFlowAsset* TaskFlowAsset = nullptr;

	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<IDetailsView> DetailsView;

	TSharedPtr<FUICommandList> GraphEditorCommands;

	TSharedPtr<FWorkspaceItem> WorkspaceMenuCategory;

	/** Subscription handle for the node-state-changed broadcast. */
	FDelegateHandle NodeStateChangedHandle;

	/** Subscription handle for the PIE-ended event. */
	FDelegateHandle PieEndHandle;

	/** Subscription handle for the PIE-began event. */
	FDelegateHandle PieBeginHandle;

	/** Whether the connection-flow animation ticker is currently active. */
	bool bIsAnimating = false;

	/** Handle of the animation ticker (FTSTicker uses a weak-pointer based handle). */
	FTSTicker::FDelegateHandle AnimTickerHandle;

	static const FName GraphTabId;
	static const FName DetailsTabId;
};
