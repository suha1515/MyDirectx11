//픽셀쉐이더는 레스터라이즈 된 픽셀의 색깔을 정한다.

float4 main(float3 color : Color) : SV_Target
{
	return float4(color,1.0f);
}