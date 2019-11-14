#pragma once
#include "Graphics.h"
#include "SolidSphere.h"
#include "ConstantBuffer.h"
#include "ConditionalNoexcept.h"
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
	void Draw(Graphics& gfx) const noxnd;
	void Bind(Graphics& gfx, DirectX::FXMMATRIX view) const noexcept;
private:
	// ������۸� �ѱ涧 hlsl ���̴������� �ش� �������̿� padding ���� �ִٰ� �����ϰ�޴´�
	// ������ �츮�� ����ü�� �е������� �ٷκپ������Ƿ� hlsl���� �������� �Դٰ� ���� �޽����� �����Եȴ�
	// �̰�� ����ü �������̿� �е� ������ �ְų� alignas Ű���带 ����Ͽ� �ش� �������̿� �е����� ����

	//float3 �� ��� hlsl ���� 16�е����� ������ �ִٰ� �����Ұ��̴�. �׳� float�� �Ǵµ�
	struct PointLightCBuf
	{
		alignas(16)DirectX::XMFLOAT3 pos;			//������ ��ġ
		alignas(16)DirectX::XMFLOAT3 ambient;		//�ֺ���
		alignas(16)DirectX::XMFLOAT3 diffuseColor; //�л걤
		float	diffuseIntensity;
		float	attConst;
		float	attLin;
		float	attQuad;
	};
private:
	PointLightCBuf		cbData;							//���� �������
	mutable SolidSphere mesh;							//���忡�� ������ �ð�ȭ�� ���� ��ü
	mutable Bind::PixelConstantBuffer<PointLightCBuf> cbuf;	//�Ƚ����̴� ������ۿ� �� ��
};

