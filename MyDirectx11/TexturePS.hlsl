Texture2D tex ;

//샘플러 스테이트를 응용프로그램에서 지정한 슬롯에따라 지정되는데
//기본값은 0 이다 즉 :register(t0) ,t1,t2.. 등 응용프로그램이 지정한 슬롯에따라 레지스터에 쉐이더에서는 접근하면된다.
SamplerState splr;


float4 main(float2 tc : TEXCOORD) : SV_TARGET
{
	return tex.Sample(splr,tc);
}