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
    //float3  materialColor;
    float   specularIntensity;  //정반사 강도
    float   specularPower;      //정반사 승수   
    //float   padding[2];          //패딩
};

Texture2D tex;
SamplerState splr;


float4 main(float3 viewPos : POSITION, float3 n : NORMAL, float2 tc : Texcoord) : SV_TARGET
{
	//물체의 조각 (정점) 에서  광원으로의 벡터(단위x)
    const float3 vToL = lightPos - viewPos;
	//정점부터 광원까지의 거리
    const float  distToL = length(vToL);
	//정점부터 광원까지의 방향벡터
    const float3 dirToL = vToL / distToL;
	//***점광원***//
    // 빛 감쇄치 정하기
    const float  att = 1.0f / (attConst + attLin * distToL + attQuad * (distToL * distToL));
	// 빛 강도
    const float3 diffuse = diffuseColor * diffuseIntensity * att * max(0.0f, dot(dirToL, n));
    //************//

    //***정반사***//
    //빛벡터에 대한 반사벡터
    const float3 w = n * dot(vToL, n);
    const float3 r = w * 2.0f - vToL;
    //시야벡터와 빛반사벡터 사이의 각도를 기반으로 정반사의 강도를 제곱수와 함께 구한다
    const float3 specular = att * (diffuseColor * diffuseIntensity) * specularIntensity * pow(max(0.0f, dot(normalize(-r), normalize(viewPos))), specularPower);
    //************//

	// 최종 색 (텍스처 기반 색)
    //return float4(saturate(diffuse + ambient + specular), 1.0f) * tex.Sample(splr, tc);
    return float4(saturate((diffuse + ambient) * tex.Sample(splr, tc).rgb + specular), 1.0f);
}

//attenuation 빛 감쇠의 적절한 값을 알고싶으면 아래사이트를 참고할것
//점광원에 대한 공식도 볼것.

//wiki.orgre3d.org/-point+lightPos+Attenuation