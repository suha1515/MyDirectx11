#include "Mesh.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

// Mesh
//�����ڿ��� ���ε� ������ ��ü���� �����͸� �ѱ�µ� �̶� �ε������۴� ���� �����Ͽ�
//AddIndexBuffer�� �ε������� ���ε��� �����Ѵ�.������ ����ũ �����͸� ����ϰ� �����Ƿ� 
//������ �μ��� ����ũ������ ���ͷ� �����Ƿ� ���� pi�� ����ũ �����ͷ� �����ؾ��ϹǷ� ���� 1���� ����ũ����Ʈ��
//����Ű�� �־���ϹǷ� ������ �Ѱ��� Release ���־�� �Ѵ�
//���ε� �����͵��� �ε������� �����ͷ� ĳ�����Ѵ�.
Mesh::Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bind::Bindable>> bindPtrs)
{
	if (!IsStaticInitialized())
	{
		AddStaticBind(std::make_unique<Bind::Topology>(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));
	}

	for (auto& pb : bindPtrs)
	{
		if (auto pi = dynamic_cast<Bind::IndexBuffer*>(pb.get()))
		{
			AddIndexBuffer(std::unique_ptr<Bind::IndexBuffer>{ pi });
			pb.release();
		}
		else//�ε������� �׿��� ���� Bind�� ȣ���Ѵ�.
		{
			AddBind(std::move(pb));
		}
	}
	//�������̴����� ����� Ʈ������ ���ε�
	AddBind(std::make_unique<Bind::TransformCbuf>(gfx, *this));
}
// �׸��⸦ ����ϴ� �Լ��̴�.
	// ���� ����� ���ý��ۿ��� �ڽ��ϰ�� �θ�κ����� ����� �����Ǿ� ���Ǳ⿡ accumlatedTransform �̴�.
void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
{
	DirectX::XMStoreFloat4x4(&transform, accumulatedTransform);
	Drawable::Draw(gfx);
}
DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat4x4(&transform);
}


// Node Ŭ����(Ʈ������)
/*
	���� �� �� �޽����� �ϳ��� ���ϱ����� �����ϰ��ִٸ� ���� �� ���ϱ����� �߽��̶�� �����ϸ� �ɰ��̴�.
	���� �޽����� �����͵��� �������־� Draw �Լ��� �ڽ��� Ʈ���������� �ڽ��� ��� �޽��鿡 �����Ͽ� �׸���.
	���� 1���̻��� �ڽ��� ������������ �ش� �ڽĵ鵵 ����� Ʈ���������ް� �׸���.
*/
Node::Node(const std:: string& name,std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd
	:
meshPtrs(std::move(meshPtrs)),
name(name)
{
	DirectX::XMStoreFloat4x4(&this->transform, transform);
}
//������ ����� �޾Ƽ� �ڽ��� Ʈ�������� ���Ͽ� �޽���� �ڽĵ鿡 �����Ͽ� �׸���.
void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
{
	const auto built = DirectX::XMLoadFloat4x4(&transform) * accumulatedTransform;
	for (const auto pm : meshPtrs)
	{
		pm->Draw(gfx, built);
	}
	for (const auto& pc : childPtrs)
	{
		pc->Draw(gfx, built);
	}
}
void Node::ShowTree(int& nodeIndexTracked, std::optional<int>& selectedIndex) const noexcept
{
	// nodexIndex �� gui Ʈ�� ��带 ���� uid ���۵��Ѵ�. ����ϸ鼭 �����ϰ� �ȴ�.
	const int currentNodeIndex = nodeIndexTracked;
	nodeIndexTracked++;
	// �ֱ� ��带 ���� �÷��׸� �����.
	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
		((currentNodeIndex == selectedIndex.value_or(-1)) ? ImGuiTreeNodeFlags_Selected : 0) |
		((childPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);
	//���� ��尡 Ȯ��Ǹ� ��ͷ� �ڽĵ��� �׸���.
	if (ImGui::TreeNodeEx((void*)(intptr_t)currentNodeIndex, node_flags, name.c_str()))
	{
		selectedIndex = ImGui::IsItemClicked() ? currentNodeIndex : selectedIndex;
		for (const auto& pChild : childPtrs)
		{
			pChild->ShowTree(nodeIndexTracked, selectedIndex);
		}
		ImGui::TreePop();
	}
}
//�ڽĳ�带 �߰��ϴ� �ڵ�
void Node::AddChild(std::unique_ptr<Node> pChild) noxnd
{
	assert(pChild);
	childPtrs.push_back(std::move(pChild));
}

// Model
class ModelWindow // pImpl idiom, only defined in this .cpp 
				  //pimple �̶� ������ �������� ���̱� ���� �ϳ��� ���� ����̴�.
{
public:
	void Show(const char* windowName, const Node& root) noexcept
	{
		// window name defaults to "Model"
		windowName = windowName ? windowName : "Model";
		//��� �ε������ ���õ� ��带 �����ϱ����� �ʿ��� ����
		int nodeIndexTracker = 0;
		if (ImGui::Begin(windowName))
		{
			ImGui::Columns(2, nullptr, true);
			root.ShowTree(nodeIndexTracker,selectedIndex);

			ImGui::NextColumn();
			ImGui::Text("Orientation");
			ImGui::SliderAngle("Roll", &pos.roll, -180.0f, 180.0f);
			ImGui::SliderAngle("Pitch", &pos.pitch, -180.0f, 180.0f);
			ImGui::SliderAngle("Yaw", &pos.yaw, -180.0f, 180.0f);
			ImGui::Text("Position");
			ImGui::SliderFloat("X", &pos.x, -20.0f, 20.0f);
			ImGui::SliderFloat("Y", &pos.y, -20.0f, 20.0f);
			ImGui::SliderFloat("Z", &pos.z, -20.0f, 20.0f);
		}
		ImGui::End();
	}
	dx::XMMATRIX GetTransform() const noexcept
	{
		return dx::XMMatrixRotationRollPitchYaw(pos.roll, pos.pitch, pos.yaw) *
			dx::XMMatrixTranslation(pos.x, pos.y, pos.z);
	}
private:
	std::optional<int> selectedIndex;
	struct
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	} pos;
};

// Model Ŭ����
/*
	3d ������Ʈ�� �ҷ����� �޽�,���� ����� �����Ѵ�
*/
//3d ���� assimp�� �ҷ��´�.
Model::Model(Graphics& gfx, const std::string fileName)
	:pWindow(std::make_unique<ModelWindow>())
{
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile(fileName.c_str(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices
	);
	//assimp������ Scene�̶�� �Էµ� �������� ��Ʈ �����̴�.
	//��� ������ Scene���� �����Ѵ�.

	//�ش� ������Ʈ�� �Ž��� ���Ϳ� ����
	for (size_t i = 0; i < pScene->mNumMeshes; i++)
	{
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i]));
	}
	//��Ʈ��带 ParseNode�� �����Ѵ�.
	pRoot = ParseNode(*pScene->mRootNode);
}

//�׸��� �۾��� �����Ѵ�.
void Model::Draw(Graphics& gfx) const noxnd
{
	pRoot->Draw(gfx, pWindow->GetTransform());
}
// �𵨺� ImGui ����
void Model::ShowWindow(const char* windowName) noexcept
{
	pWindow->Show(windowName, *pRoot);
}
Model::~Model() noexcept
{
}
// ������Ʈ�� �޽��� �����Ͽ� ������ �����Ѵ�.
std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh)
{
	//aiMesh�� ���״�� ���������� �ε����� �̷���� ���ϱ����̴�.
	namespace dx = DirectX;
	using Dvtx::VertexLayout;

	//���� ���̾ƿ��� �����Ѵ� (��ġ,���)
	Dvtx::VertexBuffer vbuf(std::move(
		VertexLayout{}
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
	));
	//����������ŭ �������ۿ� �����Ѵ�.
	//�������ۿ� ���� ���̾ƿ��� �����Ͽ����Ƿ� �ش� ���̾ƿ���� ���Եȴ�.
	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		vbuf.EmplaceBack(
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
		);
	}

	// �޽����� aiProcess_Triangulate �� �ﰢ�� ������ �޴´�
	// ��� �ε����� ���ǰ���*3 ���ϸ� ���ε��������� ũ�⸦ ���� �� �ִ�.
	std::vector<unsigned short> indices;
	indices.reserve(mesh.mNumFaces * 3);
	
	//���� ������ŭ �����Ͽ� ������ �ε��������� ���ۿ� �����Ѵ�. �ε������� 3���̻��̸� ����
	for (unsigned int i = 0; i < mesh.mNumFaces; i++)
	{
		const auto& face = mesh.mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}
	//���ε尡���� ��ü���� ���� ���� �����̳�
	std::vector<std::unique_ptr<Bind::Bindable>> bindablePtrs;
	//������ �غ��� ����,�ε������۸� ����
	bindablePtrs.push_back(std::make_unique<Bind::VertexBuffer>(gfx, vbuf));

	bindablePtrs.push_back(std::make_unique<Bind::IndexBuffer>(gfx, indices));

	//�������̴��� �ҷ��´� ���� ����.
	auto pvs = std::make_unique<Bind::VertexShader>(gfx, L"PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();
	bindablePtrs.push_back(std::move(pvs));

	//�ȼ����̴��� �ҷ��´� ���� ����.
	bindablePtrs.push_back(std::make_unique<Bind::PixelShader>(gfx, L"PhongPS.cso"));

	//�������۷κ��� �Է·��̾ƿ��� �����Ѵ�.
	bindablePtrs.push_back(std::make_unique<Bind::InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

	//�Ƚ����̴� ������ۿ� �����ϱ����� ����ü
	struct PSMaterialConstant
	{
		DirectX::XMFLOAT3 color = { 0.6f,0.6f,0.8f };
		float specularIntensity = 0.6f;
		float specularPower = 30.0f;
		float padding[3];
	} pmc;
	//�ش� ������۸� �Ƚ����̴� ����1�� ������ �ش� ���δ��� ��ü�� �����̳ʿ� ����
	bindablePtrs.push_back(std::make_unique<Bind::PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));

	// �̸�� ������ ��ġ�� �ش� 3d ������Ʈ�� �Ľ��Ͽ� �����Ϸ�Ǿ���.
	// mesh ��ü�� ����ũ�����ͷ� �Ҵ��Ͽ� ���ε������̳ʿ��� ���������ο� ���ε��� �����Ѵ�.
	return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
}
std::unique_ptr<Node> Model::ParseNode(const aiNode& node) noexcept
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
	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		const auto meshIdx = node.mMeshes[i];
		curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
	}
	//������ �޽� �����̳ʿ��Բ� Ʈ���������� ��尴ü�� �Ҵ��Ѵ�.
	auto pNode = std::make_unique<Node>(node.mName.C_Str(),std::move(curMeshPtrs), transform);
	//���� ��忡 �ڽ��� ������� �ڽ� ������ŭ �߰��Ѵ�.
	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		//���⼭ ���ȣ��� �ڽĿ����� ��������� �����ϰ� �ɰ��̴�.
		pNode->AddChild(ParseNode(*node.mChildren[i]));
	}

	return pNode;
}

