#pragma once
#include "Drawable.h"
#include "BindableCommon.h"
#include "Vertex.h"
#include <optional>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ConditionalNoexcept.h"


//�� �ҷ����� ����ó�� Ŭ����
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


// Mesh Ŭ����
/*
	���� �޽��� ��Ÿ���� Ŭ�����̴�.
*/
class Mesh : public Drawable
{
public:
	Mesh(Graphics& gfx, std::vector<std::shared_ptr<Bind::Bindable>> bindPtrs);
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
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
	Node(int id,const std::string& name,std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd;
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;	//Ʈ�������� �����Ѵ�.
	int GetId()const noexcept;
	void ShowTree(Node*& pSelectedNode) const noexcept;
private:
	void AddChild(std::unique_ptr<Node> pChild) noxnd;
	// �ε����� ���� ���� ��忡 �����Ҽ� �ִ�.
	
private:
	std::string name;								//��庰 �̸��߰�.
	int	id;											//��庰 id
	std::vector<std::unique_ptr<Node>> childPtrs;	//�ڽ� ���
	std::vector<Mesh*> meshPtrs;					//��� �޽���
	DirectX::XMFLOAT4X4 transform;					//���Ͽ��� ���� Ʈ������
	DirectX::XMFLOAT4X4 appliedTransform;			//�������쿡�� ����Ǵ� Ʈ������
};

// Model Ŭ����
/*
	3d ������Ʈ�� �ҷ����� �޽�,���� ����� �����Ѵ�
*/
class Model
{
public:
	Model(Graphics& gfx, const std::string fileName);
	void Draw(Graphics& gfx) const noxnd;
	void ShowWindow(const char* windowname = nullptr) noexcept;
	~Model() noexcept;	//�Ҹ��� ������ ���������� ���漱��Ÿ������ ����ũ �����͸� ����� ���ٴµ� �ֱ׷���..
private:

	// const aiMaterial* const* pMaterial �� ����.. ������
	// ��ġ�� ������ �ϴ� const aiMaterial* const �� const ��ü�� ����Ű�� const �������̴�
	// ���⼭ �ǵڿ� * ����������.. �ϴ� .aiMaterial�� �����������̰� ���͸��� �迭�� ����Ų��.
	// �� ���͸����� ù�ּҰ� �����ϴµ� �������δ� ������ƴ� �����͸� ��������Ϳ��ѱ�� �� ������ƴ� �����ʹ� �迭�� ����Ų��..? �ϴ� ���߿� �ڼ��� �˾ƺ���.
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh,const aiMaterial* const* pMaterial);
	std::unique_ptr<Node> ParseNode(int& nexId,const aiNode& node) noexcept;
private:
	std::unique_ptr<Node> pRoot;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
	std::unique_ptr<class ModelWindow> pWindow;
};