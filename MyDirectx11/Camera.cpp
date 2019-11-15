#include "Camera.h"
#include "imgui/imgui.h"
#include "BsMath.h"

namespace dx = DirectX;

Camera::Camera() noexcept
{
	Reset();
}

//������� ��ȯ�Ѵ�.
DirectX::XMMATRIX Camera::GetMatrix() const noexcept
{
	//������ȸ��
	using namespace dx;

	const dx::XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
	// ī�޶��� ȸ���� ���溤�Ϳ� �����Ѵ�.
	const auto lookVector = XMVector3Transform(forwardBaseVector,
		XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f)
	);
	// ī�޶� Ʈ�������� ����� Ʈ�������� ��� ������� ������Ʈ�� ���� ����Ǹ� ī�޶��� �������� �׻� +y �̴�
	const auto camPosition = XMLoadFloat3(&pos);
	const auto camTarget = camPosition + lookVector;
	return XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

//imgui�� �̿��� �޴����� ī�޶� �������� �����Ѵ�.
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
//��� ī�޶� ���� �ʱ�ȭ.
void Camera::Reset() noexcept
{
	pos = { 0.0f,7.5f,-18.0f };
	pitch = 0.0f;
	yaw = 0.0f;
}

//ī�޶� ȸ��
void Camera::Rotate(float dx, float dy) noexcept
{
	//yaw�� �¿츦 ���°��̹Ƿ� ������ ����.
	yaw = wrap_angle(yaw + dx * rotationSpeed);
	//pitch�� ���Ʒ��� ���°��ε� ���������̻��� �������� clamp �����Ѵ�.
	pitch = std::clamp(pitch + dy * rotationSpeed, -PI / 2.0f, PI / 2.0f);
}

void Camera::Translate(DirectX::XMFLOAT3 translation) noexcept
{
	//���⼭ translation�� ���⺤�Ͱ� �ɰ��ε� ���⺤�Ϳ� ī�޶� ȸ����ȯ�� �����ϰ�
	//ī�޶� ���ǵ常ŭ �������ϸ鼭 ������ �ӵ��� ���Ѵ�.
	dx::XMStoreFloat3(&translation, dx::XMVector3Transform(
		dx::XMLoadFloat3(&translation),
		dx::XMMatrixRotationRollPitchYaw(pitch, yaw, 0.0f) *
		dx::XMMatrixScaling(travelSpeed, travelSpeed, travelSpeed)
	));

	//���� ������ġ�� �����ָ� ī�޶��̵��̴�.
	pos = {
		pos.x + translation.x,
		pos.y + translation.y,
		pos.z + translation.z
	};
}
