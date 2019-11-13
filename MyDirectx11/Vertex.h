#pragma once
#include <vector>
#include <type_traits>
#include "Graphics.h"

namespace MyVertex
{
	struct BGRAColor
	{
		unsigned char a;
		unsigned char r;
		unsigned char g;
		unsigned char b;
	};

	//VertexLayout
	/*
		�������̾ƿ��� �����ϴ� Ŭ�����̴�.
		�� �츮�� { "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 }
		���̾ƿ��� �����Ѱ�ó�� �ش� ����� ������ Ŭ�����̴�.

		Ȥ�� �� ���̾ƿ��� �ƴ϶��
			struct Vertex
			{
				dx::XMFLOAT3 pos;
				dx::XMFLOAT3 n;
			};
		�̷��� �����ϴ°��� �ٲ۰��ϰ��̴�.
	*/
	class VertexLayout
	{
	public:
		//���� ��������� ������ ��Ÿ���� ����ü�̴�.
		enum ElementType
		{
			Position2D,		//2���� ��ġ
			Position3D,		//3���� ��ġ
			Texture2D,		//�ؽ��� 2D
			Normal,			//�븻 ��
			Float3Color,	//�÷�
			Float4Color,
			BGRAColor,
			Count,
		};
		//���ø� ������� ������ ���̾ƿ��� ���� �����ϰ� �����Ѵ�
		//���ø��� Ư��ȭ�� �̿��Ͽ� Map<ElementType>�� �ش��ϴ� ����ü������� �����Ѵ�.
		//������Ÿ�ӿ� �����ϱ����� static constexpr Ű���带 ����Ͽ���.
		template<ElementType> struct Map;
		template<> struct Map<Position2D>
		{
			using SysType = DirectX::XMFLOAT2;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
			static constexpr const char* semantic = "Position";
		};
		template<> struct Map<Position3D>
		{
			using SysType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "Position";
		};
		template<> struct Map<Texture2D>
		{
			using SysType = DirectX::XMFLOAT2;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32_FLOAT;
			static constexpr const char* semantic = "Texcoord";
		};
		template<> struct Map<Normal>
		{
			using SysType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "Normal";
		};
		template<> struct Map<Float3Color>
		{
			using SysType = DirectX::XMFLOAT3;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32_FLOAT;
			static constexpr const char* semantic = "Color";
		};
		template<> struct Map<Float4Color>
		{
			using SysType = DirectX::XMFLOAT4;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R32G32B32A32_FLOAT;
			static constexpr const char* semantic = "Color";
		};
		template<> struct Map<BGRAColor>
		{
			using SysType = MyVertex::BGRAColor;
			static constexpr DXGI_FORMAT dxgiFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
			static constexpr const char* semantic = "Color";
		};
		class Element
		{
		public:
			Element(ElementType type, size_t offset)
				: type(type), offset(offset)
			{}
			size_t GetOffsetAfter() const noexcept(!IS_DEBUG)
			{
				return offset + Size();
			}
			size_t GetOffset() const
			{
				return offset;
			}
			size_t Size() const noexcept(!IS_DEBUG)
			{
				return SizeOf(type);
			}

			static constexpr size_t SizeOf(ElementType type) noexcept(!IS_DEBUG)
			{
				//������ ����ġ���� �ٸ��� ���ø��� Ư��ȭ������
				//���ϴ� ���̾ƿ��� ������ ���ø�ȭ�Ͽ� �ش� Ÿ���������Ѵ�
				//�̷���� ���� �ý���Ÿ���� ������ �����ʿ��Ͼ��� �Ǽ��� ������ �پ���.
				using namespace DirectX;
				switch (type)
				{
				case Position2D:
					return sizeof(Map<Position2D>::SysType);
				case Position3D:
					return sizeof(Map<Position3D>::SysType);
				case Texture2D:
					return sizeof(Map<Texture2D>::SysType);
				case Normal:
					return sizeof(Map<Normal>::SysType);
				case Float3Color:
					return sizeof(Map<Float3Color>::SysType);
				case Float4Color:
					return sizeof(Map<Float4Color>::SysType);
				case BGRAColor:
					return sizeof(Map<BGRAColor>::SysType);
				}
				assert("Invalid element type" && false);
				return 0u;
			}
			ElementType GetType() const noexcept
			{
				return type;
			}
			//�������� D3D11 layout�� ��ȯ�Ѵ�.
			//�� �ش� ����ü(�ý��� Ÿ��) �� ���� ����,�ø�ƽ,�����°� �뵵�� �ڵ����� ����ü�� ��ȯ�Ѵ�.
			D3D11_INPUT_ELEMENT_DESC GetDesc() const noexcept(!IS_DEBUG)
			{
				switch (type)
				{
				case Position2D:
					return GenerateDesc<Position2D>(GetOffset());
				case Position3D:
					return GenerateDesc<Position3D>(GetOffset());
				case Texture2D:
					return GenerateDesc<Texture2D>(GetOffset());
				case Normal:
					return GenerateDesc<Normal>(GetOffset());
				case Float3Color:
					return GenerateDesc<Float3Color>(GetOffset());
				case Float4Color:
					return GenerateDesc<Float4Color>(GetOffset());
				case BGRAColor:
					return GenerateDesc<BGRAColor>(GetOffset());
				}
				assert("Invalid element type" && false);
				return { "INVALID",0,DXGI_FORMAT_UNKNOWN,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 };
			}
		private:
			template<ElementType type>
			static constexpr D3D11_INPUT_ELEMENT_DESC GenerateDesc(size_t offset) noexcept(!IS_DEBUG)
			{
				return { Map<type>::semantic,0,Map<type>::dxgiFormat,0,(UINT)offset,D3D11_INPUT_PER_VERTEX_DATA,0 };
			}
		private:
			ElementType type;		//������ ������
			size_t  offset;			//�ش� ������ ������
		};
	public:
		//���� ���Ҹ� �����ϴ� �Լ�. ���ø� Ÿ������ �Ѱ� �ش� Ÿ���� ��������� ��ȯ�Ѵ�.
		template<ElementType Type>
		const Element& Resolve() const noexcept(!IS_DEBUG)
		{
			for (auto& e : elements)
			{
				if (e.GetType() == Type)
				{
					return e;
				}
			}
			assert("Could not resolve element type" && false);
			return elements.front();
		}
		//�� �Լ��� ����� �Ȱ����� �ε��� ������� ��ȯ�Ѵ�.
		const Element& ResolveByIndex(size_t i) const noexcept(!IS_DEBUG)
		{
			return elements[i];
		}
		//������ ���ø��� Ÿ���� ������� �����Ͽ�����
		//������ Enum ����ü�� ����
		VertexLayout& Append(ElementType type) noexcept(!IS_DEBUG)
		{
			elements.emplace_back(type, Size());
			return *this;
		}

		//��ü ������ ũ�⸦ ��ȯ�ϴ� �Լ�.
		size_t Size() const noexcept(!IS_DEBUG)
		{
			return elements.empty() ? 0u : elements.back().GetOffsetAfter();
		}
		//������ ������ ��ȯ�Ѵ�.
		size_t GetElementCount() const noexcept
		{
			return elements.size();
		}
		//���ҵ��� ���̾ƿ��� ���ͷ� ��ȯ�Ѵ�.
		std::vector<D3D11_INPUT_ELEMENT_DESC> GetD3DLayout() const noexcept(!IS_DEBUG)
		{
			std::vector<D3D11_INPUT_ELEMENT_DESC> desc;
			desc.reserve(GetElementCount());
			for (const auto& e : elements)
			{
				desc.push_back(e.GetDesc());
			}
			return desc;
		}
	private:
		std::vector<Element> elements; //���� ���Ҹ� ��� �����̳�
	};

	//Vertex 
	/*
		���� ������ ������ �����ϰ� ���ִ� ���Ͻ� Ŭ����
		���� ������ ������ �������ִ� Ŭ�������ƴ� ���Ͻ� Ȥ�� �並 �����ϴ� Ŭ�����̴�
		�� �Ʒ��� �������ۿ� �����ϴ� ���������� ���Ͻ� �並 �����Ѵ�.

	*/
	class Vertex
	{
		friend class VertexBuffer;
	public:
		template<VertexLayout::ElementType Type>
		//���ø� Ÿ���� ��ġ�� ã�� �����Ѵ�. �� Poisition2D�� �ش� ���� ���̾ƿ��� �����ϸ�
		//Position2D ������ ù�ּҸ� ã�Ƽ� ��ȯ�Ѵ�.
		auto& Attr() noexcept(!IS_DEBUG)
		{
			//������ ���ø��� ���� Type�� ���� Attr�� ����Ͽ� ������ ���������� �����ҋ�
			//���� �񱳹��� ���� ĳ���ù�ȯ�� �Ͽ��� ������ ���ø� Ư��ȭ�� Map�� Ÿ�Ը������ϸ�
			//�ش� ���������� �ý���Ÿ������ ĳ������ �� �־� �ڵ������� �Ǿ���.
			auto pAttribute = pData + layout.Resolve<Type>().GetOffset();
			return * reinterpret_cast<typename VertexLayout::Map<Type>::SysType*>(pAttribute);
		}
		template<typename T>
		void SetAttributeByIndex(size_t i, T&& val) noexcept(!IS_DEBUG)
		{
			const auto& element = layout.ResolveByIndex(i);
			auto pAttribute = pData + element.GetOffset();
			//�� ����ġ���� �ý���Ÿ���� �ѱ����ʾ� Ÿ���� ��߳� ������ ��������.
			switch (element.GetType())
			{
			case VertexLayout::Position2D:
				SetAttribute<VertexLayout::Position2D>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Position3D:
				SetAttribute<VertexLayout::Position3D>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Texture2D:
				SetAttribute<VertexLayout::Texture2D>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Normal:
				SetAttribute<VertexLayout::Normal>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Float3Color:
				SetAttribute<VertexLayout::Float3Color>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Float4Color:
				SetAttribute<VertexLayout::Float4Color>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::BGRAColor:
				SetAttribute<VertexLayout::BGRAColor>(pAttribute, std::forward<T>(val));
				break;
			default:
				assert("Bad element type" && false);
			}
		}
	protected:
		Vertex(char* pData, const VertexLayout& layout) noexcept(!IS_DEBUG)
			:
			pData(pData),
			layout(layout)
		{
			assert(pData != nullptr);
		}
	private:
		// �� ���ø��Լ��� �Ķ���� ���� ����ϴ� �Լ��� ��
		// �츮�� ������ �������ۿ����� �������̾ƿ��� �����ϴ� ������ �������ε�.
		// �������ۿ����� �پ��� ������ ���� ������ �ɰ��̴� ��ġ���̿ü����հ� �������� ������ �������ִ�
		// �̷���쿡 �츮�� ������ ������ ���ۿ� �ѱ�� �����Ƿ�
		// �����������忡���� ���Ұ� 1����Ʈ�� �����̳ʸ� ������ �ְ� ���� ������ ���� ���� Ŭ�������� 
		// �Ʒ� �Ķ�������� ������ ���ø��Լ��� �����Ͽ� �ѱ���̴�

		/*
			�ϴ� �������ϸ� ���� Position3D,Texture2D,Normal �� �̷���� ���������Ͽ� ���̾ƿ��� ���� �ε����� �Ѵٰ��ϸ�
			SetAttributeByIndex �Լ��� �θ����̴� �� �Լ��� �����ε��Ǿ��ִµ� �ϳ���  �ε����� �ش� Ÿ�Կ����� ������ �޴� �Լ���
			�Ǵٸ� �ϳ��� ù��°���� �Ķ�������� �Ű������� ������ �Լ��� �ִ� ���Լ��� ȣ���Ͽ� ó���� 0,Position3D,{Texture2D,Normal}
			�̷��� �Ѿ�ٸ� ù��°���� �ε����ϴ� �Լ��� ȣ���Ͽ� �ش� Ÿ���� ����ġ�� ������ Ÿ������ ��ȯ�ؼ�
			�ش� ������ �ּҰ��� ���� ����� ���̴�.

			�� ������� ������ �غ��ڸ�

			ù��°�� �������ۿ��� EmplaceBack() �Լ��� ȣ��� ���Ұ� ������𸣴� ������ �Ķ�������������� �����Ѵ�
			�������۴� ���̾ƿ��� �����Ǿ������Ƿ� ���� ������ ũ�� + ���ε��� ������ ũ��(���̾ƿ�) ��ŭ �ø���.
			�״��� SetAttributeByIndex �Լ��� ù��° ���ڷ� 0u �� Back()���� ȣ���Ѱ��� ���� ����ũ�� + ���̾ƿ�ũ�� ���¿���
			-���̾ƿ�ũ�����Ͽ� ������ ������ ���ּ��� ���� Vertex�� ��ȯ�Ѵ�. �� ������ �ް����� ����Ű�� Vertex���� SetAttributeByIndex ȣ���
			0u�� ��ġ�� ���̾ƿ��� ���� ���ڸ��� ����Ű�� ������ ���� �Ķ���� ���� ���޵Ǿ����� first�� ������ rest�� �и��Ǿ�
			�Լ��� ���޵ǰ� �ȴ� First�� ���޵Ǹ� ���̾ƿ��� �����ϴ� �ε�������� ���Ұ˻� �Լ��� �ش������ �������� ��ԵǸ�

			���� Vertex�� ������ ȣ��� pData �� ������ ù��° �ּҴ� ������ Back() �ּҷ� �Ǿ������Ƿ� �ش� �ּҷκ��� + �ε��̵� ������ ���������ϸ�
			pAttribute �� �ش� ������ ��ġ�� ����Ű�� ������ ���´�. �� �ּҸ� ������ SetAttribute�� ȣ���Ͽ� ���Ҹ� �ش��ϴ� Ÿ������ ��ȯ�Ͽ� ����� �ϰԵȴ�.

			���� ù�ּҿ� Position3D�� ��ϵǾ��ٸ� ������ Textur2D,Normal�� �ٽ� ����Ͽ� �ι�° �Լ����� ��
			ù��° ���� ������������ �и��Ǿ� Textur2D���� ���� ������� ��ϵǰԵȴ� �������� ���̻� �и��� �Ķ���� ���� ���ٸ�
			ù��° �Լ��� ȣ��Ǿ� ������ �ް����� ������ ���Եȴ�.(�Ķ�������� 2���̻��ε� �ƴϹǷ� �ٸ� �Լ��� ã�Ƽ� ȣ���Ұ��̴�)

		*/
		template<typename First, typename ...Rest>
		// enables parameter pack setting of multiple parameters by element index
		void SetAttributeByIndex(size_t i, First&& first, Rest&&... rest) noexcept(!IS_DEBUG)
		{
			SetAttributeByIndex(i, std::forward<First>(first));
			SetAttributeByIndex(i + 1, std::forward<Rest>(rest)...);	//����������� ���� ���Ҹ� ����Ű�Եȴ�.
		}
		// helper to reduce code duplication in SetAttributeByIndex
		template<VertexLayout::ElementType DestLayoutType, typename SrcType>
		void SetAttribute(char* pAttribute, SrcType&& val) noexcept(!IS_DEBUG)
		{
			using Dest = typename VertexLayout::Map<DestLayoutType>::SysType;
			//is_assignable �� src���� Dest�� �Ҵ簡�ɿ��θ� üũ�Ѵ�.
			if constexpr (std::is_assignable<Dest, SrcType>::value)
			{
				//ù��° ���̾ƿ��� Position3D �̰� {0.0f,0.0f,0.0f}�� �Ѱ�ٸ� 
				//���� ����ġ������ �˻��Ͽ� ������ �����ּҿ� ����Ѵ�.
				*reinterpret_cast<Dest*>(pAttribute) = val;
			}
			else
			{
				assert("Parameter attribute type mismatch" && false);
			}
		}
	private:
		char* pData = nullptr;
		const VertexLayout& layout;
	};

	//ConstVertex
	/*
		���� ���ؽ��� �޸� ���ȭ�� Ŭ����
		��������� ���ؽ��� ������ �ִ�.
	*/

	class ConstVertex
	{
	public:
		ConstVertex(const Vertex& v) noexcept(!IS_DEBUG)
			:
			vertex(v)
		{}
		template<VertexLayout::ElementType Type>
		const auto& Attr() const noexcept(!IS_DEBUG)
		{
			return const_cast<Vertex&>(vertex).Attr<Type>();
		}
	private:
		Vertex vertex;
	};

	// VertexBuffer
	/*
		���� ���� Ŭ���� ���� ������ ������ �������ִ� Ŭ������
		���� ���̾ƿ��� �����Ͽ� �ش� ���̾ƿ��� ���� ������ �޴´�
		������ ������ ������ �𸣹Ƿ� �Ķ���������޾� ���ҿ� �߰��ϸ�
		1byte �����̳ʷ� ���ӵȰ����� �����ϰԵȴ�.
	*/
	class VertexBuffer
	{
	public:
		VertexBuffer(VertexLayout layout) noexcept(!IS_DEBUG)
			:
			layout(std::move(layout))
		{}
		const char* GetData() const noexcept(!IS_DEBUG)
		{
			return buffer.data();
		}
		const VertexLayout& GetLayout() const noexcept
		{
			return layout;
		}
		//������ ��������ȯ�Ѵ�.
		size_t Size() const noexcept(!IS_DEBUG)
		{
			return buffer.size() / layout.Size();
		}
		//������ ����� ��ȯ�Ѵ�. 
		size_t SizeBytes() const noexcept(!IS_DEBUG)
		{
			return buffer.size();
		}
		//���ο� ������ �����Ͽ� �����̳ʳ��� ������ �ֵ����Ѵ�.
		template<typename ...Params>
		void EmplaceBack(Params&&... params) noexcept(!IS_DEBUG)
		{
			//�Ʒ� ó������ ���� sizeof ... �Ƹ� �Ķ�������� ���̸� ���ϴ°��ϰ��̴� ��
			//������ ���̾ƿ� ���ҵ��� ������ �Ķ���� ���� ������ ���������� �̽���ġ�ΰ��̴�.
			assert(sizeof...(params) == layout.GetElementCount() && "Param count doesn't match number of vertex elements");
			buffer.resize(buffer.size() + layout.Size());
			Back().SetAttributeByIndex(0u, std::forward<Params>(params)...);
		}
		//������ ���� ��ȯ
		Vertex Back() noexcept(!IS_DEBUG)
		{
			assert(buffer.size() != 0u);
			//buffer�� ���������Ҹ� Vertex�� �����Ѵ� �������� X
			return Vertex{ buffer.data() + buffer.size() - layout.Size(),layout };
		}
		//ù��° ������ȯ
		Vertex Front() noexcept(!IS_DEBUG)
		{
			assert(buffer.size() != 0u);
			return Vertex{ buffer.data(),layout };
		}
		//�ε����� ������ ���ٰ����ϴ�.
		Vertex operator[](size_t i) noexcept(!IS_DEBUG)
		{
			assert(i < Size());
			return Vertex{ buffer.data() + layout.Size() * i,layout };
		}
		ConstVertex Back() const noexcept(!IS_DEBUG)
		{
			return const_cast<VertexBuffer*>(this)->Back();
		}
		ConstVertex Front() const noexcept(!IS_DEBUG)
		{
			return const_cast<VertexBuffer*>(this)->Front();
		}
		ConstVertex operator[](size_t i) const noexcept(!IS_DEBUG)
		{
			return const_cast<VertexBuffer&>(*this)[i];
		}
	private:
		std::vector<char> buffer;			//��� �������� ����Ʈ �����͸� ��� �����̳�
		VertexLayout layout;				//������ ������ �������ִ� ���̾ƿ� Ŭ����
	};
}
