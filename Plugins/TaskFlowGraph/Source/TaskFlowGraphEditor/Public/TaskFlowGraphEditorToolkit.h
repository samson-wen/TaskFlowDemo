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

/** [EN] Task Flow asset editor: the flow Graph on the left, the selected node's properties on the right. [CN] 任务流程资产编辑器：左边是流程 Graph，右边是选中节点的属性 */
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

	/** [EN] Delete the currently selected nodes (Delete key). [CN] 删除当前选中的节点（Delete 键） */
	void DeleteSelectedNodes();
	bool CanDeleteSelectedNodes() const;

	/** [EN] Copy the currently selected nodes (Ctrl+C). [CN] 复制当前选中的节点（Ctrl+C） */
	void CopySelectedNodes();
	bool CanCopyNodes() const;

	/** [EN] Cut the currently selected nodes (Ctrl+X). [CN] 剪切当前选中的节点（Ctrl+X） */
	void CutSelectedNodes();
	bool CanCutNodes() const;

	/** [EN] Paste nodes from the clipboard (Ctrl+V). [CN] 粘贴剪贴板中的节点（Ctrl+V） */
	void PasteNodes();
	bool CanPasteNodes() const;

	/** [EN] Copy and paste (Ctrl+D). [CN] 复制并粘贴（Ctrl+D） */
	void DuplicateNodes();
	bool CanDuplicateNodes() const;

	/** [EN] Export the graph to a Json file. [CN] 导出图到 Json 文件 */
	void ExportToJson();

	/** [EN] Import the graph from a Json file. [CN] 从 Json 文件导入图 */
	void ImportFromJson();

	/** [EN] Extend the toolbar (add export/import buttons). [CN] 扩展工具栏（添加导出/导入按钮） */
	void ExtendToolbar();

	/** [EN] Refresh the node's display color when its state changes. [CN] 节点状态变化时刷新该节点的显示颜色 */
	void OnNodeStateChanged(UTaskFlowNode* Node);

	/** [EN] Reset all nodes to gray when PIE ends. [CN] PIE 结束时把所有节点重置为灰色 */
	void OnPIEEnded(bool bIsSimulating);

	/** [EN] Begin PIE: start animating connection flows. [CN] PIE 开始：启动连接线流动动效 */
	void OnPIEBegan(bool bIsSimulating);

	/** [EN] Start the per-frame animation ticker (called on PIE begin or when a node starts executing). [CN] 启动按帧刷新的动画 ticker（PIE 开始或节点开始执行时调用） */
	void StartAnimation();

	/** [EN] Stop the animation ticker. [CN] 停止动画 ticker */
	void StopAnimation();

	/** [EN] Per-frame ticker that repaints the graph panel so the flow dots keep animating. [CN] 每帧 ticker，重绘图面板以保持连线动效持续播放 */
	bool TickAnimate(float DeltaTime);

private:
	UTaskFlowAsset* TaskFlowAsset = nullptr;

	TSharedPtr<SGraphEditor> GraphEditor;
	TSharedPtr<IDetailsView> DetailsView;

	TSharedPtr<FUICommandList> GraphEditorCommands;

	TSharedPtr<FWorkspaceItem> WorkspaceMenuCategory;

	/** [EN] Subscription handle for the node-state-changed broadcast. [CN] 节点状态变化广播的订阅句柄 */
	FDelegateHandle NodeStateChangedHandle;

	/** [EN] Subscription handle for the PIE-ended event. [CN] PIE 结束事件的订阅句柄 */
	FDelegateHandle PieEndHandle;

	/** [EN] Subscription handle for the PIE-began event. [CN] PIE 开始事件的订阅句柄 */
	FDelegateHandle PieBeginHandle;

	/** [EN] Whether the connection-flow animation ticker is currently active. [CN] 连接线动效 ticker 是否正在运行 */
	bool bIsAnimating = false;

	/** [EN] Handle of the animation ticker (FTSTicker uses a weak-pointer based handle). [CN] 动画 ticker 的句柄（FTSTicker 使用基于弱指针的句柄类型） */
	FTSTicker::FDelegateHandle AnimTickerHandle;

	static const FName GraphTabId;
	static const FName DetailsTabId;
};
