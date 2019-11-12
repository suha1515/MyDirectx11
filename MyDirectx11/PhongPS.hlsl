 cbuffer LightBuf
{
	float3 lightPos;
    float3 ambient;
    float3 diffuseColor;
    float  diffuseIntensity;
    float attConst;
    float attLin;
    float attQuad;
};

cbuffer ObjectCBuf
{
    float3 materialColor;
};


float4 main(float3 worldPos : POSITION, float3 n : NORMAL) : SV_TARGET
{
	//물체의 조각 (정점) 에서  광원으로의 벡터(단위x)
	const float3 vToL = lightPos - worldPos;
	//정점부터 광원까지의 거리
    const float distToL = length(vToL);
	//정점부터 광원까지의 방향벡터
    const float3 dirToL = vToL / distToL;
	// 빛 섞기
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// 빛 강도
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
	// 최종 색
    return float4(saturate(diffuse+ambient)*materialColor,1.0f);
}

//attenuation 빛 감쇠의 적절한 값을 알고싶으면 아래사이트를 참고할것
//점광원에 대한 공식도 볼것.

//wiki.orgre3d.org/-point+lightPos+Attenuation