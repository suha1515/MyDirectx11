//20번째 강의 상수버퍼2를 위한 상수버퍼
cbuffer CBuf
{
	float4 face_colors[6];
};


//픽셀쉐이더는 레스터라이즈 된 픽셀의 색깔을 정한다.

float4 main(uint tid : SV_PrimitiveID) : SV_Target
{
	return face_colors[tid / 2];
}