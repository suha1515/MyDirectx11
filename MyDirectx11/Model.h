#pragma once
#include "DrawableBase.h"
#include "BindableBase.h"
#include "Vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


// Mesh Ŭ����
/*
	���� �޽��� ��Ÿ���� Ŭ�����̴�.
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
			//�����ڿ��� ���ε� ������ ��ü���� �����͸� �ѱ�µ� �̶� �ε������۴� ���� �����Ͽ�
			//AddIndexBuffer�� �ε������� ���ε��� �����Ѵ�.������ ����ũ �����͸� ����ϰ� �����Ƿ� 
			//������ �μ��� ����ũ������ ���ͷ� �����Ƿ� ���� pi�� ����ũ �����ͷ� �����ؾ��ϹǷ� ���� 1���� ����ũ����Ʈ��
			//����Ű�� �־���ϹǷ� ������ �Ѱ��� Release ���־�� �Ѵ�
			//���ε� �����͵��� �ε������� �����ͷ� ĳ�����Ѵ�.
			if (auto pi = dynamic_cast<IndexBuffer*>(pb.get()))
			{
				AddIndexBuffer(std::unique_ptr<IndexBuffer>{pi});
				pb.release();
			}
			//�ε������� �׿��� ���� Bind�� ȣ���Ѵ�.
			else
				AddBind(std::move(pb));
		}
		//�������̴����� ����� Ʈ������ ���ε�
		AddBind(std::make_unique<TransformCbuf>(gfx, *this));
	}
	// �׸��⸦ ����ϴ� �Լ��̴�.
	// ���� ����� ���ý��ۿ��� �ڽ��ϰ�� �θ�κ����� ����� �����Ǿ� ���Ǳ⿡ accumlatedTransform �̴�.
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
	//mutable Ű����� const ��� �Լ������� ���� �ٲ�� �����̴�.
	mutable DirectX::XMFLOAT4X4 transform;
};

// Node Ŭ����(Ʈ������)
/*
	���� �� �� �޽����� �ϳ��� ���ϱ����� �����ϰ��ִٸ� ���� �� ���ϱ����� �߽��̶�� �����ϸ� �ɰ��̴�.
	���� �޽����� �����͵��� �������־� Draw �Լ��� �ڽ��� Ʈ���������� �ڽ��� ��� �޽��鿡 �����Ͽ� �׸���.
	���� 1���̻��� �ڽ��� ������������ �ش� �ڽĵ鵵 ����� Ʈ���������ް� �׸���.
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
	//������ ����� �޾Ƽ� �ڽ��� Ʈ�������� ���Ͽ� �޽���� �ڽĵ鿡 �����Ͽ� �׸���.
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumlatedTransform) const noexcept(!IS_DEBUG)
	{
		//�θ� �ִٸ� ������ ��Ŀ� ���� �� �������̴�.
		const auto built = DirectX::XMLoadFloat4x4(&transform) * accumlatedTransform;
		for (const auto pm : meshPtrs)
			pm->Draw(gfx, built);
		for (const auto& pc : childPtrs)
			pc->Draw(gfx, built);
	}
private:
	//�ڽĳ�带 �߰��ϴ� �ڵ�
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

// Model Ŭ����
/*
	3d ������Ʈ�� �ҷ����� �޽�,���� ����� �����Ѵ�
	
*/

class Model
{
public:
	//3d ���� assimp�� �ҷ��´�.
	Model(Graphics& gfx, const std::string fileName)
	{
		Assimp::Importer imp;
		const auto pScene = imp.ReadFile(fileName.c_str(),
			aiProcess_Triangulate |
			aiProcess_JoinIdenticalVertices);

		//assimp������ Scene�̶�� �Էµ� �������� ��Ʈ �����̴�.
		//��� ������ Scene���� �����Ѵ�.

		//�ش� ������Ʈ�� �Ž��� ���Ϳ� ����
		for (size_t i = 0; i < pScene->mNumMeshes; ++i)
			meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));

		//��Ʈ��带 ParseNode�� �����Ѵ�.
		pRoot = ParseNode(*pScene->mRootNode);
	}
	
	// ParseMesh
	// ������Ʈ�� �޽��� �����Ͽ� ������ �����Ѵ�.
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh)
	{
		//aiMesh�� ���״�� ���������� �ε����� �̷���� ���ϱ����̴�.
		namespace dx = DirectX;
		using MyVertex::VertexLayout;

		//���� ���̾ƿ��� �����Ѵ� (��ġ,���)
		MyVertex::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
		));

		//����������ŭ �������ۿ� �����Ѵ�.
		//�������ۿ� ���� ���̾ƿ��� �����Ͽ����Ƿ� �ش� ���̾ƿ���� ���Եȴ�.
		for (size_t i = 0; i < mesh.mNumVertices; ++i)
		{
			vbuf.EmplaceBack(
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
			);
		}


		std::vector<unsigned short> indices;
		// �޽����� aiProcess_Triangulate �� �ﰢ�� ������ �޴´�
		// ��� �ε����� ���ǰ���*3 ���ϸ� ���ε��������� ũ�⸦ ���� �� �ִ�.
		indices.reserve(mesh.mNumFaces * 3);

		//���� ������ŭ �����Ͽ� ������ �ε��������� ���ۿ� �����Ѵ�. �ε������� 3���̻��̸� ����
		for (size_t i = 0; i < mesh.mNumFaces; ++i)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		//���ε尡���� ��ü���� ���� ���� �����̳�
		std::vector<std::unique_ptr<Bindable>> bindablePtrs;

		//������ �غ��� ����,�ε������۸� ����
		bindablePtrs.push_back(std::make_unique<VertexBuffer>(gfx, vbuf));

		bindablePtrs.push_back(std::make_unique<IndexBuffer>(gfx, indices));

		//�������̴��� �ҷ��´� ���� ����.
		auto pvs = std::make_unique<VertexShader>(gfx, L"PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		//�ȼ����̴��� �ҷ��´� ���� ����.
		bindablePtrs.push_back(std::make_unique<PixelShader>(gfx, L"PhongPS.cso"));

		//�������۷κ��� �Է·��̾ƿ��� �����Ѵ�.
		bindablePtrs.push_back(std::make_unique<InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

		//�Ƚ����̴� ������ۿ� �����ϱ����� ����ü
		struct PSMaterialConstant
		{
			DirectX::XMFLOAT3 color = { 0.6f,0.6f,0.8f };
			float specularIntensity = 0.6f;
			float specularPower = 30.0f;
			float padding[3];
		}pmc;
		//�ش� ������۸� �Ƚ����̴� ����1�� ������ �ش� ���δ��� ��ü�� �����̳ʿ� ����
		bindablePtrs.push_back(std::make_unique<PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));

		// �̸�� ������ ��ġ�� �ش� 3d ������Ʈ�� �Ľ��Ͽ� �����Ϸ�Ǿ���.
		// mesh ��ü�� ����ũ�����ͷ� �Ҵ��Ͽ� ���ε������̳ʿ��� ���������ο� ���ε��� �����Ѵ�.
		return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
	}


	std::unique_ptr<Node> ParseNode(const aiNode& node)
	{
		namespace dx = DirectX;
		// assimp ���� �� ���� Ʈ�������� ������ �����Ƿ� �ش� ����� �޾ƿ´�
		const auto transform = dx::XMMatrixTranspose(dx::XMLoadFloat4x4(
			reinterpret_cast<const dx::XMFLOAT4X4*>(&node.mTransformation)
		));

		std::vector<Mesh*> curMeshPtrs;
		//����� �޽� ������ŭ ���� ũ�⸦ �����Ѵ�
		curMeshPtrs.reserve(node.mNumMeshes);
		//�޽��� �����Ͽ� �ش�޽��� �ε����� ���ѵ�
		//�𵨿� �����Ǿ��ִ� �޽����Ϳ��� �����͸� ���´�.
		for (size_t i = 0; i < node.mNumMeshes; ++i)
		{
			const auto meshIdx = node.mMeshes[i];
			curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
		}

		//������ �޽� �����̳ʿ��Բ� Ʈ���������� ��尴ü�� �Ҵ��Ѵ�.
		auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);
		//���� ��忡 �ڽ��� ������� �ڽ� ������ŭ �߰��Ѵ�.
		for (size_t i = 0; i < node.mNumChildren; ++i)
		{
			//���⼭ ���ȣ��� �ڽĿ����� ��������� �����ϰ� �ɰ��̴�.
			pNode->AddChild(ParseNode(*node.mChildren[i]));
		}
		return pNode;
	}
	//�׸��� �۾��� �����Ѵ�.
	void Draw(Graphics& gfx) const
	{
		//��Ʈ���� �׵������ ������ȴ�.
		pRoot->Draw(gfx, DirectX::XMMatrixIdentity());
	}

private:
	std::unique_ptr<Node> pRoot;					//��Ʈ ���
	std::vector<std::unique_ptr<Mesh>> meshPtrs;	//���� �޽���
};