#pragma once
#include "DrawableBase.h"
#include "BindableCommon.h"
#include "Vertex.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ConditionalNoexcept.h"

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
public:
	Node(const std::string& name,std::vector<Mesh*> meshPtrs, const DirectX::XMMATRIX& transform) noxnd;
	void Draw(Graphics& gfx, DirectX::FXMMATRIX accumulatedTransform) const noxnd;
	void RenderTree() const noexcept;
private:
	void AddChild(std::unique_ptr<Node> pChild) noxnd;
private:
	std::string name;								//��庰 �̸��߰�.
	std::vector<std::unique_ptr<Node>> childPtrs;	//�ڽ� ���
	std::vector<Mesh*> meshPtrs;					//��� �޽���
	DirectX::XMFLOAT4X4 transform;					//��� Ʈ������
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
private:
	static std::unique_ptr<Mesh> ParseMesh(Graphics& gfx, const aiMesh& mesh);
	std::unique_ptr<Node> ParseNode(const aiNode& node) noexcept;
private:
	std::unique_ptr<Node> pRoot;
	std::vector<std::unique_ptr<Mesh>> meshPtrs;
	struct
	{
		float roll = 0.0f;
		float pitch = 0.0f;
		float yaw = 0.0f;
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;
	}pos;
};