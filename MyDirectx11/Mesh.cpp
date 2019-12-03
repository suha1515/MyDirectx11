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
//생성자에서 바인드 가능한 객체들의 포인터를 넘기는데 이때 인덱스버퍼는 따로 구분하여
//AddIndexBuffer로 인덱스버퍼 바인딩을 수행한다.하지만 유니크 포인터를 사용하고 있으므로 
//생성자 인수로 유니크포인터 벡터로 받으므로 따로 pi를 유니크 포인터로 전달해야하므로 오직 1개만 유니크포인트가
//가르키고 있어야하므로 나머지 한개는 Release 해주어야 한다
//바인드 포인터들을 인덱스버퍼 포인터로 캐스팅한다.
Mesh::Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>> bindPtrs)
{
		AddBind(Bind::Topology::Resolve(gfx, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST));

	for (auto& pb : bindPtrs)
	{
			AddBind(std::move(pb));
	}
	//정점쉐이더에서 사용할 트랜스폼 바인딩
	AddBind(std::make_shared<Bind::TransformCbuf>(gfx, *this));
}
// 그리기를 담당하는 함수이다.
	// 인자 행렬은 노드시스템에서 자식일경우 부모로부터의 행렬이 축적되어 사용되기에 accumlatedTransform 이다.
void Mesh::Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd
{
	DirectX::XMStoreFloat4x4(&transform, accumulatedTransform);
	Drawable::Draw(gfx);
}
DirectX::XMMATRIX Mesh::GetTransformXM() const noexcept
{
	return DirectX::XMLoadFloat4x4(&transform);
}


// Node 클래스(트리구조)
/*
	노드는 즉 각 메쉬들이 하나의 기하구조를 구성하고있다면 노드는 그 기하구조의 중심이라고 생각하면 될것이다.
	노드는 메쉬들의 포인터들을 가지고있어 Draw 함수로 자신의 트랜스폼으로 자신의 노드 메쉬들에 전달하여 그린다.
	노드는 1개이상의 자식을 가지고있으며 해당 자식들도 노드의 트랜스폼을받고 그린다.
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
//축적된 행렬을 받아서 자신의 트랜스폼과 곱하여 메쉬들과 자식들에 전달하여 그린다.
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
	//선택된 노드가 없을경우 선택된 ID 값을 -1로 지정한다.
	const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();
	// 최근 노드를 위해 플래그를 세운다.
	const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow |
		((GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0) |
		((childPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);
	//만약 노드가 확장되면 재귀로 자식들을 그린다.
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
//자식노드를 추가하는 코드
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
				  //pimple 이란 컴파일 의존성을 줄이기 위한 하나의 설계 기법이다.
{
public:
	void Show(Graphics& gfx,const char* windowName, const Node& root) noexcept
	{
		// window name defaults to "Model"
		windowName = windowName ? windowName : "Model";
		//노드 인덱스들과 선택된 노드를 추적하기위해 필요한 변수
		int nodeIndexTracker = 0;
		if (ImGui::Begin(windowName))
		{
			ImGui::Columns(2, nullptr, true);
			root.ShowTree(pSelectedNode);
			if (pSelectedNode != nullptr)//선택된 노드가 있을경우만 gui 표시
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
	std::optional<int> selectedIndex;	//선택되었는지 되었으면 해당 인덱스반환아니면 false
	Node* pSelectedNode;				//선택된노드
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
	std::unordered_map<int, TransformParameters> transforms;	//모델 gui 에서 인덱스에 해당하는 노드의 트랜스폼을 저장한다.
};

// Model 클래스
/*
	3d 오브젝트를 불러오고 메쉬,노드로 나누어서 보관한다
*/
//3d 모델을 assimp로 불러온다.
Model::Model(Graphics& gfx, const std::string& pathString,const float scale)
	:pWindow(std::make_unique<ModelWindow>())
{
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile(pathString.c_str(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices|
		aiProcess_ConvertToLeftHanded|	//왼손좌표계로
		aiProcess_GenNormals|			//노멀생성
		aiProcess_CalcTangentSpace		//탄젠트 계산
	);

	if (pScene == nullptr)
		throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
	//assimp에서는 Scene이라고 입력된 데이터의 루트 구조이다.
	//모든 구조는 Scene에서 접근한다.

	//해당 오브젝트의 매쉬를 벡터에 삽입,aiScene에 있는 머터리얼배열 도 같이 파싱한다.
	for (size_t i = 0; i < pScene->mNumMeshes; ++i)
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials,pathString,scale));
	
	//루트노드를 ParseNode에 전달한다.
	int nextId = 0;
	pRoot = ParseNode(nextId ,*pScene->mRootNode);
}

//그리기 작업을 수행한다.
void Model::Draw(Graphics& gfx) const noxnd
{
	if (auto node = pWindow->GetSelectedNode())
	{
		node->SetAppliedTransform(pWindow->GetTransform());
	}
	pRoot->Draw(gfx, dx::XMMatrixIdentity());
}
// 모델별 ImGui 구성
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
// 오브젝트의 메쉬에 접근하여 정보를 가공한다.
std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh, const aiMaterial* const* pMaterial, const std::filesystem::path& path,float scale)
{
	//aiMesh는 말그대로 여러정점과 인덱스로 이루어진 기하구조이다.
	using namespace std::string_literals;
	using namespace Bind;
	using Dvtx::VertexLayout;

	//모델 경로.
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

		//머터리얼에서 Diffuse 텍스처 정보를 가져온다
		if (material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName) == aiReturn_SUCCESS)
		{
			//해당 텍스쳐 푸쉬
			bindablePtrs.push_back(Texture::Resolve(gfx, rootPath + texFileName.C_Str()));
			hasDiffuseMap = true;
		}
		else
			//텍스처가 없다면 머터리얼에서 색깔값을 가져온다.
			material.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D&>(diffuseColor));

		//스페큘러 맵 정보를 가져온다
		if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
		{
			auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 1u);
			hasAlphaGloss = tex->HasAlhpa();
			bindablePtrs.push_back(std::move(tex));
			hasSpecularMap = true;
		}
		else
			//스페큘러 맵이 없다면.
			
		//가져온 스페큘러맵에 알파값이없을경우 기본 값으로
		if(!hasAlphaGloss)
			material.Get(AI_MATKEY_SHININESS, shininess);
		
		//노멀 맵 정보를 가져온다.
		if (material.GetTexture(aiTextureType_NORMALS, 0, &texFileName) == aiReturn_SUCCESS)
		{
			auto tex = Texture::Resolve(gfx, rootPath + texFileName.C_Str(), 2u);
			hasAlphaGloss = tex->HasAlhpa();
			bindablePtrs.push_back(std::move(tex));
			hasNormalMap = true;
		}
		//3중 하나라도 있으면 샘플러 추가.
		if (hasDiffuseMap || hasSpecularMap || hasNormalMap)
			bindablePtrs.push_back(Bind::Sampler::Resolve(gfx));
	}
	//메쉬 태그
	const auto meshTag = rootPath + "%" + mesh.mName.C_Str();

	if (hasDiffuseMap && hasNormalMap && hasSpecularMap)
	{
		//동적 레이아웃을 지정한다 (위치,노멀)
		Dvtx::VertexBuffer vbuf(std::move(
			VertexLayout{}
			.Append(VertexLayout::Position3D)
			.Append(VertexLayout::Normal)
			.Append(VertexLayout::Tangent)
			.Append(VertexLayout::Bitangent)
			.Append(VertexLayout::Texture2D)		//텍스쳐좌표값을위해 동적레이아웃 추가설정.
		));
		//정점개수만큼 정점버퍼에 삽입한다.
		//정점버퍼에 동적 레이아웃을 지정하였으므로 해당 레이아웃대로 삽입된다.
		for (unsigned int i = 0; i < mesh.mNumVertices; i++)
		{
			vbuf.EmplaceBack(
				dx::XMFLOAT3(mesh.mVertices[i].x * scale, mesh.mVertices[i].y * scale, mesh.mVertices[i].z * scale),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mTangents[i]),		//탄젠트
				*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mBitangents[i]),		//바이탄젠트
				//assimp에서 텍스처좌표가 이중배열로이루어졌는데 텍스쳐좌표를위해 8개의 채널을 가지고 있다.
				//아래는 첫번째 채널이다.두번재는 정점으로써 정점은 여러 텍스를 따로 저장할수있다.
				*reinterpret_cast<dx::XMFLOAT2*>(&mesh.mTextureCoords[0][i])
			);
		}
		// 메쉬들을 aiProcess_Triangulate 즉 삼각형 구조로 받는다
		// 모든 인덱스는 면의개수*3 을하면 총인덱스버퍼의 크기를 구할 수 있다.
		std::vector<unsigned short> indices;
		indices.reserve(mesh.mNumFaces * 3);

		//면의 개수만큼 접근하여 각면의 인덱스정보를 버퍼에 삽입한다. 인덱스점이 3개이상이면 예외
		for (unsigned int i = 0; i < mesh.mNumFaces; i++)
		{
			const auto& face = mesh.mFaces[i];
			assert(face.mNumIndices == 3);
			indices.push_back(face.mIndices[0]);
			indices.push_back(face.mIndices[1]);
			indices.push_back(face.mIndices[2]);
		}
		//정점버퍼 추가
		bindablePtrs.push_back(VertexBuffer::Resolve(gfx, meshTag, vbuf));
		//색인버퍼 추가
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
	//디퓨즈,노멀맵만 있는경우
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

		//스페큘러가 없으니 스페큘러 강도와 승수등을 정해줘야한다.
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
	//디퓨즈맵과 스페큘러맵은 있지만 노멀맵은 없는경우
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
	//디퓨즈만 있는경우
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
	//3다 없을경우
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

		//텍스처도 없을경우
		Node::PSMaterialConstantNotex pmc;
		pmc.specularPower = shininess;
		pmc.specularColor = specularColor;
		pmc.materialColor = diffuseColor;

		bindablePtrs.push_back(PixelConstantBuffer<Node::PSMaterialConstantNotex>::Resolve(gfx, pmc, 1u));
	}
	else
		throw std::runtime_error("머터리얼 의 텍스쳐조합이 잘못되었습니다");

	// 이모든 과정을 거치면 해당 3d 오브젝트를 파싱하여 가공완료되었다.
	// mesh 객체를 유니크포인터로 할당하여 바인딩컨테이너에서 파이프라인에 바인딩을 진행한다.
	return std::make_unique<Mesh>(gfx, std::move(bindablePtrs));
}
std::unique_ptr<Node> Model::ParseNode(int& nextId,const aiNode& node) noexcept
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
	for (size_t i = 0; i < node.mNumMeshes; i++)
	{
		const auto meshIdx = node.mMeshes[i];
		curMeshPtrs.push_back(meshPtrs.at(meshIdx).get());
	}
	//구성된 메쉬 컨테이너와함께 트랜스폼으로 노드객체를 할당한다.
	auto pNode = std::make_unique<Node>(nextId++,node.mName.C_Str(),std::move(curMeshPtrs), transform);
	//만약 노드에 자식이 있을경우 자식 개수만큼 추가한다.
	for (size_t i = 0; i < node.mNumChildren; i++)
	{
		//여기서 재귀호출로 자식에서도 노드정보를 구성하게 될것이다.
		pNode->AddChild(ParseNode(nextId,*node.mChildren[i]));
	}

	return pNode;
}
