#pragma once

//����Ű� �ִ� �����Ϸ��� ���� ����ο��� ������ ������ ����ϴµ�
//__LINE__ ��ũ�ΰ� ��������� �˷��ָ� __FILE__�� ����Ͽ��� ������ ������ �˷��ش�.
#define BSWND_EXCEPT(hr) Window::HrException(__LINE__,__FILE__,hr);
#define BSWND_LAST_EXCEPT() Window::HrException(__LINE__,__FILE__,GetLastError());
#define BSWND_NOGFX_EXCEPT() Window::NoGfxException(__LINE__,__FILE__);