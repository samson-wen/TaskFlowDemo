#include "TaskFlowGraphSerializer.h"

#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Dom/JsonObject.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"

#include "Nodes/TaskFlowNode.h"
#include "TaskFlowGraph.h"

namespace
{
	/** Find a loaded class by its short name. */
	UClass* FindClassByName(const FString& ClassName)
	{
		UClass* Class = FindObject<UClass>(nullptr, *ClassName);
		if (Class == nullptr)
		{
			Class = LoadObject<UClass>(nullptr, *FString::Printf(TEXT("/Script/TaskFlowGraph.%s"), *ClassName));
		}
		return Class;
	}

	/** Whether the property should be skipped (Transient or framework base-class fields). */
	bool ShouldSkipProperty(const FProperty* Prop)
	{
		if (Prop->HasAnyPropertyFlags(CPF_Transient))
		{
			return true;
		}

		const UStruct* Owner = Prop->GetOwnerStruct();
		return Owner != nullptr && (Owner == UEdGraphNode::StaticClass() || Owner == UObject::StaticClass());
	}

	void ExportStructProperties(UStruct* Struct, void* Container, TSharedRef<FJsonObject> Json);
	void ImportStructProperties(UStruct* Struct, void* Container, UObject* Outer, TSharedPtr<FJsonObject> Json);

	void ExportProperty(FProperty* Prop, void* ValuePtr, TSharedRef<FJsonObject> Json)
	{
		const FString Name = Prop->GetName();

		if (const FArrayProperty* ArrayProp = CastField<FArrayProperty>(Prop))
		{
			FProperty* Inner = ArrayProp->Inner;
			FScriptArrayHelper Helper(ArrayProp, ValuePtr);
			TArray<TSharedPtr<FJsonValue>> Arr;

			if (const FObjectProperty* InnerObj = CastField<FObjectProperty>(Inner))
			{
				// Instanced object array (e.g. Actions / Conditions).
				for (int32 i = 0; i < Helper.Num(); ++i)
				{
					if (UObject* SubObj = InnerObj->GetObjectPropertyValue(Helper.GetRawPtr(i)))
					{
						TSharedRef<FJsonObject> SubJson = MakeShareable(new FJsonObject());
						SubJson->SetStringField(TEXT("class"), SubObj->GetClass()->GetName());
						TSharedRef<FJsonObject> PropsJson = MakeShareable(new FJsonObject());
						ExportStructProperties(SubObj->GetClass(), SubObj, PropsJson);
						SubJson->SetObjectField(TEXT("properties"), PropsJson);
						Arr.Add(MakeShareable(new FJsonValueObject(SubJson)));
					}
				}
			}
			else if (const FStructProperty* InnerStruct = CastField<FStructProperty>(Inner))
			{
				// Struct array (e.g. Checks).
				for (int32 i = 0; i < Helper.Num(); ++i)
				{
					TSharedRef<FJsonObject> SubJson = MakeShareable(new FJsonObject());
					ExportStructProperties(InnerStruct->Struct, Helper.GetRawPtr(i), SubJson);
					Arr.Add(MakeShareable(new FJsonValueObject(SubJson)));
				}
			}

			Json->SetArrayField(Name, Arr);
			return;
		}

		if (const FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			TSharedRef<FJsonObject> SubJson = MakeShareable(new FJsonObject());
			ExportStructProperties(StructProp->Struct, ValuePtr, SubJson);
			Json->SetObjectField(Name, SubJson);
			return;
		}

		if (const FIntProperty* IntProp = CastField<FIntProperty>(Prop))
		{
			Json->SetNumberField(Name, IntProp->GetPropertyValue(ValuePtr));
		}
		else if (const FInt64Property* Int64Prop = CastField<FInt64Property>(Prop))
		{
			Json->SetNumberField(Name, static_cast<double>(Int64Prop->GetPropertyValue(ValuePtr)));
		}
		else if (const FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
		{
			Json->SetNumberField(Name, FloatProp->GetPropertyValue(ValuePtr));
		}
		else if (const FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
		{
			Json->SetNumberField(Name, DoubleProp->GetPropertyValue(ValuePtr));
		}
		else if (const FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
		{
			Json->SetBoolField(Name, BoolProp->GetPropertyValue(ValuePtr));
		}
		else if (const FStrProperty* StrProp = CastField<FStrProperty>(Prop))
		{
			Json->SetStringField(Name, StrProp->GetPropertyValue(ValuePtr));
		}
		else if (const FNameProperty* NameProp = CastField<FNameProperty>(Prop))
		{
			Json->SetStringField(Name, NameProp->GetPropertyValue(ValuePtr).ToString());
		}
		else if (const FTextProperty* TextProp = CastField<FTextProperty>(Prop))
		{
			Json->SetStringField(Name, TextProp->GetPropertyValue(ValuePtr).ToString());
		}
		else if (const FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
		{
			Json->SetNumberField(Name, static_cast<double>(EnumProp->GetUnderlyingProperty()->GetSignedIntPropertyValue(ValuePtr)));
		}
		else if (const FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
		{
			Json->SetNumberField(Name, static_cast<double>(ByteProp->GetPropertyValue(ValuePtr)));
		}
	}

	void ExportStructProperties(UStruct* Struct, void* Container, TSharedRef<FJsonObject> Json)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* Prop = *It;
			if (ShouldSkipProperty(Prop))
			{
				continue;
			}
			ExportProperty(Prop, Prop->ContainerPtrToValuePtr<void>(Container), Json);
		}
	}

	void ImportProperty(FProperty* Prop, void* ValuePtr, UObject* Outer, TSharedPtr<FJsonValue> Value)
	{
		if (FArrayProperty* ArrayProp = CastField<FArrayProperty>(Prop))
		{
			const TArray<TSharedPtr<FJsonValue>>* Arr = nullptr;
			if (Value == nullptr || !Value->TryGetArray(Arr))
			{
				return;
			}

			FProperty* Inner = ArrayProp->Inner;
			FScriptArrayHelper Helper(ArrayProp, ValuePtr);

			if (FObjectProperty* InnerObj = CastField<FObjectProperty>(Inner))
			{
				for (const TSharedPtr<FJsonValue>& Elem : *Arr)
				{
					TSharedPtr<FJsonObject> SubJson = Elem->AsObject();
					if (!SubJson.IsValid())
					{
						continue;
					}

					UClass* SubClass = FindClassByName(SubJson->GetStringField(TEXT("class")));
					if (SubClass == nullptr || !SubClass->IsChildOf(InnerObj->PropertyClass))
					{
						continue;
					}

					const int32 Index = Helper.AddValue();
					UObject* SubObj = NewObject<UObject>(Outer, SubClass, NAME_None, RF_Transactional);
					InnerObj->SetObjectPropertyValue(Helper.GetRawPtr(Index), SubObj);

					TSharedPtr<FJsonObject> PropsJson = SubJson->GetObjectField(TEXT("properties"));
					if (PropsJson.IsValid())
					{
						ImportStructProperties(SubClass, SubObj, SubObj, PropsJson);
					}
				}
			}
			else if (FStructProperty* InnerStruct = CastField<FStructProperty>(Inner))
			{
				for (const TSharedPtr<FJsonValue>& Elem : *Arr)
				{
					TSharedPtr<FJsonObject> SubJson = Elem->AsObject();
					if (!SubJson.IsValid())
					{
						continue;
					}
					const int32 Index = Helper.AddValue();
					ImportStructProperties(InnerStruct->Struct, Helper.GetRawPtr(Index), Outer, SubJson);
				}
			}
			return;
		}

		if (FStructProperty* StructProp = CastField<FStructProperty>(Prop))
		{
			TSharedPtr<FJsonObject> SubJson = Value != nullptr ? Value->AsObject() : nullptr;
			if (SubJson.IsValid())
			{
				ImportStructProperties(StructProp->Struct, ValuePtr, Outer, SubJson);
			}
			return;
		}

		if (Value == nullptr)
		{
			return;
		}

		if (FIntProperty* IntProp = CastField<FIntProperty>(Prop))
		{
			IntProp->SetPropertyValue(ValuePtr, static_cast<int32>(Value->AsNumber()));
		}
		else if (FInt64Property* Int64Prop = CastField<FInt64Property>(Prop))
		{
			Int64Prop->SetPropertyValue(ValuePtr, static_cast<int64>(Value->AsNumber()));
		}
		else if (FFloatProperty* FloatProp = CastField<FFloatProperty>(Prop))
		{
			FloatProp->SetPropertyValue(ValuePtr, static_cast<float>(Value->AsNumber()));
		}
		else if (FDoubleProperty* DoubleProp = CastField<FDoubleProperty>(Prop))
		{
			DoubleProp->SetPropertyValue(ValuePtr, Value->AsNumber());
		}
		else if (FBoolProperty* BoolProp = CastField<FBoolProperty>(Prop))
		{
			BoolProp->SetPropertyValue(ValuePtr, Value->AsBool());
		}
		else if (FStrProperty* StrProp = CastField<FStrProperty>(Prop))
		{
			StrProp->SetPropertyValue(ValuePtr, Value->AsString());
		}
		else if (FNameProperty* NameProp = CastField<FNameProperty>(Prop))
		{
			NameProp->SetPropertyValue(ValuePtr, FName(*Value->AsString()));
		}
		else if (FTextProperty* TextProp = CastField<FTextProperty>(Prop))
		{
			TextProp->SetPropertyValue(ValuePtr, FText::FromString(Value->AsString()));
		}
		else if (FEnumProperty* EnumProp = CastField<FEnumProperty>(Prop))
		{
			EnumProp->GetUnderlyingProperty()->SetIntPropertyValue(ValuePtr, static_cast<int64>(Value->AsNumber()));
		}
		else if (FByteProperty* ByteProp = CastField<FByteProperty>(Prop))
		{
			ByteProp->SetPropertyValue(ValuePtr, static_cast<uint8>(Value->AsNumber()));
		}
	}

	void ImportStructProperties(UStruct* Struct, void* Container, UObject* Outer, TSharedPtr<FJsonObject> Json)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* Prop = *It;
			if (ShouldSkipProperty(Prop))
			{
				continue;
			}

			TSharedPtr<FJsonValue> Value = Json->TryGetField(Prop->GetName());
			ImportProperty(Prop, Prop->ContainerPtrToValuePtr<void>(Container), Outer, Value);
		}
	}
}

bool TaskFlowGraphSerializer::ExportGraphToJson(UTaskFlowGraph* Graph, FString& OutJson)
{
	if (Graph == nullptr)
	{
		return false;
	}

	TSharedRef<FJsonObject> Root = MakeShareable(new FJsonObject());
	Root->SetNumberField(TEXT("version"), 1);

	TArray<TSharedPtr<FJsonValue>> NodesArr;
	TArray<TSharedPtr<FJsonValue>> LinksArr;
	TMap<UEdGraphNode*, int32> NodeToId;

	int32 NodeId = 0;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node == nullptr)
		{
			continue;
		}

		NodeToId.Add(Node, NodeId);

		TSharedRef<FJsonObject> NodeJson = MakeShareable(new FJsonObject());
		NodeJson->SetNumberField(TEXT("id"), NodeId);
		NodeJson->SetStringField(TEXT("class"), Node->GetClass()->GetName());
		NodeJson->SetNumberField(TEXT("x"), Node->NodePosX);
		NodeJson->SetNumberField(TEXT("y"), Node->NodePosY);

		TSharedRef<FJsonObject> PropsJson = MakeShareable(new FJsonObject());
		ExportStructProperties(Node->GetClass(), Node, PropsJson);
		NodeJson->SetObjectField(TEXT("properties"), PropsJson);

		NodesArr.Add(MakeShareable(new FJsonValueObject(NodeJson)));
		++NodeId;
	}

	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node == nullptr)
		{
			continue;
		}

		const int32* FromIdPtr = NodeToId.Find(Node);
		if (FromIdPtr == nullptr)
		{
			continue;
		}

		for (UEdGraphPin* Pin : Node->Pins)
		{
			if (Pin == nullptr || Pin->Direction != EGPD_Output)
			{
				continue;
			}

			for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (LinkedPin == nullptr || LinkedPin->GetOwningNode() == nullptr)
				{
					continue;
				}

				const int32* ToIdPtr = NodeToId.Find(LinkedPin->GetOwningNode());
				if (ToIdPtr == nullptr)
				{
					continue;
				}

				TSharedRef<FJsonObject> LinkJson = MakeShareable(new FJsonObject());
				LinkJson->SetNumberField(TEXT("fromNode"), *FromIdPtr);
				LinkJson->SetStringField(TEXT("fromPin"), Pin->PinName.ToString());
				LinkJson->SetNumberField(TEXT("toNode"), *ToIdPtr);
				LinkJson->SetStringField(TEXT("toPin"), LinkedPin->PinName.ToString());
				LinksArr.Add(MakeShareable(new FJsonValueObject(LinkJson)));
			}
		}
	}

	Root->SetArrayField(TEXT("nodes"), NodesArr);
	Root->SetArrayField(TEXT("links"), LinksArr);

	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutJson);
	return FJsonSerializer::Serialize(Root, Writer);
}

bool TaskFlowGraphSerializer::ImportGraphFromJson(UTaskFlowGraph* Graph, const FString& InJson, FString& OutError)
{
	if (Graph == nullptr)
	{
		OutError = TEXT("Graph is null");
		return false;
	}

	TSharedPtr<FJsonObject> Root;
	TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(InJson);
	if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
	{
		OutError = TEXT("Failed to parse Json");
		return false;
	}

	// Clear existing nodes.
	TArray<UEdGraphNode*> NodesToDestroy;
	for (UEdGraphNode* Node : Graph->Nodes)
	{
		if (Node != nullptr)
		{
			NodesToDestroy.Add(Node);
		}
	}
	for (UEdGraphNode* Node : NodesToDestroy)
	{
		Node->DestroyNode();
	}
	Graph->Nodes.Reset();

	// First pass: create nodes.
	TMap<int32, UEdGraphNode*> IdToNode;
	const TArray<TSharedPtr<FJsonValue>>* NodesArr = nullptr;
	if (Root->TryGetArrayField(TEXT("nodes"), NodesArr))
	{
		for (const TSharedPtr<FJsonValue>& Elem : *NodesArr)
		{
			TSharedPtr<FJsonObject> NodeJson = Elem->AsObject();
			if (!NodeJson.IsValid())
			{
				continue;
			}

			const int32 Id = NodeJson->GetIntegerField(TEXT("id"));
			const FString ClassName = NodeJson->GetStringField(TEXT("class"));

			UClass* NodeClass = FindClassByName(ClassName);
			if (NodeClass == nullptr || !NodeClass->IsChildOf(UTaskFlowNode::StaticClass()))
			{
				OutError = FString::Printf(TEXT("Unknown node class: %s"), *ClassName);
				continue;
			}

			UEdGraphNode* Node = NewObject<UEdGraphNode>(Graph, NodeClass, NAME_None, RF_Transactional);
			Node->NodePosX = NodeJson->GetIntegerField(TEXT("x"));
			Node->NodePosY = NodeJson->GetIntegerField(TEXT("y"));

			TSharedPtr<FJsonObject> PropsJson = NodeJson->GetObjectField(TEXT("properties"));
			if (PropsJson.IsValid())
			{
				ImportStructProperties(NodeClass, Node, Node, PropsJson);
			}

			Node->CreateNewGuid();
			Node->AllocateDefaultPins();
			Graph->AddNode(Node);

			IdToNode.Add(Id, Node);
		}
	}

	// Second pass: build connections.
	const TArray<TSharedPtr<FJsonValue>>* LinksArr = nullptr;
	if (Root->TryGetArrayField(TEXT("links"), LinksArr))
	{
		for (const TSharedPtr<FJsonValue>& Elem : *LinksArr)
		{
			TSharedPtr<FJsonObject> LinkJson = Elem->AsObject();
			if (!LinkJson.IsValid())
			{
				continue;
			}

			const int32 FromId = LinkJson->GetIntegerField(TEXT("fromNode"));
			const int32 ToId = LinkJson->GetIntegerField(TEXT("toNode"));
			const FString FromPin = LinkJson->GetStringField(TEXT("fromPin"));
			const FString ToPin = LinkJson->GetStringField(TEXT("toPin"));

			UEdGraphNode** FromNodePtr = IdToNode.Find(FromId);
			UEdGraphNode** ToNodePtr = IdToNode.Find(ToId);
			if (FromNodePtr == nullptr || ToNodePtr == nullptr)
			{
				continue;
			}

			UEdGraphPin* From = (*FromNodePtr)->FindPin(FName(*FromPin), EGPD_Output);
			UEdGraphPin* To = (*ToNodePtr)->FindPin(FName(*ToPin), EGPD_Input);
			if (From != nullptr && To != nullptr)
			{
				From->MakeLinkTo(To);
			}
		}
	}

	return true;
}
