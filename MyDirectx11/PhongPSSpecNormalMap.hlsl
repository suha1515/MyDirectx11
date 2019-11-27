 cbuffer LightBuf
{
	float3 viewLightPos;            //광원위치
    float3 ambient;             //주변광
    float3 diffuseColor;        //확산광
    float  diffuseIntensity;    //확산광 세기
    float attConst;             //빛 감쇄 상수
    float attLin;               //빛 감쇄 거리
    float attQuad;              //빛 감쇄 정방체
};

//오브젝트 고유의 색과 빛나는정도를 정한다.
cbuffer ObjectCBuf
{
    bool normalMapEnabled;
    bool specularMapEnabled;
    bool hasGloss;
    float specularPowerConst;
    float3 specularColor;
    float specularMapweight;
};

Texture2D tex;
// 스페큘러 맵.
Texture2D spec;
Texture2D nmap;

SamplerState splr;

float3 MapNormal(const in float3 tan,
	const in float3 bitan,
	const in float3 normal,
	const in float2 tc,
	uniform Texture2D nmap,
	uniform SamplerState splr)
{
	// tan/bitan/normal 을이용하여 같은공간(타겟)으로 변환시키는 변환행렬 생성
	const float3x3 tanToTarget = float3x3(
		normalize(tan),
		normalize(bitan),
		normalize(normal));

	const float3 normalSample = nmap.Sample(splr, tc).xyz;
	const float3 tanNormal = normalSample * 2.0f - 1.0f;

	return normalize(mul(tanNormal, tanToTarget));
}

//const in float <<- ? 무슨 키워드지?
float Attenuate(uniform float attConst, uniform float attLin, uniform float attQuad, const in float distFragToL)
{
	return 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
}

float3 Diffuse(
	uniform float3 diffuseColor,
	uniform float diffuseIntensity,
	const in float att,
	const in float3 viewDirFragToL,
	const in float3 viewNormal)
{
	return diffuseColor * diffuseIntensity * att * max(0.0f, dot(viewDirFragToL, viewNormal));
}

float3 Speculate(
	const in float3 specularColor,
	uniform float specularIntensity,
	const in float3 viewNormal,
	const in float3 viewFragToL,
	const in float3 viewPos,
	const in float att,
	const in float specularPower)
{
	//반사 벡터 계산
	const float3 w = viewNormal * dot(viewFragToL, viewNormal);
	const float3 r = normalize(w * 2.0f - viewFragToL);
	//뷰스페이스에서 카메라에서 정점으로의 벡터
	const float3 viewCamToFrag = normalize(viewPos);
	//정반사의 양을 시야벡터와 반사벡터사이의 각에의해 정한다. ( 범위는 제곱승수로 좁힌다)
	return att * specularColor * specularIntensity * pow(max(0.0f, dot(-r, viewCamToFrag)), specularPower);
}

float4 main(float3 viewPos : Position, float3 viewNormal : Normal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
    if(normalMapEnabled)
    {
		viewNormal = MapNormal(tan, bitan, viewNormal, tc, nmap, splr);
    }
	//물체의 조각 (정점) 에서  광원으로의 벡터(단위x)
    const float3 viewFragToL = viewLightPos - viewPos;
	//정점부터 광원까지의 거리
    const float distFragToL = length(viewFragToL);
	//정점부터 광원까지의 방향벡터
    const float3 viewDirFragToL = viewFragToL / distFragToL;
	//***점광원***//
    // 빛 감쇄치 정하기
	const float att = Attenuate(attConst, attLin, attQuad, distFragToL);
	// 빛 강도
	const float3 diffuse = Diffuse(diffuseColor, diffuseIntensity, att, viewDirFragToL, viewNormal);
    //************//

    //시선벡터와 반사벡터사이의 각에 기반하여 정반사 강도를 구한다.
    float3 specularReflectionColor;
    //스페큘러 맵에서 승수값을 받기전까진 기본으로 머터리얼값을 사용한다.
    float specularPower = specularPowerConst;
    if(specularMapEnabled)
    {
        //수치화된 스페큘러가 아닌. 스페큘러맵에서 강도를 가져온다.
        const float4 specularSample = spec.Sample(splr, tc);
        specularReflectionColor = specularSample.rgb * specularMapweight;

        //스페큘러맵의 알파값이 승수값이므로 있는경우 파워값을 받아온다
        if(hasGloss)
        {
            specularPower = pow(2.0f, specularSample.a * 13.0f);
        }
    }
    else
    {
        specularReflectionColor = specularColor;
    }
	// 정반사
	const float3 specularReflected = Speculate(specularReflectionColor, 1.0f, viewNormal, viewFragToL, viewPos, att, specularPower);
	// 최종 색 (텍스처 기반 색)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specularReflected * specularReflectionColor), 1.0f);
}