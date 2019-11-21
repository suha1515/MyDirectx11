#include "Mesh.h"
#include "imgui/imgui.h"
#include "Surface.h"
#include <unordered_map>
#include <sstream>

namespace dx = DirectX;


ModelException::ModelException(int line, const char* file, std::string note) noexcept
	:BsException(line,file),
	note(std::move(note))
{}

const char* ModelException::what() const noexcept
{
	std::ostringstream oss;
	oss << BsException::what() << std::endl
		<< "[Note] " << GetNote();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* ModelException::GetType() const noexcept
{
	return "Model Exception";
}

const std::string& ModelException::GetNote() const noexcept
{
	return note;
}
// Mesh
//�����ڿ��� ���ε� ������ ��ü���� �����͸� �ѱ�µ� �̶� �ε������۴� ���� �����Ͽ�
//AddIndexBuffer�� �ε������� ���ε��� �����Ѵ�.������ ����ũ �����͸� ����ϰ� �����Ƿ� 
//������ �μ��� ����ũ������ ���ͷ� �����Ƿ� ���� pi�� ����ũ �����ͷ� �����ؾ��ϹǷ� ���� 1���� ����ũ����Ʈ��
//����Ű�� �־���ϹǷ� ������ �Ѱ��� Release ���־�� �Ѵ�
//���ε� �����͵��� �ε������� �����ͷ� ĳ�����Ѵ�.
Mesh::Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>> bindPtrs)
{
		AddBind(Bind::Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	for (auto& pb : bindPtrs)
	{
			AddBind(std::move(pb));
	}
	//�������̴����� ����� Ʈ������ ���ε�
	AddBind(std::make_shared<Bind::TransformCbuf>(gfx, *this));
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
Node::Node(int id,const std:: string& name,std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform_in) noxnd
	:
id(id),
meshPtrs(std::move(meshPtrs)),
name(name)
{
	dx::XMStoreFloat4x4(&transform, transform_in);
	dx::XMStoreFloat4x4(&appliedTransform, dx::XMMatrixIdentity());
}
//������ ����� �޾Ƽ� �ڽ��� Ʈ�������� ���Ͽ� �޽���� �ڽĵ鿡 �����Ͽ� �׸���.
void Node::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
{
	const auto built = 
		dx::XMLoadFloat4x4(&appliedTransform) *
		dx::XMLoadFloat4x4(&transform)*
		accumulatedTransform;
	for (const auto pm : meshPtrs)
	{
		pm->Draw(gfx, built);
	}
	for (const auto& pc : childPtrs)
	{
		pc->Draw(gfx, built);
	}
}

void Node::ShowTree( Node*& pSelectedNode) const noexcept
{
	//���õ� ��尡 ������� ���õ� ID ���� -1�� �����Ѵ�.
	const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();
	// �ֱ� ��带 ���� �÷��׸� �����.
	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
		((GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0) |
		((childPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);
	//���� ��尡 Ȯ��Ǹ� ��ͷ� �ڽĵ��� �׸���.
	const auto expanded = ImGui::TreeNodeEx(
		(void*)(intptr_t)GetId(), node_flags, name.c_str()
	);

	if (ImGui::IsItemClicked())
	{
		pSelectedNode = const_cast<Node*>(this);
	}
	if (expanded)
	{
		for (const auto& pChild : childPtrs)
		{
			pChild->ShowTree(pSelectedNode);
		}
		ImGui::TreePop();
	}
}
void Node::SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept
{
	dx::XMStoreFloat4x4(&appliedTransform, transform);
}
//�ڽĳ�带 �߰��ϴ� �ڵ�
void Node::AddChild(std::unique_ptr<Node> pChild) noxnd
{
	assert(pChild);
	childPtrs.push_back(std::move(pChild));
}
int Node::GetId() const noexcept
{
	return id;
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
			root.ShowTree(pSelectedNode);
			if (pSelectedNode != nullptr)//���õ� ��尡 ������츸 gui ǥ��
			{
				auto& transform = transforms[pSelectedNode->GetId()];
				ImGui::NextColumn();
				ImGui::Text("Orientation");
				ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
				ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
				ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
				ImGui::Text("Position");
				ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
				ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
				ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);
			}
		}
		ImGui::End();
	}
	dx::XMMATRIX GetTransform() const noexcept
	{
		assert(pSelectedNode != nullptr);
		const auto& transform = transforms.at(pSelectedNode->GetId());
		return
			dx::XMMatrixRotationRollPitchYaw(transform.roll, transform.pitch, transform.yaw) *
			dx::XMMatrixTranslation(transform.x, transform.y, transform.z);
	}
	Node* GetSelectedNode() const noexcept
	{
		return pSelectedNode;
	}
private:
	std::optional<int> selectedIndex;	//���õǾ����� �Ǿ����� �ش� �ε�����ȯ�ƴϸ� false
	Node* pSelectedNode;				//���õȳ��
	struct TransformParameters
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	} ;
	std::unordered_map<int, TransformParameters> transforms;	//�� gui ���� �ε����� �ش��ϴ� ����� Ʈ�������� �����Ѵ�.
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
		aiProcess_JoinIdenticalVertices|
		aiProcess_ConvertToLeftHanded|	//�޼���ǥ���
		aiProcess_GenNormals|			//��ֻ���
		aiProcess_CalcTangentSpace		//ź��Ʈ ���
	);

	if (pScene == nullptr)
		throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
	//assimp������ Scene�̶�� �Էµ� �������� ��Ʈ �����̴�.
	//��� ������ Scene���� �����Ѵ�.

	//�ش� ������Ʈ�� �Ž��� ���Ϳ� ����,aiScene�� �ִ� ���͸���迭 �� ���� �Ľ��Ѵ�.
	for (size_t i = 0; i < pScene->mNumMeshes; ++i)
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials));

	//��Ʈ��带 ParseNode�� �����Ѵ�.
	int nextId = 0;
	pRoot = ParseNode(nextId ,*pScene->mRootNode);
}

//�׸��� �۾��� �����Ѵ�.
void Model::Draw(Graphics& gfx) const noxnd
{
	if (auto node = pWindow->GetSelectedNode())
	{
		node->SetAppliedTransform(pWindow->GetTransform());
	}
	pRoot->Draw(gfx, dx::XMMatrixIdentity());
}
// �𵨺� ImGui ����
void Model::ShowWindow(const char* windowName) noexcept
{
	pWindow->Show(windowName, *pRoot);
}
void Model::SetRootTransform(DirectX::FXMMATRIX tf) noexcept
{
	pRoot->SetAppliedTransform(tf);
}
Model::~Model() noexcept
{
}
// ������Ʈ�� �޽��� �����Ͽ� ������ �����Ѵ�.
std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh,const aiMaterial* const* pMaterial)
{
	//aiMesh�� ���״�� ���������� �ε����� �̷���� ���ϱ����̴�.
	namespace dx = DirectX;
	using namespace Bind;
	using Dvtx::VertexLayout;

	//���� ���̾ƿ��� �����Ѵ� (��ġ,���)
	Dvtx::VertexBuffer vbuf(std::move(
		VertexLayout{}
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		.Append(VertexLayout::Tangent)
		.Append(VertexLayout::Bitangent)
		.Append(VertexLayout::Texture2D)		//�ؽ�����ǥ�������� �������̾ƿ� �߰�����.
	));

	// �޽��� ��������� ���͸��� �迭�� �ε����� �������ִ�.
	// �ε����� �޽��� ���� ���͸��� ������ �� �ִ�.
	auto& material = *pMaterial[mesh.mMaterialIndex];

	//����������ŭ �������ۿ� �����Ѵ�.
	//�������ۿ� ���� ���̾ƿ��� �����Ͽ����Ƿ� �ش� ���̾ƿ���� ���Եȴ�.
	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		vbuf.EmplaceBack(
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),		//ź��Ʈ
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),		//����ź��Ʈ
			//assimp���� �ؽ�ó��ǥ�� ���߹迭���̷�����µ� �ؽ�����ǥ������ 8���� ä���� ������ �ִ�.
			//�Ʒ��� ù��° ä���̴�.�ι���� �������ν� ������ ���� �ؽ��� ���� �����Ҽ��ִ�.
			*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
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
	std::vector<std::shared_ptr<Bindable>> bindablePtrs;

	using namespace std::string_literals;
	const auto base = "Models\\brick_wall\\"s;

	//����ŧ�� ���� �ִ��� Ȯ���Ѵ�.
	bool hasSpecularMap = false;
	float shininess = 35.0f;
	if (mesh.mMaterialIndex >= 0)
	{
		auto& material = *pMaterial[mesh.mMaterialIndex];

		aiString texFileName;
		// ���͸����� �ؽ����̸��� �����´�
		material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName);
		// �ش� �ؽ����̸��� ������� �ؽ��ĸ��ҽ��� ���ε��Ѵ�.
		bindablePtrs.push_back(Texture::Resolve(gfx, base + texFileName.C_Str()));
		// ���͸����� ����ŧ�� ���� �����´�.
		// GetTexutre �� ���ϰ��� ���Ͽ� �ش� ���͸����� ����ŧ�� ���� �ִ��� �˻��Ѵ�. ������츸 ���ε� (�Ϻ� ��ü�� ����ŧ���� ���� �� �ִ�)
		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
		{
			// ����ŧ���� ���ε�. (������ ������ �ؽ��Ĵ� 0�� ����ŧ���� 1�� �����Ѵ�)
			bindablePtrs.push_back(Texture::Resolve(gfx, base + texFileName.C_Str(), 1));
			hasSpecularMap = true;
		}
		else
			material.Get(AI_MATKEY_SHININESS, shininess);	//�Ű������� 3���������� ��ũ�ΰ� 2���εǾ��ִ�..
		
		material.GetTexture(aiTextureType_NORMALS, 0, &texFileName);
		bindablePtrs.push_back(Texture::Resolve(gfx, base + texFileName.C_Str(), 2));

		// ���÷� ���ε�.
		bindablePtrs.push_back(Sampler::Resolve(gfx));

	}

	//����,�ε��� ���� UID�� ���� ��� + �޽��̸�
	auto meshTag = base + "%" + mesh.mName.C_Str();
	//������ �غ��� ����,�ε������۸� ����,�±׵� ����
	bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

	bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

	//�������̴��� �ҷ��´� ���� ����.
	auto pvs = VertexShader::Resolve(gfx, "PhongVSNormalMap.cso");
	auto pvsbc = pvs->GetBytecode();
	bindablePtrs.push_back(std::move(pvs));

	//�������۷κ��� �Է·��̾ƿ��� �����Ѵ�.
	bindablePtrs.push_back(std::make_shared<Bind::InputLayout>(gfx, vbuf.GetLayout(), pvsbc));

	//����ŧ�� ����������� �ش��ϴ� ���̴��� ���ε��Ѵ�.
	if (hasSpecularMap)
	{	
		bindablePtrs.push_back(std::make_shared<Bind::PixelShader>(gfx, "PhongPSSpecNormalMap.cso"));

		struct PSMaterialConstant
		{
			BOOL normalMapEnabled = TRUE;
			float padding[3];
		}pmc;
		bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, pmc, 1u));
	}
	else
	{
		bindablePtrs.push_back(std::make_shared<Bind::PixelShader>(gfx, "PhongPSNormalMap.cso"));

		//�Ƚ����̴� ������ۿ� �����ϱ����� ����ü
		struct PSMaterialConstant
		{
			//DirectX::XMFLOAT3 color = { 0.6f,0.6f,0.8f };
			float specularIntensity = 0.8f;
			float specularPower;
			BOOL  normalMapEnabled  = TRUE;
			float padding[1];				//�ؽ�ó��ǥ�� �е����� �ִµ� ������ ���� �𸣰ڴ�.
		} pmc;
		pmc.specularPower = shininess;
		//�ش� ������۸� �Ƚ����̴� ����1�� ������ �ش� ���δ��� ��ü�� �����̳ʿ� ����
		bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstant>::Resolve(gfx, pmc, 1u));
	}
	
	// �̸�� ������ ��ġ�� �ش� 3d ������Ʈ�� �Ľ��Ͽ� �����Ϸ�Ǿ���.
	// mesh ��ü�� ����ũ�����ͷ� �Ҵ��Ͽ� ���ε������̳ʿ��� ���������ο� ���ε��� �����Ѵ�.
	return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
}
std::unique_ptr<Node> Model::ParseNode(int& nextId,const aiNode& node) noexcept
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
	auto pNode = std::make_unique<Node>(nextId++,node.mName.C_Str(),std::move(curMeshPtrs), transform);
	//���� ��忡 �ڽ��� ������� �ڽ� ������ŭ �߰��Ѵ�.
	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		//���⼭ ���ȣ��� �ڽĿ����� ��������� �����ϰ� �ɰ��̴�.
		pNode->AddChild(ParseNode(nextId,*node.mChildren[i]));
	}

	return pNode;
}
