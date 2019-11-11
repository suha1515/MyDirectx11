#include "Camera.h"
#include "imgui/imgui.h"

namespace dx = DirectX;

//������� ��ȯ�Ѵ�.
DirectX::XMMATRIX Camera::GetMatrix() const noexcept
{
	const auto pos = dx::XMVector3Transform(
		dx::XMVectorSet(0.0f, 0.0f, -radius, 0.0f),				//�������κ��� radius��ŭ ��������
		dx::XMMatrixRotationRollPitchYaw(phi, -theta, 0.0f));	//phi,theta�� ����,�ϱ� ���� �������� �Ѵٴµ�.. ��Ȯ�� ������ �𸣰ٴ�.
		//������ ����. �ϱذ������� �漱 �� ������ ������ �������� �¿�� �漱�� ���Ϸ� ����,�漱 �˻�
		//phi �� �漱, theta�� ����.
	return dx::XMMatrixLookAtLH(pos,dx::XMVectorZero(),			//LookAtLH�� ������ �ٶ󺸰�,���⺤�͸� �����ϰ� ���� ȸ������� ���Ѵ�.	
		dx::XMVectorSet(0.0f,1.0f,0.0f,0.0f))*
		dx::XMMatrixRotationRollPitchYaw(pitch,-yaw,roll);
}

//imgui�� �̿��� �޴����� ī�޶� �������� �����Ѵ�.
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
//��� ī�޶� ���� �ʱ�ȭ.
void Camera::Reset() noexcept
{
	radius = 20.0f;
	theta = 0.0f;
	phi = 0.0f;
	pitch = 0.0f;
	yaw = 0.0f;
	roll = 0.0f;
}
