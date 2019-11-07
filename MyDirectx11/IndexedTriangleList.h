#pragma once

#include <vector>
#include <DirectXMath.h>

template<class T>
class IndexedTriangleList
{
public:
	IndexedTriangleList() = default;
	IndexedTriangleList(std::vector<T> verts_in, std::vector<unsigned short> indice_in)
		:vertices(std::move(verts_in)), indices(std::move(indices_in))
	{
		assert(vertices.size() > 2);
		assert(indices.size() % 3 == 0);
	}
	void Transform(Directx::FXMMATRIX matrix)
	{
		for (auto& v : vertices)
		{
			const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.pos);
			DirectX::XMStoreFloat3(
				&v.pos, DirectX::XMVector3Transform(pos, matrix)
			);
		}
	}
public:
	std::vector <T> vertices;
	std::vector<unsigned short> indices;

};