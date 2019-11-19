#pragma once
#include "Drawable.h"
#include "BindableCommon.h"
#include "Vertex.h"
#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ConditionalNoexcept.h"


//모델 불러오기 예외처리 클래스
class ModelException : public BsException
{
public:
	ModelException(int line, const char* file, std::string note) noexcept;
	const char* what() const noexcept override;
	const char* GetType() const noexcept override;
	const std::string& GetNote() const noexcept;
private:
	std::string note;
};


// Mesh 클래스
/*
	모델의 메쉬를 나타내는 클래스이다.
*/
class Mesh : public Drawable
{
public:
	Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>> bindPtrs);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
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
	Node(int id,const std::string& name,std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd;
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;	//트랜스폼을 적용한다.
	int GetId()const noexcept;
	void ShowTree(Node*& pSelectedNode) const noexcept;
private:
	void AddChild(std::unique_ptr<Node> pChild) noxnd;
	// 인덱스를 통해 직접 노드에 접근할수 있다.
	
private:
	std::string name;								//노드별 이름추가.
	int	id;											//노드별 id
	std::vector<std::unique_ptr<Node>> childPtrs;	//자식 노드
	std::vector<Mesh*> meshPtrs;					//노드 메쉬들
	DirectX::XMFLOAT4X4 transform;					//파일에서 나온 트랜스폼
	DirectX::XMFLOAT4X4 appliedTransform;			//모델윈도우에서 적용되는 트랜스폼
};

// Model 클래스
/*
	3d 오브젝트를 불러오고 메쉬,노드로 나누어서 보관한다
*/
class Model
{
public:
	Model(Graphics& gfx, const std::string fileName);
	void Draw(Graphics& gfx) const noxnd;
	void ShowWindow(const char* windowname = nullptr) noexcept;
	~Model() noexcept;	//소멸자 구성을 하지않으면 전방선언타입으로 유니크 포인터를 만들수 없다는데 왜그럴까..
private:

	// const aiMaterial* const* pMaterial 와 같은.. 구조는
	// 흔치는 않지만 일단 const aiMaterial* const 는 const 객체를 가르키는 const 포인터이다
	// 여기서 또뒤에 * 가들어간이유는.. 일단 .aiMaterial은 이중포인터이고 머터리얼 배열을 가르킨다.
	// 즉 머터리얼의 첫주소가 들어가야하는데 설명으로는 상수가아닌 포인터를 상수포인터에넘기며 그 상수가아닌 포인터는 배열을 가르킨다..? 일단 나중에 자세히 알아보자.
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh,const aiMaterial* const* pMaterial);
	std::unique_ptr<Node> ParseNode(int& nexId,const aiNode& node) noexcept;
private:
	std::unique_ptr<Node> pRoot;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
	std::unique_ptr<class ModelWindow> pWindow;
};