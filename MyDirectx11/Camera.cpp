#include "Camera.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

//뷰행렬을 반환한다.
DirectX::XMMATRIX Camera::GetMatrix() const noexcept
{
	const auto pos = dx::XMVector3Transform(
		dx::XMVectorSet(0.0f, 0.0f, -radius, 0.0f),				//원점으로부터 radius만큼 떨어진다
		dx::XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f));	//phi,theta가 적도,북극 등을 기준으로 한다는데.. 정확히 설명은 모르겟다.
		//적도의 위선. 북극과남극의 경선 즉 위선은 원점을 기준으로 좌우로 경선은 상하로 위선,경선 검색
		//phi 는 경선, theta는 위선.
	return dx::XMMatrixLookAtLH(pos,dx::XMVectorZero(),			//LookAtLH로 원점을 바라보고,상향벡터를 지정하고 따로 회전행렬을 곱한다.	
		dx::XMVectorSet(0.0f,1.0f,0.0f,0.0f))*
		dx::XMMatrixRotationRollPitchYaw(pitch,-yaw,roll);
}

//imgui를 이용한 메뉴구성 카메라 변수들을 조절한다.
void Camera::SpwanControlWindow() noexcept
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("Radius", &radius, 0.0f, 80.f, "%.1f");
		ImGui::SliderAngle("Theta", &theta, -180.0f, 180.0f);
		ImGui::SliderAngle("Phi", &phi, -89.0f, 89.0f);
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &roll, -180.f, 180.f);
		ImGui::SliderAngle("Pitch", &pitch, -180.f, 180.f);
		ImGui::SliderAngle("Yaw", &yaw, -180.f, 180.f);
		if (ImGui::Button("Reset"))
			Reset();
	}
	ImGui::End();
}
//모든 카메라 변수 초기화.
void Camera::Reset() noexcept
{
	radius = 20.0f;
	theta = 0.0f;
	phi = 0.0f;
	pitch = 0.0f;
	yaw = 0.0f;
	roll = 0.0f;
}
