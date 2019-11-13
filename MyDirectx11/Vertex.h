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
		정점레이아웃을 설명하는 클래스이다.
		즉 우리가 { "Position",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 }
		레이아웃을 지정한것처럼 해당 기능을 래핑한 클래스이다.

		혹은 그 레이아웃이 아니라면
			struct Vertex
			{
				dx::XMFLOAT3 pos;
				dx::XMFLOAT3 n;
			};
		이렇게 구성하는것을 바꾼것일것이다.
	*/
	class VertexLayout
	{
	public:
		//정점 구성요소의 종류를 나타내는 열거체이다.
		enum ElementType
		{
			Position2D,		//2차원 위치
			Position3D,		//3차원 위치
			Texture2D,		//텍스쳐 2D
			Normal,			//노말 값
			Float3Color,	//컬러
			Float4Color,
			BGRAColor,
			Count,
		};
		//템플릿 기법으로 기존에 레이아웃을 좀더 간편하게 정리한다
		//템플릿의 특수화를 이용하여 Map<ElementType>에 해당하는 구조체멤버들을 제공한다.
		//컴파일타임에 접근하기위해 static constexpr 키워드를 사용하였다.
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
				//기존의 스위치문과 다르게 템플릿의 특수화를통해
				//원하는 레이아웃의 정보를 템플릿화하여 해당 타입을전달한다
				//이런경우 직접 시스템타입을 일일히 적을필요하없어 실수의 여지가 줄어든다.
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
			//동적으로 D3D11 layout을 반환한다.
			//즉 해당 열거체(시스템 타입) 에 따라 포맷,시맨틱,오프셋과 용도를 자동으로 구조체로 반환한다.
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
			ElementType type;		//원소의 종류와
			size_t  offset;			//해당 원소의 오프셋
		};
	public:
		//정점 원소를 접근하는 함수. 템플릿 타입으로 넘겨 해당 타입의 상수참조로 반환한다.
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
		//위 함수와 기능을 똑같지만 인덱스 기반으로 반환한다.
		const Element& ResolveByIndex(size_t i) const noexcept(!IS_DEBUG)
		{
			return elements[i];
		}
		//이전에 템플릿의 타입을 기반으로 전달하였지만
		//이제는 Enum 열거체로 전달
		VertexLayout& Append(ElementType type) noexcept(!IS_DEBUG)
		{
			elements.emplace_back(type, Size());
			return *this;
		}

		//전체 정점의 크기를 반환하는 함수.
		size_t Size() const noexcept(!IS_DEBUG)
		{
			return elements.empty() ? 0u : elements.back().GetOffsetAfter();
		}
		//원소의 개수를 반환한다.
		size_t GetElementCount() const noexcept
		{
			return elements.size();
		}
		//원소들의 레이아웃을 벡터로 반환한다.
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
		std::vector<Element> elements; //정점 원소를 담는 컨테이너
	};

	//Vertex 
	/*
		단일 정점의 정보에 접근하게 해주는 프록시 클래스
		실제 정점의 정보를 가지고있는 클래스가아닌 프록시 혹은 뷰를 제공하는 클래스이다
		즉 아래의 정점버퍼에 존재하는 정점에대한 프록시 뷰를 제공한다.

	*/
	class Vertex
	{
		friend class VertexBuffer;
	public:
		template<VertexLayout::ElementType Type>
		//템플릿 타입의 위치를 찾아 접근한다. 즉 Poisition2D가 해당 정점 레이아웃에 존재하면
		//Position2D 원소의 첫주소를 찾아서 반환한다.
		auto& Attr() noexcept(!IS_DEBUG)
		{
			//기존에 템플릿에 들어온 Type에 따라 Attr을 사용하여 버퍼의 정점정보에 접근할떄
			//많은 비교문을 통해 캐스팅반환을 하였다 하지만 템플릿 특수화로 Map에 타입만전달하면
			//해당 정보에대한 시스템타입으로 캐스팅할 수 있어 코드절약이 되었다.
			auto pAttribute = pData + layout.Resolve<Type>().GetOffset();
			return * reinterpret_cast<typename VertexLayout::Map<Type>::SysType*>(pAttribute);
		}
		template<typename T>
		void SetAttributeByIndex(size_t i, T&& val) noexcept(!IS_DEBUG)
		{
			const auto& element = layout.ResolveByIndex(i);
			auto pAttribute = pData + element.GetOffset();
			//이 스위치문도 시스템타입을 넘기지않아 타입이 어긋날 걱정이 없어졌다.
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
		// 이 템플릿함수는 파라매터 팩을 사용하는 함수다 즉
		// 우리가 선언한 정점버퍼에서는 정점레이아웃이 설명하는 정점을 넣을것인데.
		// 정점버퍼에서는 다양한 정보를 가진 정점이 될것이다 위치값이올수도잇고 법선값을 가지고 있을수있다
		// 이런경우에 우리는 고정된 정보를 버퍼에 넘길수 없으므로
		// 정점버퍼입장에서는 원소가 1바이트인 컨테이너를 가지고 있고 이제 정점의 뷰인 정점 클래스에서 
		// 아래 파라매터팩을 가지는 템플릿함수를 제공하여 넘길것이다

		/*
			일단 설명을하면 만약 Position3D,Texture2D,Normal 로 이루어진 정점에대하여 레이아웃을 통해 인덱싱을 한다고하면
			SetAttributeByIndex 함수를 부를것이다 이 함수는 오버로딩되어있는데 하나는  인덱스와 해당 타입에대한 정보를 받는 함수와
			또다른 하나는 첫번째값과 파라매터팩을 매개변수로 가지는 함수가 있다 이함수를 호출하여 처음에 0,Position3D,{Texture2D,Normal}
			이렇게 넘어간다면 첫번째값은 인덱싱하는 함수를 호출하여 해당 타입을 스위치에 지정된 타입으로 변환해서
			해당 원소의 주소값에 값을 기록할 것이다.

			즉 순서대로 나열을 해보자면

			첫번째로 정점버퍼에서 EmplaceBack() 함수의 호출로 원소가 몇개인지모르는 정점을 파라매터팩형식으로 전달한다
			정점버퍼는 레이아웃이 지정되어있으므로 현재 버퍼의 크기 + 새로들어올 정점의 크기(레이아웃) 만큼 늘린다.
			그다음 SetAttributeByIndex 함수의 첫번째 인자로 0u 즉 Back()에서 호출한것은 현재 버퍼크기 + 레이아웃크기 상태에서
			-레이아웃크리를하여 기존의 버퍼의 뒷주소의 뷰인 Vertex를 반환한다. 즉 버퍼의 뒷공간을 가르키는 Vertex에서 SetAttributeByIndex 호출로
			0u의 위치는 레이아웃의 가장 앞자리를 가르키고 있으며 이제 파라매터 팩이 전달되었으니 first와 나머지 rest로 분리되어
			함수에 전달되게 된다 First로 전달되면 레이아웃이 제공하는 인덱스기반의 원소검색 함수로 해당원소의 오프셋을 얻게되며

			현재 Vertex의 생성자 호출로 pData 즉 정점의 첫번째 주소는 버퍼의 Back() 주소로 되어있으므로 해당 주소로부터 + 인덱싱된 원소의 오프셋을하면
			pAttribute 즉 해당 원소의 위치를 가르키는 변수가 나온다. 그 주소를 가지고 SetAttribute를 호출하여 원소를 해당하는 타입으로 변환하여 기록을 하게된다.

			이제 첫주소에 Position3D가 기록되었다면 나머지 Textur2D,Normal은 다시 재귀하여 두번째 함수에서 들어가
			첫번째 값과 나머지값으로 분리되어 Textur2D또한 같은 방법으로 기록되게된다 마지막에 더이상 분리할 파라매터 팩이 없다면
			첫번째 함수가 호출되어 버퍼의 뒷공간에 정점이 들어가게된다.(파라매터팩은 2개이상인데 아니므로 다른 함수를 찾아서 호출할것이다)

		*/
		template<typename First, typename ...Rest>
		// enables parameter pack setting of multiple parameters by element index
		void SetAttributeByIndex(size_t i, First&& first, Rest&&... rest) noexcept(!IS_DEBUG)
		{
			SetAttributeByIndex(i, std::forward<First>(first));
			SetAttributeByIndex(i + 1, std::forward<Rest>(rest)...);	//증가시켜줘야 다음 원소를 가르키게된다.
		}
		// helper to reduce code duplication in SetAttributeByIndex
		template<VertexLayout::ElementType DestLayoutType, typename SrcType>
		void SetAttribute(char* pAttribute, SrcType&& val) noexcept(!IS_DEBUG)
		{
			using Dest = typename VertexLayout::Map<DestLayoutType>::SysType;
			//is_assignable 은 src에서 Dest로 할당가능여부를 체크한다.
			if constexpr (std::is_assignable<Dest, SrcType>::value)
			{
				//첫번째 레이아웃이 Position3D 이고 {0.0f,0.0f,0.0f}를 넘겼다면 
				//위에 스위치문에서 검사하여 정보를 버퍼주소에 기록한다.
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
		기존 버텍스와 달리 상수화된 클래스
		멤버변수로 버텍스를 가지고 있다.
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
		정점 버퍼 클래스 실제 정점의 정보를 가지고있는 클래스로
		정점 레이아웃을 지정하여 해당 레이아웃의 정점 정보를 받는다
		정점의 원소의 종류를 모르므로 파라매터팩을받아 원소에 추가하며
		1byte 컨테이너로 연속된공간에 저장하게된다.
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
		//정점의 개수를반환한다.
		size_t Size() const noexcept(!IS_DEBUG)
		{
			return buffer.size() / layout.Size();
		}
		//버퍼의 사이즈를 반환한다. 
		size_t SizeBytes() const noexcept(!IS_DEBUG)
		{
			return buffer.size();
		}
		//새로운 정점을 생성하여 컨테이너끝에 넣을수 있도록한다.
		template<typename ...Params>
		void EmplaceBack(Params&&... params) noexcept(!IS_DEBUG)
		{
			//아래 처음보는 구문 sizeof ... 아마 파라매터팩의 길이를 뜻하는것일것이다 즉
			//버퍼의 레이아웃 원소들의 개수와 파라매터 팩의 개수가 맞지않으면 미스매치인것이다.
			assert(sizeof...(params) == layout.GetElementCount() && "Param count doesn't match number of vertex elements");
			buffer.resize(buffer.size() + layout.Size());
			Back().SetAttributeByIndex(0u, std::forward<Params>(params)...);
		}
		//마지막 정점 반환
		Vertex Back() noexcept(!IS_DEBUG)
		{
			assert(buffer.size() != 0u);
			//buffer의 마지막원소를 Vertex로 제공한다 직접제공 X
			return Vertex{ buffer.data() + buffer.size() - layout.Size(),layout };
		}
		//첫번째 정점반환
		Vertex Front() noexcept(!IS_DEBUG)
		{
			assert(buffer.size() != 0u);
			return Vertex{ buffer.data(),layout };
		}
		//인덱스로 정점에 접근가능하다.
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
		std::vector<char> buffer;			//모든 정점들의 바이트 데이터를 담는 컨테이너
		VertexLayout layout;				//정점의 구조를 설명해주는 레이아웃 클래스
	};
}
