#include <gtest/gtest.h>

#include "Hazel/Core/Log.h"

int main(int argc, char** argv)
{
	Hazel::Log::Init();
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
