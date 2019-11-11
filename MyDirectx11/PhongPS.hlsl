 cbuffer LightBuf
{
	float3 lightPos;
};

static const float3 materialColor = { 0.7f,0.7f,0.9f };
static const float3 ambient = { 0.05f,0.05f,0.15f };
static const float3 diffuseColor = { 1.0f,1.0f,1.0f };
static const float diffuseIntensity = 1.0f;
static const float attConst = 1.0f;
static const float attLin = 0.045f;
static const float attQuad = 0.0075f;

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
    return float4(saturate(diffuse+ambient),1.0f);
}

//attenuation 빛 감쇠의 적절한 값을 알고싶으면 아래사이트를 참고할것
//점광원에 대한 공식도 볼것.

//wiki.orgre3d.org/-point+lightPos+Attenuation