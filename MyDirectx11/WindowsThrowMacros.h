#pragma once

//디버거가 있는 컴파일러는 보통 어떤라인에서 에러가 났는지 출력하는데
//__LINE__ 매크로가 어떤라인인지 알려주며 __FILE__은 어떤파일에서 에러가 났는지 알려준다.
#define BSWND_EXCEPT(hr) Window::HrException(__LINE__,__FILE__,hr);
#define BSWND_LAST_EXCEPT() Window::HrException(__LINE__,__FILE__,GetLastError());
#define BSWND_NOGFX_EXCEPT() Window::NoGfxException(__LINE__,__FILE__);