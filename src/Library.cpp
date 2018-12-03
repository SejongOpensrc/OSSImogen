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




#include "Library.h"


// 중요 참고 사항 : Ser - 데이터를 파일에 쓰거나 읽는 작업 수행


enum : uint32_t  // 열거형 정의
{
	v_initial,          // 버전
	v_materialComment,  // material에 대한 설명
	v_lastVersion       // 마지막 버전
};
#define ADD(_fieldAdded, _fieldName) if (dataVersion >= _fieldAdded){ Ser(_fieldName); }         // 정상적인 버전인 경우, 해당 field에 대해 Ser작업 수행 
#define ADD_LOCAL(_localAdded, _type, _localName, _defaultValue) \                                   
_type _localName = (_defaultValue); \                                                        // localName을 초기화하고
if (dataVersion >= (_localAdded)) { Ser(_localName)); }                                      // 정상적인 버전인 경우, 해당 local에 대해 Ser작업 수행
#define REM(_fieldAdded, _fieldRemoved, _type, _fieldName, _defaultValue) \
	_type _fieldName = (_defaultValue); \                                                        // localName을 초기화하고
if (dataVersion >= (_fieldAdded) && dataVersion < (_fieldRemoved)) { Ser(_fieldName); }      // 정상적인 버전인 경우, 해당 field에 대해 Ser작업 수행
#define VERSION_IN_RANGE(_from, _to) \                                                           // 버전이 from과 to 사이에 있는지에 대하여 반환 
(dataVersion >= (_from) && dataVersion < (_to))

	template<bool doWrite> struct Serialize   // 클래스처럼 구현된 구조체 (C++ 방식)
{
	Serialize(const char *szFilename)  // 생성자
	{
		fp = fopen(szFilename, doWrite ? "wb" : "rb");  // 쓰기모드 or 읽기모드로 파일을 open
	}
	~Serialize()                      // 소멸자
	{
		if (fp)
			fclose(fp);                                 // 파일 close
	}
	template<typename T> void Ser(T& data)  // 제네릭 데이터											
	{
		if (doWrite)  // 쓰기모드
			fwrite(&data, sizeof(T), 1, fp);  // data를 fp에 씀
		else          // 읽기모드
			fread(&data, sizeof(T), 1, fp);   // fp에서 data로 읽어들임
	}
	void Ser(std::string& data)  // 스트링 데이터
	{
		if (doWrite)
		{
			uint32_t len = uint32_t(data.length() + 1);
			fwrite(&len, sizeof(uint32_t), 1, fp);   // 데이터의 길이(len)를 fp에 쓰고
			fwrite(data.c_str(), len, 1, fp);        // data의 string도 fp에 씀
		}
		else
		{
			uint32_t len;
			fread(&len, sizeof(uint32_t), 1, fp);   // 데이터의 길이를 읽어들이고 
			data.resize(len);                       // 길이에 맞게 데이터를 리사이징하여
			fread(&data[0], len, 1, fp);            // 데이터에 스트링을 읽어들임
		}
	}
	template<typename T> void Ser(std::vector<T>& data)  // 제네릭 벡터 데이터
	{
		uint32_t count = uint32_t(data.size());  // 데이터의 길이 읽어들이고
		Ser(count);                              // 길이(count)에 대하여 Ser 작업을 수행 후
		data.resize(count);                      // 길이에 맞게 데이터를 리사이징하여
		for (auto& item : data)                  // data 컬렉션에 대해 
			Ser(&item);                          // 모두 Ser 작업을 해줌
	}
	void Ser(std::vector<uint8_t>& data)  // int형 벡터 데이터 (반환 없음)
	{
		uint32_t count = uint32_t(data.size());
		Ser(count);
		if (!count)                                 // count = 0 이면 끝
			return;
		if (doWrite)
		{
			fwrite(data.data(), count, 1, fp);      // 길이만큼 data를 fp에 씀
		}
		else
		{
			data.resize(count);
			fread(&data[0], count, 1, fp);          // 길이만큼 fp에서 data로 읽어들임
		}
	}

	void Ser(InputSampler *inputSampler)               // inputSampler(샘플링 작업이 수행되는 텍스쳐)의 각 요소에 대해 Ser작업 수행
	{
		ADD(v_initial, inputSampler->mWrapU);         // U축에 대한 래핑
		ADD(v_initial, inputSampler->mWrapV);         // V축에 대한 래핑
		ADD(v_initial, inputSampler->mFilterMin);     // 축소 필터
		ADD(v_initial, inputSampler->mFilterMag);     // 확대 필터
	}
	void Ser(MaterialNode *materialNode)               // materialNode(텍스쳐를 포함하고 있는 노드)의 각 요소에 대해 Ser작업 수행
	{
		ADD(v_initial, materialNode->mType);            // 노드타입
		ADD(v_initial, materialNode->mPosX);            // 노드의 x 위치
		ADD(v_initial, materialNode->mPosY);            // 노드의 y 위치
		ADD(v_initial, materialNode->mInputSamplers);   // 노드가 포함하고 있는 inputSampler
		ADD(v_initial, materialNode->mParameters);      // 패러미터 값들
	}
	void Ser(MaterialConnection *materialConnection)   // materialConnection(노드의 연결들)의 각 요소에 대해 Ser작업 수행
	{
		ADD(v_initial, materialConnection->mInputNode);   // 나에게 연결되는 노드
		ADD(v_initial, materialConnection->mOutputNode);  // 나에게서 연결된 노드
		ADD(v_initial, materialConnection->mInputSlot);   // 연결될 수 있는 노드들의 슬롯
		ADD(v_initial, materialConnection->mOutputSlot);  // 연결할 수 있는 노드들의 슬롯
	}
	void Ser(Material *material)                       // material(작업이 수행되는 영역)의 각 요소에 대해 Ser작업 수행
	{
		ADD(v_initial, material->mName);                  // material의 이름
		ADD(v_materialComment, material->mComment);       // material에 대한 설명
		ADD(v_initial, material->mMaterialNodes);         // material의 노드들
		ADD(v_initial, material->mMaterialConnections);   // material의 연결들
	}
	bool Ser(Library *library)   // 라이브러리
	{
		if (!fp)  // 파일포인터가 없는 경우 종료
			return false;
		if (doWrite)
			dataVersion = v_lastVersion - 1;  // 쓰기모드인 경우 마지막 버전의 바로 이전버전을 데이터 버전으로 설정
		Ser(dataVersion);                   // 데이터 버전에 대해 Ser 작업 수행
		if (dataVersion > v_lastVersion)    // 올바르지 않은 버전인 경우 종료
			return false; // no forward compatibility
		ADD(v_initial, library->mMaterials);// material에 대해 Ser 작업 수행
		return true;
	}
	FILE *fp;
	uint32_t dataVersion;
};

typedef Serialize<true> SerializeWrite;  // 쓰기모드
typedef Serialize<false> SerializeRead;  // 읽기모드

void LoadLib(Library *library, const char *szFilename)  // 라이브러리 불러오기
{
	SerializeRead(szFilename).Ser(library);
}

void SaveLib(Library *library, const char *szFilename)  // 라이브러리 저장
{
	SerializeWrite(szFilename).Ser(library);
}