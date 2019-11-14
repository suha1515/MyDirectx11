#pragma once
#include "DrawableBase.h"
#include "BindableBase.h"
#include "Vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


// Mesh 클래스
/*
	모델의 메쉬를 나타내는 클래스이다.
*/
class Mesh : public DrawableBase<Mesh>
{
public:
	Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs)
	{
		if (!IsStaticInitialized())
			AddStaticBind(std::make_unique<Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

		for (auto& pb : bindPtrs)
		{	
			//생성자에서 바인드 가능한 객체들의 포인터를 넘기는데 이때 인덱스버퍼는 따로 구분하여
			//AddIndexBuffer로 인덱스버퍼 바인딩을 수행한다.하지만 유니크 포인터를 사용하고 있으므로 
			//생성자 인수로 유니크포인터 벡터로 받으므로 따로 pi를 유니크 포인터로 전달해야하므로 오직 1개만 유니크포인트가
			//가르키고 있어야하므로 나머지 한개는 Release 해주어야 한다
			//바인드 포인터들을 인덱스버퍼 포인터로 캐스팅한다.
			if (auto pi = dynamic_cast<IndexBuffer*>(pb.get()))
			{
				AddIndexBuffer(std::unique_ptr<IndexBuffer>{pi});
				pb.release();
			}
			//인덱스버퍼 그외의 것은 Bind를 호출한다.
			else
				AddBind(std::move(pb));
		}
		//정점쉐이더에서 사용할 트랜스폼 바인딩
		AddBind(std::make_unique<TransformCbuf>(gfx, *this));
	}
	// 그리기를 담당하는 함수이다.
	// 인자 행렬은 노드시스템에서 자식일경우 부모로부터의 행렬이 축적되어 사용되기에 accumlatedTransform 이다.
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumlatedTransform) const noexcept (!IS_DEBUG)
	{
		DirectX::XMStoreFloat4x4(&transform, accumlatedTransform);
		Drawable::Draw(gfx);
	}
	DirectX::XMMATRIX GetTransformXM() const noexcept override
	{
		return DirectX::XMLoadFloat4x4(&transform);
	}

private:
	//mutable 키워드는 const 멤버 함수에서도 값이 바뀌는 변수이다.
	mutable DirectX::XMFLOAT4X4 transform;
};

// Node 클래스(트리구조)
/*
	노드는 즉 각 메쉬들이 하나의 기하구조를 구성하고있다면 노드는 그 기하구조의 중심이라고 생각하면 될것이다.
	노드는 메쉬들의 포인터들을 가지고있어 Draw 함수로 자신의 트랜스폼으로 자신의 노드 메쉬들에 전달하여 그린다.
	노드는 1개이상의 자식을 가지고있으며 해당 자식들도 노드의 트랜스폼을받고 그린다.
*/
class Node
{
	friend class Model;
public:
	Node(std::vector<Mesh*> meshPtrs,const DirectX::XMMATRIX& transform) noexcept(!IS_DEBUG)
		: meshPtrs(std::move(meshPtrs))
	{
		DirectX::XMStoreFloat4x4(&this->transform, transform);
	}
	//축적된 행렬을 받아서 자신의 트랜스폼과 곱하여 메쉬들과 자식들에 전달하여 그린다.
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumlatedTransform) const noexcept(!IS_DEBUG)
	{
		//부모가 있다면 축적된 행렬에 값이 들어가 있을것이다.
		const auto built = DirectX::XMLoadFloat4x4(&transform) * accumlatedTransform;
		for (const auto pm : meshPtrs)
			pm->Draw(gfx, built);
		for (const auto& pc : childPtrs)
			pc->Draw(gfx, built);
	}
private:
	//자식노드를 추가하는 코드
	void AddChild(std::unique_ptr<Node> pChild) noexcept (!IS_DEBUG)
	{
		assert(pChild);
		childPtrs.push_back(std::move(pChild));
	}
private:
	std::vector<std::unique_ptr<Node>> childPtrs;
	std::vector<Mesh*> meshPtrs;
	DirectX::XMFLOAT4X4 transform;
};

// Model 클래스
/*
	3d 오브젝트를 불러오고 메쉬,노드로 나누어서 보관한다
	
*/

class Model
{
public:
	//3d 모델을 assimp로 불러온다.
	Model(Graphics& gfx, const std::string fileName)
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(fileName.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices);

		//assimp에서는 Scene이라고 입력된 데이터의 루트 구조이다.
		//모든 구조는 Scene에서 접근한다.

		//해당 오브젝트의 매쉬를 벡터에 삽입
		for (size_t i = 0; i < pScene->mNumMeshes; ++i)
			meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));

		//루트노드를 ParseNode에 전달한다.
		pRoot = ParseNode(*pScene->mRootNode);
	}
	
	// ParseMesh
	// 오브젝트의 메쉬에 접근하여 정보를 가공한다.
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh)
	{
		//aiMesh는 말그대로 여러정점과 인덱스로 이루어진 기하구조이다.
		namespace dx = DirectX;
		using MyVertex::VertexLayout;

		//동적 레이아웃을 지정한다 (위치,노멀)
		MyVertex::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
		));

		//정점개수만큼 정점버퍼에 삽입한다.
		//정점버퍼에 동적 레이아웃을 지정하였으므로 해당 레이아웃대로 삽입된다.
		for (size_t i = 0; i < mesh.mNumVertices; ++i)
		{
			vbuf.EmplaceBack(
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
			);
		}


		std::vector<unsigned short> indices;
		// 메쉬들을 aiProcess_Triangulate 즉 삼각형 구조로 받는다
		// 모든 인덱스는 면의개수*3 을하면 총인덱스버퍼의 크기를 구할 수 있다.
		indices.reserve(mesh.mNumFaces * 3);

		//면의 개수만큼 접근하여 각면의 인덱스정보를 버퍼에 삽입한다. 인덱스점이 3개이상이면 예외
		for (size_t i = 0; i < mesh.mNumFaces; ++i)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		//바인드가능한 객체들을 담을 벡터 컨테이너
		std::vector<std::unique_ptr<Bindable>> bindablePtrs;

		//위에서 준비한 정점,인덱스버퍼를 삽입
		bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));

		bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices));

		//정점쉐이더를 불러온다 그후 삽입.
		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		//픽셀쉐이더를 불러온다 그후 삽입.
		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

		//정점버퍼로부터 입력레이아웃을 삽입한다.
		bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

		//픽쉘세이더 상수버퍼에 전달하기위한 구조체
		struct PSMaterialConstant
		{
			DirectX::XMFLOAT3 color = { 0.6f,0.6f,0.8f };
			float specularIntensity = 0.6f;
			float specularPower = 30.0f;
			float padding[3];
		}pmc;
		//해당 상수버퍼를 픽쉘세이더 슬롯1에 지정후 해당 바인더블 객체를 컨테이너에 삽입
		bindablePtrs.push_back(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));

		// 이모든 과정을 거치면 해당 3d 오브젝트를 파싱하여 가공완료되었다.
		// mesh 객체를 유니크포인터로 할당하여 바인딩컨테이너에서 파이프라인에 바인딩을 진행한다.
		return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
	}


	std::unique_ptr<Node> ParseNode(const aiNode& node)
	{
		namespace dx = DirectX;
		// assimp 에서 각 노드는 트랜스폼을 가지고 있으므로 해당 행렬을 받아온다
		const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
			reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
		));

		std::vector<Mesh*> curMeshPtrs;
		//노드의 메쉬 개수만큼 벡터 크기를 예약한다
		curMeshPtrs.reserve(node.mNumMeshes);
		//메쉬에 접근하여 해당메쉬의 인덱스를 구한뒤
		//모델에 보관되어있는 메쉬벡터에서 포인터를 얻어온다.
		for (size_t i = 0; i < node.mNumMeshes; ++i)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
		}

		//구성된 메쉬 컨테이너와함께 트랜스폼으로 노드객체를 할당한다.
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);
		//만약 노드에 자식이 있을경우 자식 개수만큼 추가한다.
		for (size_t i = 0; i < node.mNumChildren; ++i)
		{
			//여기서 재귀호출로 자식에서도 노드정보를 구성하게 될것이다.
			pNode->AddChild(ParseNode(*node.mChildren[i]));
		}
		return pNode;
	}
	//그리기 작업을 수행한다.
	void Draw(Graphics& gfx) const
	{
		//루트노드는 항등행렬을 가지면된다.
		pRoot->Draw(gfx, DirectX::XMMatrixIdentity());
	}

private:
	std::unique_ptr<Node> pRoot;					//루트 노드
	std::vector<std::unique_ptr<Mesh>> meshPtrs;	//모델의 메쉬들
};