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

float3 MapNormalViewSpace(const float3 tan, const float3 bitan, const float3 viewNormal, const float2 tc, Texture2D nmap, SamplerState splr)
{
	const float3x3 tanToView = float3x3(
		normalize(tan),
		normalize(bitan),
		normalize(viewNormal));

	const float3 normalSample = nmap.Sample(splr, tc).xyz;
	const float3 tanNormal = normalSample * 2.0f - 1.0f;

	return normalize(mul(tanNormal, tanToView));
}

float4 main(float3 viewPos : Position, float3 n : viewNormal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
    if(normalMapEnabled)
    {
		viewNormal = MapNormalViewSpace(tan, bitan, viewNormal, tc, nmap, splr);
    }


	//물체의 조각 (정점) 에서  광원으로의 벡터(단위x)
    const float3 viewFragToL = viewLightPos - viewPos;
	//정점부터 광원까지의 거리
    const float distFragToL = length(viewFragToL);
	//정점부터 광원까지의 방향벡터
    const float3 viewDirFragToL = viewFragToL / distFragToL;
	//***점광원***//
    // 빛 감쇄치 정하기
    const float att = 1.0f / (attConst + attLin * distFragToL + attQuad * (distFragToL * distFragToL));
	// 빛 강도
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(viewDirFragToL, viewNormal));
    //************//

    //***정반사***//
    //빛벡터에 대한 반사벡터
    const float3 w = viewNormal * dot(viewFragToL, viewNormal);
    const float3 r = w * 2.0f - viewFragToL;
    

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
    const float3 specular = att * (diffuseColor * diffuseIntensity) * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specularPower);
	// 최종 색 (텍스처 기반 색)
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular * specularReflectionColor), 1.0f);
}