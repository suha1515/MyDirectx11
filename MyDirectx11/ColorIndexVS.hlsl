
cbuffer CBuf
{
	matrix transform;
};
//HLSL은 기본적으로 들어오는 모든 행렬은 열위주로 여긴다
//row_major 은 행렬이 행위주임을 알려줄는 키워드이다. 하지만 이러한 연산은 조금 느리게 만든다.
//이부분을 최적화 하려면 기본값인 열위주의 행렬을 넘기고 넘기기전에 CPU에서 Transpose를 통해 전치를 시켜준다.

float4 main(float3 pos : Position) :SV_Position
{
	return mul(float4(pos, 1.0f), transform);
}