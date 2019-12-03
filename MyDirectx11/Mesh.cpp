#include "Mesh.h"
#include "imgui/imgui.h"
#include "Surface.h"
#include <unordered_map>
#include <sstream>
#include <filesystem>
#include "BsXM.h"

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
const DirectX::XMFLOAT4X4& Node::GetAppliedTransform() const noexcept
{
	return appliedTransform;
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
	void Show(Graphics& gfx,const char* windowName, const Node& root) noexcept
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
				const auto id = pSelectedNode->GetId();
				auto i = transforms.find(id);
				if (i == transforms.end())
				{
					const auto& applied = pSelectedNode->GetAppliedTransform();
					const auto angles = ExtractEulerAngles(applied);
					const auto translation = ExtractTranslation(applied);
					TransformParameters tp;
					tp.roll = angles.z;
					tp.pitch = angles.x;
					tp.yaw = angles.y;
					tp.x = translation.x;
					tp.y = translation.y;
					tp.z = translation.z;
					std::tie(i, std::ignore) = transforms.insert({ id,tp });
				}
				auto& transform = i->second;
				ImGui::NextColumn();
				ImGui::Text("Orientation");
				ImGui::SliderAngle("Roll", &transform.roll, -180.0f, 180.0f);
				ImGui::SliderAngle("Pitch", &transform.pitch, -180.0f, 180.0f);
				ImGui::SliderAngle("Yaw", &transform.yaw, -180.0f, 180.0f);
				ImGui::Text("Position");
				ImGui::SliderFloat("X", &transform.x, -20.0f, 20.0f);
				ImGui::SliderFloat("Y", &transform.y, -20.0f, 20.0f);
				ImGui::SliderFloat("Z", &transform.z, -20.0f, 20.0f);

				if (!pSelectedNode->ConrolMesh(gfx, skinMaterial))
				{
					pSelectedNode->ConrolMesh(gfx, ringMaterial);
				}

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
	Node::PSMaterialConstantFullmonte skinMaterial;
	Node::PSMaterialConstantNotex ringMaterial;
	std::unordered_map<int, TransformParameters> transforms;	//�� gui ���� �ε����� �ش��ϴ� ����� Ʈ�������� �����Ѵ�.
};

// Model Ŭ����
/*
	3d ������Ʈ�� �ҷ����� �޽�,���� ����� �����Ѵ�
*/
//3d ���� assimp�� �ҷ��´�.
Model::Model(Graphics& gfx, const std::string& pathString,const float scale)
	:pWindow(std::make_unique<ModelWindow>())
{
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile(pathString.c_str(),
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
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials,pathString,scale));
	
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
void Model::ShowWindow(Graphics& gfx, const char* windowName) noexcept
{
	pWindow->Show(gfx,windowName, *pRoot);
}
void Model::SetRootTransform(DirectX::FXMMATRIX tf) noexcept
{
	pRoot->SetAppliedTransform(tf);
}
Model::~Model() noexcept
{
}
// ������Ʈ�� �޽��� �����Ͽ� ������ �����Ѵ�.
std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterial, const std::filesystem::path& path,float scale)
{
	//aiMesh�� ���״�� ���������� �ε����� �̷���� ���ϱ����̴�.
	using namespace std::string_literals;
	using namespace Bind;
	using Dvtx::VertexLayout;

	//�� ���.
	const auto rootPath = path.parent_path().string() + "\\";

	std::vector<std::shared_ptr<Bindable>> bindablePtrs;


	bool hasSpecularMap = false;
	bool hasAlphaGloss = false;
	bool hasNormalMap = false;
	bool hasDiffuseMap = false;
	float shininess = 2.0f;
	dx::XMFLOAT4 specularColor = { 0.18f,0.18f,0.18f,1.0f };
	dx::XMFLOAT4 diffuseColor = { 0.45f,0.45f,0.85f,1.0f };

	if (mesh.mMaterialIndex >= 0)
	{
		auto& material = *pMaterial[mesh.mMaterialIndex];
		aiString texFileName;

		//���͸��󿡼� Diffuse �ؽ�ó ������ �����´�
		if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
		{
			//�ش� �ؽ��� Ǫ��
			bindablePtrs.push_back(Texture::Resolve(gfx, rootPath + texFileName.C_Str()));
			hasDiffuseMap = true;
		}
		else
			//�ؽ�ó�� ���ٸ� ���͸��󿡼� ������ �����´�.
			material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));

		//����ŧ�� �� ������ �����´�
		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
		{
			auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 1u);
			hasAlphaGloss = tex->HasAlhpa();
			bindablePtrs.push_back(std::move(tex));
			hasSpecularMap = true;
		}
		else
			//����ŧ�� ���� ���ٸ�.
			
		//������ ����ŧ���ʿ� ���İ��̾������ �⺻ ������
		if(!hasAlphaGloss)
			material.Get(AI_MATKEY_SHININESS, shininess);
		
		//��� �� ������ �����´�.
		if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
		{
			auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 2u);
			hasAlphaGloss = tex->HasAlhpa();
			bindablePtrs.push_back(std::move(tex));
			hasNormalMap = true;
		}
		//3�� �ϳ��� ������ ���÷� �߰�.
		if (hasDiffuseMap || hasSpecularMap || hasNormalMap)
			bindablePtrs.push_back(Bind::Sampler::Resolve(gfx));
	}
	//�޽� �±�
	const auto meshTag = rootPath + "%" + mesh.mName.C_Str();

	if (hasDiffuseMap && hasNormalMap && hasSpecularMap)
	{
		//���� ���̾ƿ��� �����Ѵ� (��ġ,���)
		Dvtx::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Tangent)
			.Append(VertexLayout::Bitangent)
			.Append(VertexLayout::Texture2D)		//�ؽ�����ǥ�������� �������̾ƿ� �߰�����.
		));
		//����������ŭ �������ۿ� �����Ѵ�.
		//�������ۿ� ���� ���̾ƿ��� �����Ͽ����Ƿ� �ش� ���̾ƿ���� ���Եȴ�.
		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
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
		//�������� �߰�
		bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));
		//���ι��� �߰�
		bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

		auto pvs = VertexShader::Resolve(gfx, "PhongVSNormalMap.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		bindablePtrs.push_back(PixelShader::Resolve(gfx, "PhongPSSpecNormalMap.cso"));
		bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

		Node::PSMaterialConstantFullmonte pmc;

		pmc.specularPower = shininess;
		pmc.hasGlossMap = hasAlphaGloss ? TRUE : FALSE;
		bindablePtrs.push_back(PixelConstantBuffer<Node::PSMaterialConstantFullmonte>::Resolve(gfx, pmc, 1u));
	}
	//��ǻ��,��ָʸ� �ִ°��
	else if (hasDiffuseMap && hasNormalMap)
	{
		Dvtx::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Tangent)
			.Append(VertexLayout::Bitangent)
			.Append(VertexLayout::Texture2D)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		std::vector<unsigned short> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
		bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

		bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

		auto pvs = VertexShader::Resolve(gfx, "PhongVSNormalMap.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		bindablePtrs.push_back(PixelShader::Resolve(gfx, "PhongPSNormalMap.cso"));

		bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

		//����ŧ���� ������ ����ŧ�� ������ �¼����� ��������Ѵ�.
		struct PSMaterialConstantDiffnorm
		{
			float specularIntensity;
			float specularPower;
			BOOL  normalMapEnabled = TRUE;
			float padding[1];
		} pmc;
		pmc.specularPower = shininess;
		pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;

		bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstantDiffnorm>::Resolve(gfx, pmc, 1u));
	}
	//��ǻ��ʰ� ����ŧ������ ������ ��ָ��� ���°��
	else if (hasDiffuseMap && !hasNormalMap && hasSpecularMap)
	{
	Dvtx::VertexBuffer vbuf(std::move(
		VertexLayout{}
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		.Append(VertexLayout::Texture2D)
	));

	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		vbuf.EmplaceBack(
			dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
			*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
		);
	}

	std::vector<unsigned short> indices;
	indices.reserve(mesh.mNumFaces * 3);
	for (unsigned int i = 0; i < mesh.mNumFaces; i++)
	{
		const auto& face = mesh.mFaces[i];
		assert(face.mNumIndices == 3);
		indices.push_back(face.mIndices[0]);
		indices.push_back(face.mIndices[1]);
		indices.push_back(face.mIndices[2]);
	}

	bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

	bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

	auto pvs = VertexShader::Resolve(gfx, "PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();
	bindablePtrs.push_back(std::move(pvs));

	bindablePtrs.push_back(PixelShader::Resolve(gfx, "PhongPSSpec.cso"));

	bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

	struct PSMaterialConstantDiffuseSpec
	{
		float specularPowerConst;
		BOOL hasGloss;
		float specularMapWeight;
		float padding;
	} pmc;
	pmc.specularPowerConst = shininess;
	pmc.hasGloss = hasAlphaGloss ? TRUE : FALSE;
	pmc.specularMapWeight = 1.0f;
	// this is CLEARLY an issue... all meshes will share same mat const, but may have different
	// Ns (specular power) specified for each in the material properties... bad conflict
	bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstantDiffuseSpec>::Resolve(gfx, pmc, 1u));
	}
	//��ǻ� �ִ°��
	else if (hasDiffuseMap)
	{
		Dvtx::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Texture2D)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}

		std::vector<unsigned short> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

		bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

		auto pvs = VertexShader::Resolve(gfx, "PhongVS.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		bindablePtrs.push_back(PixelShader::Resolve(gfx, "PhongPS.cso"));

		bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

		struct PSMaterialConstantDiffuse
		{
			float specularIntensity;
			float specularPower;
			float padding[2];
		} pmc;
		pmc.specularPower = shininess;
		pmc.specularIntensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;

		bindablePtrs.push_back(PixelConstantBuffer<PSMaterialConstantDiffuse>::Resolve(gfx, pmc, 1u));
	}
	//3�� �������
	else if (!hasDiffuseMap && !hasNormalMap && !hasSpecularMap)
	{
		Dvtx::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
		));

		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i])
			);
		}

		std::vector<unsigned short> indices;
		indices.reserve(mesh.mNumFaces * 3);
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}

		bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));

		bindablePtrs.push_back(IndexBuffer::Resolve(gfx, meshTag, indices));

		auto pvs = VertexShader::Resolve(gfx, "PhongVSNotex.cso");
		auto pvsbc = pvs->GetBytecode();
		bindablePtrs.push_back(std::move(pvs));

		bindablePtrs.push_back(PixelShader::Resolve(gfx, "PhongPSNotex.cso"));

		bindablePtrs.push_back(InputLayout::Resolve(gfx, vbuf.GetLayout(), pvsbc));

		//�ؽ�ó�� �������
		Node::PSMaterialConstantNotex pmc;
		pmc.specularPower = shininess;
		pmc.specularColor = specularColor;
		pmc.materialColor = diffuseColor;

		bindablePtrs.push_back(PixelConstantBuffer<Node::PSMaterialConstantNotex>::Resolve(gfx, pmc, 1u));
	}
	else
		throw std::runtime_error("���͸��� �� �ؽ��������� �߸��Ǿ����ϴ�");

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
