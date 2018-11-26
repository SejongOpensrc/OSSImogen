#include "Library.h"

  
// �߿� ���� ���� : Ser - �����͸� ���Ͽ� ���ų� �д� �۾� ����


enum : uint32_t  // ������ ����
{
	v_initial,          // ����
	v_materialComment,  // material�� ���� ����
	v_lastVersion       // ������ ����
};
#define ADD(_fieldAdded, _fieldName) if (dataVersion >= _fieldAdded){ Ser(_fieldName); }         // �������� ������ ���, �ش� field�� ���� Ser�۾� ���� 
#define ADD_LOCAL(_localAdded, _type, _localName, _defaultValue) \                                   
	_type _localName = (_defaultValue); \                                                        // localName�� �ʱ�ȭ�ϰ�
	if (dataVersion >= (_localAdded)) { Ser(_localName)); }                                      // �������� ������ ���, �ش� local�� ���� Ser�۾� ����
#define REM(_fieldAdded, _fieldRemoved, _type, _fieldName, _defaultValue) \
	_type _fieldName = (_defaultValue); \                                                        // localName�� �ʱ�ȭ�ϰ�
	if (dataVersion >= (_fieldAdded) && dataVersion < (_fieldRemoved)) { Ser(_fieldName); }      // �������� ������ ���, �ش� field�� ���� Ser�۾� ����
#define VERSION_IN_RANGE(_from, _to) \                                                           // ������ from�� to ���̿� �ִ����� ���Ͽ� ��ȯ 
	(dataVersion >= (_from) && dataVersion < (_to))

template<bool doWrite> struct Serialize   // Ŭ����ó�� ������ ����ü (C++ ���)
{
	Serialize(const char *szFilename)  // ������
	{
		fp = fopen(szFilename, doWrite ? "wb" : "rb");  // ������ or �б���� ������ open
	}
	~Serialize()                      // �Ҹ���
	{
		if (fp)
			fclose(fp);                                 // ���� close
	}
	template<typename T> void Ser(T& data)  // ���׸� ������											
	{
		if (doWrite)  // ������
			fwrite(&data, sizeof(T), 1, fp);  // data�� fp�� ��
		else          // �б���
			fread(&data, sizeof(T), 1, fp);   // fp���� data�� �о����
	}
	void Ser(std::string& data)  // ��Ʈ�� ������
	{
		if (doWrite)
		{
			uint32_t len = uint32_t(data.length() + 1);
			fwrite(&len, sizeof(uint32_t), 1, fp);   // �������� ����(len)�� fp�� ����
			fwrite(data.c_str(), len, 1, fp);        // data�� string�� fp�� ��
		}
		else
		{
			uint32_t len;
			fread(&len, sizeof(uint32_t), 1, fp);   // �������� ���̸� �о���̰� 
			data.resize(len);                       // ���̿� �°� �����͸� ������¡�Ͽ�
			fread(&data[0], len, 1, fp);            // �����Ϳ� ��Ʈ���� �о����
		}
	}
	template<typename T> void Ser(std::vector<T>& data)  // ���׸� ���� ������
	{
		uint32_t count = uint32_t(data.size());  // �������� ���� �о���̰�
		Ser(count);                              // ����(count)�� ���Ͽ� Ser �۾��� ���� ��
		data.resize(count);                      // ���̿� �°� �����͸� ������¡�Ͽ�
		for (auto& item : data)                  // data �÷��ǿ� ���� 
			Ser(&item);                          // ��� Ser �۾��� ����
	}
	void Ser(std::vector<uint8_t>& data)  // int�� ���� ������ (��ȯ ����)
	{
		uint32_t count = uint32_t(data.size());
		Ser(count);
		if (!count)                                 // count = 0 �̸� ��
			return;
		if (doWrite)
		{
			fwrite(data.data(), count, 1, fp);      // ���̸�ŭ data�� fp�� ��
		}
		else
		{
			data.resize(count);                     
			fread(&data[0], count, 1, fp);          // ���̸�ŭ fp���� data�� �о����
		}
	}

	void Ser(InputSampler *inputSampler)               // inputSampler(���ø� �۾��� ����Ǵ� �ؽ���)�� �� ��ҿ� ���� Ser�۾� ����
	{
		ADD(v_initial, inputSampler->mWrapU);         // U�࿡ ���� ����
		ADD(v_initial, inputSampler->mWrapV);         // V�࿡ ���� ����
		ADD(v_initial, inputSampler->mFilterMin);     // ��� ����
		ADD(v_initial, inputSampler->mFilterMag);     // Ȯ�� ����
	}
	void Ser(MaterialNode *materialNode)               // materialNode(�ؽ��ĸ� �����ϰ� �ִ� ���)�� �� ��ҿ� ���� Ser�۾� ����
	{
		ADD(v_initial, materialNode->mType);            // ���Ÿ��
		ADD(v_initial, materialNode->mPosX);            // ����� x ��ġ
		ADD(v_initial, materialNode->mPosY);            // ����� y ��ġ
		ADD(v_initial, materialNode->mInputSamplers);   // ��尡 �����ϰ� �ִ� inputSampler
		ADD(v_initial, materialNode->mParameters);      // �з����� ����
	}
	void Ser(MaterialConnection *materialConnection)   // materialConnection(����� �����)�� �� ��ҿ� ���� Ser�۾� ����
	{
		ADD(v_initial, materialConnection->mInputNode);   // ������ ����Ǵ� ���
		ADD(v_initial, materialConnection->mOutputNode);  // �����Լ� ����� ���
		ADD(v_initial, materialConnection->mInputSlot);   // ����� �� �ִ� ������ ����
		ADD(v_initial, materialConnection->mOutputSlot);  // ������ �� �ִ� ������ ����
	}
	void Ser(Material *material)                       // material(�۾��� ����Ǵ� ����)�� �� ��ҿ� ���� Ser�۾� ����
	{
		ADD(v_initial, material->mName);                  // material�� �̸�
		ADD(v_materialComment, material->mComment);       // material�� ���� ����
		ADD(v_initial, material->mMaterialNodes);         // material�� ����
		ADD(v_initial, material->mMaterialConnections);   // material�� �����
	}
	bool Ser(Library *library)   // ���̺귯��
	{
		if (!fp)  // ���������Ͱ� ���� ��� ����
			return false;
		if (doWrite)
			dataVersion = v_lastVersion-1;  // �������� ��� ������ ������ �ٷ� ���������� ������ �������� ����
		Ser(dataVersion);                   // ������ ������ ���� Ser �۾� ����
		if (dataVersion > v_lastVersion)    // �ùٸ��� ���� ������ ��� ����
			return false; // no forward compatibility
		ADD(v_initial, library->mMaterials);// material�� ���� Ser �۾� ����
		return true;
	}
	FILE *fp;
	uint32_t dataVersion;
};

typedef Serialize<true> SerializeWrite;  // ������
typedef Serialize<false> SerializeRead;  // �б���

void LoadLib(Library *library, const char *szFilename)  // ���̺귯�� �ҷ�����
{
	SerializeRead(szFilename).Ser(library);
}

void SaveLib(Library *library, const char *szFilename)  // ���̺귯�� ����
{
	SerializeWrite(szFilename).Ser(library);
}
