#pragma once
#include "DrawableBase.h"
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
class Mesh : public DrawableBase<Mesh>
{
public:
	Mesh(Graphics& gfx, std::vector<std::unique_ptr<Bind::Bindable>> bindPtrs);
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
	friend class ModelWindow;			//���������� ��ɿ� �����Ѵ�.
public:
	Node(const std::string& name,std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd;
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	void SetAppliedTransform(DirectX::FXMMATRIX transform) noexcept;	//Ʈ�������� �����Ѵ�.
private:
	void AddChild(std::unique_ptr<Node> pChild) noxnd;
	// �ε����� ���� ���� ��忡 �����Ҽ� �ִ�.
	void ShowTree(int& nodeIndexTracked, std::optional<int>& selectedIndex,Node*& pSelectedNode) const noexcept;
	//nodexIdex�� ��庰 ������ �ε����� �����Ѵ�
private:
	std::string name;								//��庰 �̸��߰�.
	std::vector<std::unique_ptr<Node>> childPtrs;	//�ڽ� ���
	std::vector<Mesh*> meshPtrs;					//��� �޽���
	DirectX::XMFLOAT4X4 transform;					//��� Ʈ������

	DirectX::XMFLOAT4X4 baseTransform;				//���Ͽ��� ���� Ʈ������
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
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);
	std::unique_ptr<Node> ParseNode(const aiNode& node) noexcept;
private:
	std::unique_ptr<Node> pRoot;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
	std::unique_ptr<class ModelWindow> pWindow;
};