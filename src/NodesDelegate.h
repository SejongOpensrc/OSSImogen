// https://github.com/CedricGuillemet/Imogen
//
// The MIT License(MIT)
// 
// Copyright(c) 2018 Cedric Guillemet
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include "Nodes.h"
#include "Evaluation.h"
#include "curve.h"
#include "Library.h"
#include "nfd.h"

// Ÿ�� ��� ������ ��ǥ ( DB�� xml��� ��� �� ��忡 ���� ������ �̷��� ���������� ǥ���ص� )
struct TileNodeEditGraphDelegate : public NodeGraphDelegate
{
	TileNodeEditGraphDelegate(Evaluation& evaluation) : mEvaluation(evaluation), mbMouseDragging(false)
	{
		mCategoriesCount = 9;
		static const char *categories[] = {
			"Transform", // ����
			"Generator", // ���׷�����
			"Material", // ���׸���
			"Blend", // ���� (�ͽ�)
			"Filter", // ���� �����, ���͸�
			"Noise", // ����, ������
			"File", // ����
			"Paint", // Draw, ����Ʈ
			"Cubemap",
			"AddedOne" // hw, test value�� �ϳ� �߰��غ�
		}; 
		mCategories = categories;
		assert(!mInstance);
		mInstance = this;
	}

	Evaluation& mEvaluation;
	struct ImogenNode
	{
		size_t mType;
		size_t mEvaluationTarget;
		void *mParameters;
		size_t mParametersSize;
		unsigned int mRuntimeUniqueId;
		std::vector<InputSampler> mInputSamplers;
	};

	std::vector<ImogenNode> mNodes;

	void Clear()
	{
		mSelectedNodeIndex = -1;
		mNodes.clear();
	}

	virtual unsigned char *GetParamBlock(size_t index, size_t& paramBlockSize)
	{
		const ImogenNode & node = mNodes[index];
		paramBlockSize = ComputeNodeParametersSize(node.mType);
		return (unsigned char*)node.mParameters;
	}
	virtual void SetParamBlock(size_t index, unsigned char* parameters)
	{
		const ImogenNode & node = mNodes[index];
		memcpy(node.mParameters, parameters, ComputeNodeParametersSize(node.mType));
		mEvaluation.SetEvaluationParameters(node.mEvaluationTarget, parameters, node.mParametersSize);
		mEvaluation.SetEvaluationSampler(node.mEvaluationTarget, node.mInputSamplers);
	}

	virtual bool AuthorizeConnexion(int typeA, int typeB)
	{
		return true;
	}

	virtual unsigned int GetNodeTexture(size_t index)
	{
		return mEvaluation.GetEvaluationTexture(mNodes[index].mEvaluationTarget);
	}
	virtual void AddNode(size_t type)
	{
		size_t index = mNodes.size();
		ImogenNode node;
		node.mEvaluationTarget = mEvaluation.AddEvaluation(type, gMetaNodes[type].mName);
		node.mRuntimeUniqueId = GetRuntimeId();
		node.mType = type;
		size_t paramsSize = ComputeNodeParametersSize(type);
		node.mParameters = malloc(paramsSize);
		node.mParametersSize = paramsSize;
		memset(node.mParameters, 0, paramsSize);
		size_t inputCount = gMetaNodes[type].mInputs.size();
		node.mInputSamplers.resize(inputCount);
		mNodes.push_back(node);

		mEvaluation.SetEvaluationParameters(node.mEvaluationTarget, node.mParameters, node.mParametersSize);
		mEvaluation.SetEvaluationSampler(node.mEvaluationTarget, node.mInputSamplers);
	}

	void AddLink(int InputIdx, int InputSlot, int OutputIdx, int OutputSlot)
	{
		mEvaluation.AddEvaluationInput(OutputIdx, OutputSlot, InputIdx);
	}

	virtual void DelLink(int index, int slot)
	{
		mEvaluation.DelEvaluationInput(index, slot);
	}

	virtual void DeleteNode(size_t index)
	{
		mEvaluation.DelEvaluationTarget(index);
		free(mNodes[index].mParameters);
		mNodes.erase(mNodes.begin() + index);
		for (auto& node : mNodes)
		{
			if (node.mEvaluationTarget > index)
				node.mEvaluationTarget--;
		}
	}
	
	const float PI = 3.14159f;
	float RadToDeg(float a) { return a * 180.f / PI; }
	float DegToRad(float a) { return a / 180.f * PI; }
	void EditNode()
	{
		size_t index = mSelectedNodeIndex;

		const MetaNode* metaNodes = gMetaNodes.data();
		bool dirty = false;
		bool forceEval = false;
		bool samplerDirty = false;
		ImogenNode& node = mNodes[index];
		const MetaNode& currentMeta = metaNodes[node.mType];
		
		if (ImGui::CollapsingHeader("Samplers", 0))
		{
			for (size_t i = 0; i < node.mInputSamplers.size();i++)
			{
				InputSampler& inputSampler = node.mInputSamplers[i];
				static const char *wrapModes = { "REPEAT\0CLAMP_TO_EDGE\0CLAMP_TO_BORDER\0MIRRORED_REPEAT" };
				static const char *filterModes = { "LINEAR\0NEAREST" };
				ImGui::PushItemWidth(150);
				ImGui::Text("Sampler %d", i);
				samplerDirty |= ImGui::Combo("Wrap U", (int*)&inputSampler.mWrapU, wrapModes);
				samplerDirty |= ImGui::Combo("Wrap V", (int*)&inputSampler.mWrapV, wrapModes);
				samplerDirty |= ImGui::Combo("Filter Min", (int*)&inputSampler.mFilterMin, filterModes);
				samplerDirty |= ImGui::Combo("Filter Mag", (int*)&inputSampler.mFilterMag, filterModes);
				ImGui::PopItemWidth();
			}
			if (samplerDirty)
			{
				mEvaluation.SetEvaluationSampler(node.mEvaluationTarget, node.mInputSamplers);
			}

		}
		if (!ImGui::CollapsingHeader(currentMeta.mName.c_str(), 0, ImGuiTreeNodeFlags_DefaultOpen))
			return;

		unsigned char *paramBuffer = (unsigned char*)node.mParameters;
		int i = 0;
		for(const MetaParameter& param : currentMeta.mParams)
		{
			ImGui::PushID(667889 + i++);
			switch (param.mType)
			{
			case Con_Float:
				dirty |= ImGui::InputFloat(param.mName.c_str(), (float*)paramBuffer);
				break;
			case Con_Float2:
				dirty |= ImGui::InputFloat2(param.mName.c_str(), (float*)paramBuffer);
				break;
			case Con_Float3:
				dirty |= ImGui::InputFloat3(param.mName.c_str(), (float*)paramBuffer);
				break;
			case Con_Float4:
				dirty |= ImGui::InputFloat4(param.mName.c_str(), (float*)paramBuffer);
				break;
			case Con_Color4:
				dirty |= ImGui::ColorPicker4(param.mName.c_str(), (float*)paramBuffer);
				break;
			case Con_Int:
				dirty |= ImGui::InputInt(param.mName.c_str(), (int*)paramBuffer);
				break;
			case Con_Ramp:
				{
					ImVec2 points[8];
					
					for (int k = 0; k < 8; k++)
					{
						points[k] = ImVec2(((float*)paramBuffer)[k * 2], ((float*)paramBuffer)[k * 2 + 1]);
						if (k && points[k - 1].x > points[k].x)
							points[k] = ImVec2(1.f, 1.f);
					}

					if (ImGui::Curve("Ramp", ImVec2(250, 150), 8, points))
					{
						for (int k = 0; k < 8; k++)
						{
							((float*)paramBuffer)[k * 2] = points[k].x;
							((float*)paramBuffer)[k * 2 + 1] = points[k].y;
						}
						dirty = true;
					}
				}
				break;
			case Con_Angle:
				((float*)paramBuffer)[0] = RadToDeg(((float*)paramBuffer)[0]);
				dirty |= ImGui::InputFloat(param.mName.c_str(), (float*)paramBuffer);
				((float*)paramBuffer)[0] = DegToRad(((float*)paramBuffer)[0]);
				break;
			case Con_Angle2:
				((float*)paramBuffer)[0] = RadToDeg(((float*)paramBuffer)[0]);
				((float*)paramBuffer)[1] = RadToDeg(((float*)paramBuffer)[1]);
				dirty |= ImGui::InputFloat2(param.mName.c_str(), (float*)paramBuffer);
				((float*)paramBuffer)[0] = DegToRad(((float*)paramBuffer)[0]);
				((float*)paramBuffer)[1] = DegToRad(((float*)paramBuffer)[1]);
				break;
			case Con_Angle3:
				((float*)paramBuffer)[0] = RadToDeg(((float*)paramBuffer)[0]);
				((float*)paramBuffer)[1] = RadToDeg(((float*)paramBuffer)[1]);
				((float*)paramBuffer)[2] = RadToDeg(((float*)paramBuffer)[2]);
				dirty |= ImGui::InputFloat3(param.mName.c_str(), (float*)paramBuffer);
				((float*)paramBuffer)[0] = DegToRad(((float*)paramBuffer)[0]);
				((float*)paramBuffer)[1] = DegToRad(((float*)paramBuffer)[1]);
				((float*)paramBuffer)[2] = DegToRad(((float*)paramBuffer)[2]);
				break;
			case Con_Angle4:
				((float*)paramBuffer)[0] = RadToDeg(((float*)paramBuffer)[0]);
				((float*)paramBuffer)[1] = RadToDeg(((float*)paramBuffer)[1]);
				((float*)paramBuffer)[2] = RadToDeg(((float*)paramBuffer)[2]);
				((float*)paramBuffer)[3] = RadToDeg(((float*)paramBuffer)[3]);
				dirty |= ImGui::InputFloat4(param.mName.c_str(), (float*)paramBuffer);
				((float*)paramBuffer)[0] = DegToRad(((float*)paramBuffer)[0]);
				((float*)paramBuffer)[1] = DegToRad(((float*)paramBuffer)[1]);
				((float*)paramBuffer)[2] = DegToRad(((float*)paramBuffer)[2]);
				((float*)paramBuffer)[3] = DegToRad(((float*)paramBuffer)[3]);
				break;
			case Con_FilenameWrite:
			case Con_FilenameRead:
				dirty |= ImGui::InputText("", (char*)paramBuffer, 1024);
				ImGui::SameLine();
				if (ImGui::Button("..."))
				{
					nfdchar_t *outPath = NULL;
					nfdresult_t result = (param.mType == Con_FilenameRead) ? NFD_OpenDialog(NULL, NULL, &outPath) : NFD_SaveDialog(NULL, NULL, &outPath);

					if (result == NFD_OKAY) 
					{
						strcpy((char*)paramBuffer, outPath);
						free(outPath);
						dirty = true;
					}
				}
				ImGui::SameLine();
				ImGui::Text(param.mName.c_str());
				break;
			case Con_Enum:
				dirty |= ImGui::Combo(param.mName.c_str(), (int*)paramBuffer, param.mEnumList);
				break;
			case Con_ForceEvaluate:
				if (ImGui::Button(param.mName.c_str()))
				{
					dirty = true;
					forceEval = true;
				}
				break;
			case Con_Bool:
			{
				bool checked = (*(int*)paramBuffer) != 0;
				if (ImGui::Checkbox(param.mName.c_str(), &checked))
				{
					*(int*)paramBuffer = checked ? 1 : 0;
					dirty = true;
				}
			}
			break;
			}
			ImGui::PopID();
			paramBuffer += GetParameterTypeSize(param.mType);
		}
		
		if (dirty)
			mEvaluation.SetEvaluationParameters(node.mEvaluationTarget, node.mParameters, node.mParametersSize);
		if (forceEval)
		{
			EvaluationInfo evaluationInfo;
			evaluationInfo.forcedDirty = 1;
			evaluationInfo.uiPass = 0;
			mEvaluation.PerformEvaluationForNode(node.mEvaluationTarget, 256, 256, true, evaluationInfo);
		}
	}

	virtual void DoForce()
	{
		bool dirty = false;
		bool forceEval = false;
		for (ImogenNode& node : mNodes)
		{
			const MetaNode& currentMeta = gMetaNodes[node.mType];

			for(auto& param : currentMeta.mParams)
			{
				if (!param.mName.c_str())
					break;
				if (param.mType == Con_ForceEvaluate)
				{
					dirty = true;
					forceEval = true;
				}
			}
			if (forceEval)
			{
				EvaluationInfo evaluationInfo;
				evaluationInfo.forcedDirty = 1;
				evaluationInfo.uiPass = 0;
				mEvaluation.PerformEvaluationForNode(node.mEvaluationTarget, 256, 256, true, evaluationInfo);
			}
		}
	}

	void InvalidateParameters()
	{
		for (auto& node : mNodes)
			mEvaluation.SetEvaluationParameters(node.mEvaluationTarget, node.mParameters, node.mParametersSize);
	}

	template<typename T> static inline T nmin(T lhs, T rhs) { return lhs >= rhs ? rhs : lhs; }

	bool mbMouseDragging;
	void SetMouse(float rx, float ry, float dx, float dy, bool lButDown, bool rButDown)
	{
		if (mSelectedNodeIndex == -1)
			return;

		if (!lButDown)
			mbMouseDragging = false;

		const MetaNode* metaNodes = gMetaNodes.data();
		size_t res = 0;
		const MetaNode& metaNode = metaNodes[mNodes[mSelectedNodeIndex].mType];
		unsigned char *paramBuffer = (unsigned char*)mNodes[mSelectedNodeIndex].mParameters;
		bool parametersUseMouse = false;
		if (lButDown)
		{
			float *paramFlt = (float*)paramBuffer;
			for(auto& param : metaNode.mParams)
			{
				if (param.mbQuadSelect && param.mType == Con_Float4)
				{
					if (!mbMouseDragging)
					{
						paramFlt[2] = paramFlt[0] = rx;
						paramFlt[3] = paramFlt[1] = 1.f - ry;
						mbMouseDragging = true;
					}
					else
					{
						paramFlt[2] = rx;
						paramFlt[3] = 1.f - ry;
					}
					continue;
				}

				if (param.mRangeMinX != 0.f || param.mRangeMaxX != 0.f)
				{
					if (param.mbRelative)
					{
						paramFlt[0] += ImLerp(param.mRangeMinX, param.mRangeMaxX, dx);
						paramFlt[0] = fmodf(paramFlt[0], fabsf(param.mRangeMaxX - param.mRangeMinX)) + nmin(param.mRangeMinX, param.mRangeMaxX);
					}
					else
					{
						paramFlt[0] = ImLerp(param.mRangeMinX, param.mRangeMaxX, rx);
					}
				}
				if (param.mRangeMinY != 0.f || param.mRangeMaxY != 0.f)
				{
					if (param.mbRelative)
					{
						paramFlt[1] += ImLerp(param.mRangeMinY, param.mRangeMaxY, dy);
						paramFlt[1] = fmodf(paramFlt[1], fabsf(param.mRangeMaxY - param.mRangeMinY)) + nmin(param.mRangeMinY, param.mRangeMaxY);
					}
					else
					{
						paramFlt[1] = ImLerp(param.mRangeMinY, param.mRangeMaxY, ry);
					}
				}
				paramBuffer += GetParameterTypeSize(param.mType);
				parametersUseMouse = true;
			}
		}
		if (metaNode.mbHasUI || parametersUseMouse)
		{
			mEvaluation.SetMouse(mSelectedNodeIndex, rx, ry, lButDown, rButDown);
			mEvaluation.SetEvaluationParameters(mNodes[mSelectedNodeIndex].mEvaluationTarget, mNodes[mSelectedNodeIndex].mParameters, mNodes[mSelectedNodeIndex].mParametersSize);
		}
	}

	size_t ComputeNodeParametersSize(size_t nodeTypeIndex)
	{
		size_t res = 0;
		for(auto& param : gMetaNodes[nodeTypeIndex].mParams)
		{
			res += GetParameterTypeSize(param.mType);
		}
		return res;
	}
	bool NodeHasUI(size_t nodeIndex)
	{
		return gMetaNodes[mNodes[nodeIndex].mType].mbHasUI;
	}
	virtual bool NodeIsProcesing(size_t nodeIndex)
	{
		return mEvaluation.StageIsProcessing(nodeIndex);
	}
	virtual bool NodeIsCubemap(size_t nodeIndex)
	{
		RenderTarget *target = mEvaluation.GetRenderTarget(nodeIndex);
		if (target)
			return target->mImage.mNumFaces == 6;
		return false;
	}

	virtual void UpdateEvaluationList(const std::vector<size_t> nodeOrderList)
	{
		mEvaluation.SetEvaluationOrder(nodeOrderList);
	}

	virtual ImVec2 GetEvaluationSize(size_t nodeIndex)
	{
		int imageWidth(1), imageHeight(1);
		mEvaluation.GetEvaluationSize(int(nodeIndex), &imageWidth, &imageHeight);
		return ImVec2(float(imageWidth), float(imageHeight));
	}
	static TileNodeEditGraphDelegate *GetInstance() { return mInstance; }
	ImogenNode* Get(ASyncId id) { return GetByAsyncId(id, mNodes); }
protected:
	static TileNodeEditGraphDelegate *mInstance;
};

