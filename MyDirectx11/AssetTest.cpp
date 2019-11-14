#include "AssetTest.h"
#include "BindableBase.h"
#include "GraphicsThrowMacros.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Vertex.h"

AssetTest::AssetTest(Graphics& gfx, std::mt19937& rng, std::uniform_real_distribution<float>& adist, std::uniform_real_distribution<float>& ddist, std::uniform_real_distribution<float>& odist, std::uniform_real_distribution<float>& rdist, DirectX::XMFLOAT3 material, float scale)
	:
	TestObject(gfx, rng, adist, ddist, odist, rdist)
{
	namespace dx = DirectX;

	// ������������ �ʱ�ȭ ���� �ʾ�����
	if (!IsStaticInitialized())
	{

		using MyVertex::VertexLayout;
		//�������� Ÿ���� �����Ͽ����� ���� ����ü�θ� ���̾ƿ������� �����ϴ�.
		MyVertex::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
		));

		//assimp�� �̿��Ͽ� �� �ҷ�����
		Assimp::Importer imp;
		const auto pModel = imp.ReadFile("models\\suzanne.obj",
			aiProcess_Triangulate |	//�� �÷��״� �Ž��� ��F�� �����Ǵ����� ���� ���̴� Trinagulate�� �ﰢ������ �����ȴ�.
			aiProcess_JoinIdenticalVertices
			//�ε������۸� �̿��Ͽ� ������ �����Ҷ�. �������ϴ� �÷����ε� ��� �޽��� ����ũ ������ �����ԵǸ�
			// �ش� ������ ���� �鿡�� ����ϰԵȴ�. �̷��� �������� ������ ������ ���ϼ��ִµ� �� �ε������ۻ��� ����ϴ�
			//�ᱹ �������׸� �����ϰ� mNumVertices�� Ȯ���غ��� 7���̻��� ���̳��°��� Ȯ���� �� �������̴�.
		);
		//���� �޽��迭�� �̷�����ִµ� ->mMeshed[] �� ����Ͽ� �޽��� ������ �� �ִ�.
		const auto pMesh = pModel->mMeshes[0];

		//�� �������� ->mVerticse[] �� ���� ->mNormals �� �븻����
		for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3{ pMesh->mVertices[i].x * scale,pMesh->mVertices[i].y * scale,pMesh->mVertices[i].z * scale },
				*reinterpret_cast<dx::XMFLOAT3*>(&pMesh->mNormals[i])
			);
		}

		std::vector<unsigned short> indices;
		//mNumFace�� ���� primitive�� �����̴�. �� �ﰢ���ǰ�� ��������*3�� �� �ε����� �����̴�.
		indices.reserve(pMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
		{
			//�� �Ʒ��Ͱ��� mFaces[]�� �����ϸ� �ش� �޽��� �����ϰ� �ش� �޽��� ����3���� �̷�����ִٸ�
			//face�� �����Ͽ� face�����Ͽ� �ش� ������ �ε����� ������ �� �ִ�.
			const auto& face = pMesh->mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, vbuf));

		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, indices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));
		/*const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "Normal",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};*/

		// ������ ��ó�� ���̾ƿ��� ���� ���������ʴ´�. ���� �������� �ٲ� �ٲ�´��
		// ���Ӱ� �Է� ���̾ƿ��� �����ϱ⶧���̴�.  ���� ���̴������� �������� �ٲ����ϴ� �̴� ���߿� ������ ���̴�.
		AddStaticBind(std::make_unique<InputLayout>(gfx,vbuf.GetLayout().GetD3DLayout(),pvsbc));

		AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		struct PSMaterialConstant
		{
			DirectX::XMFLOAT3 color;
			float specularIntensity = 0.6f;
			float specularPower = 30.0f;
			float padding[3];
		} pmc;
		pmc.color = material;
		AddStaticBind(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));
	}
	else
	{
		SetIndexFromStatic();
	}

	AddBind(std::make_unique<TransformCbuf>(gfx, *this));
}