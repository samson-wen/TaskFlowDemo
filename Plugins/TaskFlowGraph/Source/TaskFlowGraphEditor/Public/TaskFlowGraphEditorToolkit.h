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

/** 任务流程资产编辑器：左边是流程 Graph，右边是选中节点的属性 */
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

	void CreateInternalWidgets();

	/** 删除当前选中的节点（Delete 键） */
	void DeleteSelectedNodes();
	bool CanDeleteSelectedNodes() const;

	/** 节点状态变化时刷新该节点的显示颜色 */
	void OnNodeStateChanged(UTaskFlowNode* Node);

	/** PIE 结束时把所有节点重置为灰色 */
	void OnPIEEnded(bool bIsSimulating);

private:
	UTaskFlowAsset* TaskFlowAsset = nullptr;

	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<IDetailsView> DetailsView;

	TSharedPtr<FUICommandList> GraphEditorCommands;

	TSharedPtr<FWorkspaceItem> WorkspaceMenuCategory;

	/** 节点状态变化广播的订阅句柄 */
	FDelegateHandle NodeStateChangedHandle;

	/** PIE 结束事件的订阅句柄 */
	FDelegateHandle PieEndHandle;

	static const FName GraphTabId;
	static const FName DetailsTabId;
};
