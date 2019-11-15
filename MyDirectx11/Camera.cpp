#include "Camera.h"
#include "imgui/imgui.h"
#include "BsMath.h"

namespace dx = DirectX;

Camera::Camera() noexcept
{
	Reset();
}

//뷰행렬을 반환한다.
DirectX::XMMATRIX Camera::GetMatrix() const noexcept
{
	//더나은회전
	using namespace dx;

	const dx::XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	// 카메라의 회전을 전방벡터에 적용한다.
	const auto lookVector = XMVector3Transform(forwardBaseVector,
		XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
	);
	// 카메라 트랜스폼을 만든다 트랜스폼은 모든 상대적인 오브젝트에 대해 적용되며 카메라의 윗방향은 항상 +y 이다
	const auto camPosition = XMLoadFloat3(&pos);
	const auto camTarget = camPosition + lookVector;
	return XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

//imgui를 이용한 메뉴구성 카메라 변수들을 조절한다.
void Camera::SpwanControlWindow() noexcept
{
	if (ImGui::Begin("Camera"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("X", &pos.x, -80.0f, 80.f, "%.1f");
		ImGui::SliderFloat("Y", &pos.y, -80.0f, 80.f, "%.1f");
		ImGui::SliderFloat("Z", &pos.z, -80.0f, 80.f, "%.1f");
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Pitch", &pitch, 0.995f * -90.0f, 0.995f * 90.f);
		ImGui::SliderAngle("Yaw", &yaw, -180.f, 180.f);
		if (ImGui::Button("Reset"))
			Reset();
	}
	ImGui::End();
}
//모든 카메라 변수 초기화.
void Camera::Reset() noexcept
{
	pos = { 0.0f,7.5f,-18.0f };
	pitch = 0.0f;
	yaw = 0.0f;
}

//카메라 회전
void Camera::Rotate(float dx, float dy) noexcept
{
	//yaw는 좌우를 보는것이므로 제한이 없다.
	yaw = wrap_angle(yaw + dx * rotationSpeed);
	//pitch는 위아래를 보는것인데 일정각도이상은 못보도록 clamp 제한한다.
	pitch = std::clamp(pitch + dy * rotationSpeed, -PI / 2.0f, PI / 2.0f);
}

void Camera::Translate(DirectX::XMFLOAT3 translation) noexcept
{
	//여기서 translation은 방향벡터가 될것인데 방향벡터에 카메라 회전변환을 적용하고
	//카메라 스피드만큼 스케일하면서 방향의 속도를 정한다.
	dx::XMStoreFloat3(&translation, dx::XMVector3Transform(
		dx::XMLoadFloat3(&translation),
		dx::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) *
		dx::XMMatrixScaling(travelSpeed, travelSpeed, travelSpeed)
	));

	//그후 현재위치에 더해주면 카메라이동이다.
	pos = {
		pos.x + translation.x,
		pos.y + translation.y,
		pos.z + translation.z
	};
}
