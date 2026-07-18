#pragma once

#include <memory>
// 知识点：需要 std::unique_ptr / std::shared_ptr / std::make_unique / std::make_shared。

// ---------------------------------------------------------------------------
// 平台与 DLL 导出宏
// ---------------------------------------------------------------------------
#ifdef HZ_PLATFORM_WINDOWS
	// 知识点：条件编译。CMake 里定义了 HZ_PLATFORM_WINDOWS，这段才会参与编译。

	#if HZ_DYNAMIC_LINK
		// 若将来引擎做成 DLL：导出/导入符号。
		#ifdef HZ_BUILD_DLL
			// 编译引擎 DLL 时：把符号导出给外面用。
			#define HAZEL_API __declspec(dllexport)
		#else
			// 链接引擎 DLL 的客户端：导入符号。
			#define HAZEL_API __declspec(dllimport)
		#endif
	#else
		// 当前是静态库：宏展开为空，不需要 dllimport/dllexport。
		#define HAZEL_API
	#endif
#else
	#error Hazel only support Windows!
#endif

// ---------------------------------------------------------------------------
// Assert（断言）宏
// ---------------------------------------------------------------------------
#ifdef HZ_DEBUG
#define HZ_ENABLE_ASSERTS
// Debug 配置下自动打开断言（也可在 CMake 里单独定义 HZ_ENABLE_ASSERTS）。
#endif

#ifdef HZ_ENABLE_ASSERTS
	// 知识点：函数式宏 + 可变参数宏 __VA_ARGS__。
	// 写法：HZ_ASSERT(ptr != nullptr, "ptr is null");
	// 展开后大致是：if (!(ptr != nullptr)) { HZ_ERROR(...); __debugbreak(); }
	//
	// 外面的 { }：把多条语句变成一个“语句块”，这样 if (x) HZ_ASSERT(...);
	// 不会和后面的 else 粘连出错（宏的常见坑）。
	//
	// __debugbreak()：MSVC 下触发断点，方便调试器停住。
	// 注意：这里用了 HZ_ERROR / HZ_CORE_ERROR，使用本头文件的翻译单元通常还需包含 Log.h。
	#define HZ_ASSERT(x, ...) { if(!(x)) { HZ_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
	#define HZ_CORE_ASSERT(x, ...) { if(!(x)) { HZ_CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); __debugbreak(); } }
#else
	// Release / 未启用断言时：宏展开为空，零开销（不生成检查代码）。
	#define HZ_ASSERT(x, ...)
	#define HZ_CORE_ASSERT(x, ...)
#endif // HZ_ENABLE_ASSERTS



#define BIT(x) (1 << x)

// 知识点：把“成员函数”绑成可调用对象，供 EventDispatcher 使用。
// 例：HZ_BIND_EVENT_FN(ExampleLayer::OnWindowResize)
// 展开：std::bind(&ExampleLayer::OnWindowResize, this, std::placeholders::_1)
// 即：调用时自动带上当前对象 this，并接收 1 个事件参数。
#define HZ_BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)


namespace Hazel {
	// 知识点：命名空间，避免与全局符号冲突；引擎 API 都挂在 Hazel:: 下。

	// 知识点：using 别名模板（alias template）。
	// Scope<T> 就是 std::unique_ptr<T> 的引擎侧名字：独占所有权，离开作用域自动 delete。
	template<typename T>
	using Scope = std::unique_ptr<T>;

	// 知识点：可变参数模板 + 完美转发。
	// CreateScope<Foo>(a, b) 等价于 std::make_unique<Foo>(a, b)，
	// Args&&... + std::forward 能保留实参的左值/右值属性，避免多余拷贝。
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args) {
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	// Ref<T> = 共享所有权智能指针；多个 Ref 指向同一对象，最后一个销毁时释放。
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}
}
