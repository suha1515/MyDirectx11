#include "AssetTest.h"
#include "BindableBase.h"
#include "GraphicsThrowMacros.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

AssetTest::AssetTest(Graphics& gfx, std::mt19937& rng, std::uniform_real_distribution<float>& adist, std::uniform_real_distribution<float>& ddist, std::uniform_real_distribution<float>& odist, std::uniform_real_distribution<float>& rdist, DirectX::XMFLOAT3 material, float scale)
	:
	TestObject(gfx, rng, adist, ddist, odist, rdist)
{
	namespace dx = DirectX;

	// 정적변수들이 초기화 되지 않았으면
	if (!IsStaticInitialized())
	{
		struct Vertex
		{
			dx::XMFLOAT3 pos;
			dx::XMFLOAT3 n;
		};
		//assimp를 이용하여 모델 불러오기
		Assimp::Importer imp;
		const auto pModel = imp.ReadFile("models\\suzanne.obj",
			aiProcess_Triangulate |	//이 플래그는 매쉬가 어덯게 구성되는지에 대한 것이다 Trinagulate는 삼각형으로 구성된다.
			aiProcess_JoinIdenticalVertices
			//인덱스버퍼를 이용하여 렌더를 진행할때. 쓰여야하는 플래그인데 모든 메쉬가 유니크 정점을 가지게되며
			// 해당 정점을 여러 면에서 사용하게된다. 이러한 과정으로 정점의 개수를 줄일수있는데 즉 인덱스버퍼사용과 비슷하다
			//결국 이프랠그를 해제하고 mNumVertices를 확인해보면 7배이상이 차이나는것을 확인할 수 있을것이다.
		);
		//모델은 메쉬배열로 이루어져있는데 ->mMeshed[] 를 사용하여 메쉬에 접근할 수 있다.
		const auto pMesh = pModel->mMeshes[0];

		std::vector<Vertex> vertices;
		vertices.reserve(pMesh->mNumVertices);
		//각 정점또한 ->mVerticse[] 로 접근 ->mNormals 는 노말접근
		for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
		{
			vertices.push_back({
				{ pMesh->mVertices[i].x * scale,pMesh->mVertices[i].y * scale,pMesh->mVertices[i].z * scale },
				*reinterpret_cast<dx::XMFLOAT3*>(&pMesh->mNormals[i])
				});
		}

		std::vector<unsigned short> indices;
		//mNumFace는 도형 primitive의 갯수이다. 즉 삼각형의경우 도형개수*3이 총 인덱스의 개수이다.
		indices.reserve(pMesh->mNumFaces * 3);
		for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
		{
			//즉 아래와같이 mFaces[]로 접근하면 해당 메쉬에 접근하고 해당 메쉬가 정점3개로 이루어져있다면
			//face에 대입하여 face를통하여 해당 정점의 인덱스에 접근할 수 있다.
			const auto& face = pMesh->mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		AddStaticBind(std::make_unique<VertexBuffer>(gfx, vertices));

		AddStaticIndexBuffer(std::make_unique<IndexBuffer>(gfx, indices));

		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
		{
			{ "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
			{ "Normal",0,DXGI_FORMAT_R32G32B32_FLOAT,0,12,D3D11_INPUT_PER_VERTEX_DATA,0 },
		};
		AddStaticBind(std::make_unique<InputLayout>(gfx, ied, pvsbc));

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
