#include "hzpch.h"
#include "Application.h"

namespace Hazel {

	Application::Application()
	{
	}


	Application::~Application()
	{
	}

	void Application::Run()
	{
		spdlog::info("Welcome to Hazel!");
		while (true);
	}

}