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
//생성자에서 바인드 가능한 객체들의 포인터를 넘기는데 이때 인덱스버퍼는 따로 구분하여
//AddIndexBuffer로 인덱스버퍼 바인딩을 수행한다.하지만 유니크 포인터를 사용하고 있으므로 
//생성자 인수로 유니크포인터 벡터로 받으므로 따로 pi를 유니크 포인터로 전달해야하므로 오직 1개만 유니크포인트가
//가르키고 있어야하므로 나머지 한개는 Release 해주어야 한다
//바인드 포인터들을 인덱스버퍼 포인터로 캐스팅한다.
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
		else//인덱스버퍼 그외의 것은 Bind를 호출한다.
		{
			AddBind(std::move(pb));
		}
	}
	//정점쉐이더에서 사용할 트랜스폼 바인딩
	AddBind(std::make_unique<Bind::TransformCbuf>(gfx, *this));
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
	void Show(const char* windowName, const Node& root) noexcept
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
	std::unordered_map<int, TransformParameters> transforms;	//모델 gui 에서 인덱스에 해당하는 노드의 트랜스폼을 저장한다.
};

// Model 클래스
/*
	3d 오브젝트를 불러오고 메쉬,노드로 나누어서 보관한다
*/
//3d 모델을 assimp로 불러온다.
Model::Model(Graphics& gfx, const std::string fileName)
	:pWindow(std::make_unique<ModelWindow>())
{
	Assimp::Importer imp;
	const auto pScene = imp.ReadFile(fileName.c_str(),
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices|
		aiProcess_ConvertToLeftHanded|
		aiProcess_GenNormals
	);

	if (pScene == nullptr)
		throw ModelException(__LINE__, __FILE__, imp.GetErrorString());
	//assimp에서는 Scene이라고 입력된 데이터의 루트 구조이다.
	//모든 구조는 Scene에서 접근한다.

	//해당 오브젝트의 매쉬를 벡터에 삽입,aiScene에 있는 머터리얼배열 도 같이 파싱한다.
	for (size_t i = 0; i < pScene->mNumMeshes; ++i)
		meshPtrs.push_back(ParseMesh(gfx, *pScene->mMeshes[i], pScene->mMaterials));

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
void Model::ShowWindow(const char* windowName) noexcept
{
	pWindow->Show(windowName, *pRoot);
}
Model::~Model() noexcept
{
}
// 오브젝트의 메쉬에 접근하여 정보를 가공한다.
std::unique_ptr<Mesh> Model::ParseMesh(Graphics& gfx, const aiMesh& mesh,const aiMaterial* const* pMaterial)
{
	//aiMesh는 말그대로 여러정점과 인덱스로 이루어진 기하구조이다.
	namespace dx = DirectX;
	using Dvtx::VertexLayout;

	//동적 레이아웃을 지정한다 (위치,노멀)
	Dvtx::VertexBuffer vbuf(std::move(
		VertexLayout{}
		.Append(VertexLayout::Position3D)
		.Append(VertexLayout::Normal)
		.Append(VertexLayout::Texture2D)		//텍스쳐좌표값을위해 동적레이아웃 추가설정.
	));

	// 메쉬는 멤버변수로 매터리얼 배열의 인덱스를 가지고있다.
	// 인덱스로 메쉬에 대한 머터리얼에 접근할 수 있다.
	auto& material = *pMaterial[mesh.mMaterialIndex];

	//정점개수만큼 정점버퍼에 삽입한다.
	//정점버퍼에 동적 레이아웃을 지정하였으므로 해당 레이아웃대로 삽입된다.
	for (unsigned int i = 0; i < mesh.mNumVertices; i++)
	{
		vbuf.EmplaceBack(
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mVertices[i]),
			*reinterpret_cast<dx::XMFLOAT3*>(&mesh.mNormals[i]),
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
	//바인드가능한 객체들을 담을 벡터 컨테이너
	std::vector<std::unique_ptr<Bind::Bindable>> bindablePtrs;

	//모든 매쉬가 머터리얼을 가지고 있는것은 아니기때문에
	//머터리얼을 가지지 않는 메쉬는 머터리얼 인덱스가 음수가 나온다.
	if (mesh.mMaterialIndex >= 0)
	{
		auto& material = *pMaterial[mesh.mMaterialIndex];
		using namespace std::string_literals;
		const auto base = "Models\\nano_textured\\"s;
		aiString texFileName;
		// 머터리얼의 텍스쳐이름을 가져온다
		material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName);
		// 해당 텍스쳐이름을 기반으로 텍스쳐리소스를 바인딩한다.
		bindablePtrs.push_back(std::make_unique<Bind::Texture>(gfx, Surface::FromFile(base + texFileName.C_Str())));

		// 머터리얼의 스페큘러 맵을 가져온다.
		material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName);
		// 스페큘러맵 바인딩. (슬롯을 나누어 텍스쳐는 0에 스페큘러는 1로 지정한다)
		bindablePtrs.push_back(std::make_unique<Bind::Texture>(gfx, Surface::FromFile(base + texFileName.C_Str()),1));
		// 샘플러 바인딩.
		bindablePtrs.push_back(std::make_unique<Bind::Sampler>(gfx));

	}

	//위에서 준비한 정점,인덱스버퍼를 삽입
	bindablePtrs.push_back(std::make_unique<Bind::VertexBuffer>(gfx, vbuf));

	bindablePtrs.push_back(std::make_unique<Bind::IndexBuffer>(gfx, indices));

	//정점쉐이더를 불러온다 그후 삽입.
	auto pvs = std::make_unique<Bind::VertexShader>(gfx, L"PhongVS.cso");
	auto pvsbc = pvs->GetBytecode();
	bindablePtrs.push_back(std::move(pvs));

	//픽셀쉐이더를 불러온다 그후 삽입.
	bindablePtrs.push_back(std::make_unique<Bind::PixelShader>(gfx, L"PhongPS.cso"));

	//정점버퍼로부터 입력레이아웃을 삽입한다.
	bindablePtrs.push_back(std::make_unique<Bind::InputLayout>(gfx, vbuf.GetLayout().GetD3DLayout(), pvsbc));

	//픽쉘세이더 상수버퍼에 전달하기위한 구조체
	struct PSMaterialConstant
	{
		//DirectX::XMFLOAT3 color = { 0.6f,0.6f,0.8f };
		float specularIntensity = 0.6f;
		float specularPower = 30.0f;	
		float padding[2];				//텍스처좌표로 패딩값을 넣는데 원리를 아직 모르겠다.
	} pmc;
	//해당 상수버퍼를 픽쉘세이더 슬롯1에 지정후 해당 바인더블 객체를 컨테이너에 삽입
	bindablePtrs.push_back(std::make_unique<Bind::PixelConstantBuffer<PSMaterialConstant>>(gfx, pmc, 1u));

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
