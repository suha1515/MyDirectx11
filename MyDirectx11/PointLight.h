#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "ConstantBuffer.h"

// ������ Ŭ����
/*
	���忡�� �������� ��Ÿ���� Ŭ�����̴�
*/

class PointLight
{
public:
	PointLight(Graphics& gfx, float radius = 0.5f);
	
	//���� ��Ʈ���� ���� imgui
	void SpawnControlWindow() noexcept;

	//������ġ �ʱ�ȭ
	void Reset() noexcept;

	//���� �ð�ȭ ��ü�� ���� Draw,Bind
	void Draw(Graphics& gfx) const noexcept (!IS_DEBUG);
	void Bind(Graphics& gfx) const noexcept;
private:
	struct PointLightCBuf
	{
		DirectX::XMFLOAT3 pos;		//������ ��ġ
		float padding;				//�е���
	};
private:
	DirectX::XMFLOAT3 pos = { 0.0f,0.0f,0.0f };			//������ ��ġ
	mutable SolidSphere mesh;							//���忡�� ������ �ð�ȭ�� ���� ��ü
	mutable PixelConstantBuffer<PointLightCBuf> cbuf;	//�Ƚ����̴� ������ۿ� �� ��
};

