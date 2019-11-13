#pragma once
#include <vector>
#include <DirectXMath.h>
#include <type_traits>

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
			//������ ���������� ���� �ٸ� ũ�⸦ ��ȯ�Ѵ�.
			static constexpr size_t SizeOf(ElementType type) noexcept(!IS_DEBUG)
			{
				using namespace DirectX;
				switch (type)
				{
				case Position2D:
					return sizeof(XMFLOAT2);
				case Position3D:
					return sizeof(XMFLOAT3);
				case Texture2D:
					return sizeof(XMFLOAT2);
				case Normal:
					return sizeof(XMFLOAT3);
				case Float3Color:
					return sizeof(XMFLOAT3);
				case Float4Color:
					return sizeof(XMFLOAT3);
				case BGRAColor:
					return sizeof(MyVertex::BGRAColor);
				}
				assert("Invalid element type" && false);
				return 0u;
			}
			ElementType GetType() const noexcept
			{
				return type;
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
		//���̾ƿ��� �����Ҷ� ���ø� Ÿ�����γѰ� �ش� ���Ҹ� �ִ´�.
		template<ElementType Type>
		VertexLayout& Append() noexcept(!IS_DEBUG)
		{
			elements.emplace_back(Type, Size());
			return *this;
		}

		//��ü ������ ũ�⸦ ��ȯ�ϴ� �Լ�.
		size_t Size() const noexcept(!IS_DEBUG)
		{
			return elements.empty() ? 0u : elements.back().GetOffsetAfter();
		}
		size_t GetElementCount() const noexcept
		{
			return elements.size();
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
			using namespace DirectX;
			const auto& element = layout.Resolve<Type>();		//element ������Ʈ�� Resolve ȣ��� �ش� Type������ ������ �����Եȴ�
			auto pAttribute = pData + element.GetOffset();		//pData ù�ּҿ��� + �����¸�ŭ �������� �ش� ������ �ּҰ� ���´�	//�� ���̾ƿ������� ������� ������ �����Ǵ��� �˰��̸�
																//�� ���������� �������� �����ϴ� ó����ġ+�������� �ش������ ù�ּҰ������� �ȴ�.
			if constexpr (Type == VertexLayout::Position2D)
			{
				return *reinterpret_cast<XMFLOAT2*>(pAttribute);	//�ش� ������ ó�� �ּҸ� ������ ĳ�������� ���� �ǵ��� �ڷ������� �ٲ۴�
			}
			else if constexpr (Type == VertexLayout::Position3D)
			{
				return *reinterpret_cast<XMFLOAT3*>(pAttribute);
			}
			else if constexpr (Type == VertexLayout::Texture2D)
			{
				return *reinterpret_cast<XMFLOAT2*>(pAttribute);
			}
			else if constexpr (Type == VertexLayout::Normal)
			{
				return *reinterpret_cast<XMFLOAT3*>(pAttribute);
			}
			else if constexpr (Type == VertexLayout::Float3Color)
			{
				return *reinterpret_cast<XMFLOAT3*>(pAttribute);
			}
			else if constexpr (Type == VertexLayout::Float4Color)
			{
				return *reinterpret_cast<XMFLOAT4*>(pAttribute);
			}
			else if constexpr (Type == VertexLayout::BGRAColor)
			{
				return *reinterpret_cast<BGRAColor*>(pAttribute);
			}
			else
			{
				assert("Bad element type" && false);
				return *reinterpret_cast<char*>(pAttribute);
			}
		}
		template<typename T>
		void SetAttributeByIndex(size_t i, T&& val) noexcept(!IS_DEBUG)
		{
			using namespace DirectX;
			const auto& element = layout.ResolveByIndex(i);
			auto pAttribute = pData + element.GetOffset();
			switch (element.GetType())
			{
			case VertexLayout::Position2D:
				SetAttribute<XMFLOAT2>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Position3D:
				SetAttribute<XMFLOAT3>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Texture2D:
				SetAttribute<XMFLOAT2>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Normal:
				SetAttribute<XMFLOAT3>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Float3Color:
				SetAttribute<XMFLOAT3>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::Float4Color:
				SetAttribute<XMFLOAT4>(pAttribute, std::forward<T>(val));
				break;
			case VertexLayout::BGRAColor:
				SetAttribute<BGRAColor>(pAttribute, std::forward<T>(val));
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
		template<typename Dest, typename Src>
		void SetAttribute(char* pAttribute, Src&& val) noexcept(!IS_DEBUG)
		{
			//is_assignable �� src���� Dest�� �Ҵ簡�ɿ��θ� üũ�Ѵ�.
			if constexpr (std::is_assignable<Dest, Src>::value)
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
		size_t Size() const noexcept(!IS_DEBUG)
		{
			return buffer.size() / layout.Size();
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
