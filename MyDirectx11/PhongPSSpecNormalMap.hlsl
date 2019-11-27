 cbuffer LightBuf
{
	float3 lightPos;            //광원위치
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

float4 main(float3 viewPos : Position, float3 n : viewNormal, float3 tan : Tangent, float3 bitan : Bitangent, float2 tc : Texcoord) : SV_TARGET
{
    if(normalMapEnabled)
    {
        const float3x3 tanToView = float3x3(
        normalize(tan),
        normalize(bitan),
        normalize(viewNormal)
        );

		//노멀 샘플링
        const float3 normalSample = nmap.Sample(splr, tc).xyz;
		float3 tanNormal;
        tanNormal = normalSample * 2.0f - 1.0f;
        // bring normal from tanspace into view space
		// 다시 정규화를 해준다.
		viewNormal = normalize(mul(tanNormal, tanToView));
    }


	//물체의 조각 (정점) 에서  광원으로의 벡터(단위x)
    const float3 vToL = lightPos - viewPos;
	//정점부터 광원까지의 거리
    const float distToL = length(vToL);
	//정점부터 광원까지의 방향벡터
    const float3 dirToL = vToL / distToL;
	//***점광원***//
    // 빛 감쇄치 정하기
    const float att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// 빛 강도
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, viewNormal));
    //************//

    //***정반사***//
    //빛벡터에 대한 반사벡터
    const float3 w = viewNormal * dot(vToL, viewNormal);
    const float3 r = w * 2.0f - vToL;
    

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