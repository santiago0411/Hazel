#pragma once

#ifdef HZ_PLATFORM_WINDOWS

extern auto Hazel::CreateApplication() -> Hazel::Application*;

int main(int argc, char** argv)
{
	auto app = Hazel::CreateApplication();
	app->Run();
	delete app;
}

#endif
