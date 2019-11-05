struct VSout
{
	float3 color : Color;
	float4 pos   : SV_Position;
};

cbuffer CBuf
{
	 matrix transform;
};
//HLSL은 기본적으로 들어오는 모든 행렬은 열위주로 여긴다
//row_major 은 행렬이 행위주임을 알려줄는 키워드이다. 하지만 이러한 연산은 조금 느리게 만든다.
//이부분을 최적화 하려면 기본값인 열위주의 행렬을 넘기고 넘기기전에 CPU에서 Transpose를 통해 전치를 시켜준다.

VSout main(float2 pos : Position, float3 color : Color) 
{
	VSout vso;
	vso.pos   = mul(float4(pos.x, pos.y, 0.0f, 1.0f),transform);
	vso.color = color;
	return vso;
}