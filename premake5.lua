


function make_module(name)
    project(name)
        dofile "premake5_module.lua"

    io.writefile("modules/" .. name .. "/_generated.cpp", "")
end

workspace "apparatus"
    architecture "x64"
    systemversion "latest"

    startproject "launcher"


    filter "system:windows"
        exceptionhandling "SEH"
        configurations {
            "Debug_OpenGL45", "Test64_OpenGL45", "Release64_OpenGL45",
            "Debug_DirectX11", "Test64_DirectX11", "Release64_DirectX11",
            "Debug", "Test", "Release"
        }
    filter "system:linux"
        configurations {
            "Debug_OpenGL45", "Test64_OpenGL45", "Release64_OpenGL45",
            "Debug", "Test", "Release"
        }

    filter {}

    defines {
        "_CRT_SECURE_NO_WARNINGS",
        "_CRTLDBG_REPORT_FLAG",
        "FMT_HEADER_ONLY"
    }

    filter "configurations:Debug*"
        defines {"_CONFIG_DEBUG", "_AP_ENABLE_GL_DEBUG_CONTEXT"}
        runtime "Debug"
        symbols "On"
        floatingpoint "Strict"

    filter "configurations:Test*"
        defines  "_CONFIG_TEST"
        runtime  "Release"
        optimize "Debug"
        floatingpoint "Default"

    filter "configurations:Release*"
        defines  { "_CONFIG_RELEASE", "_AP_DISABLE_ASSERTS", "NDEBUG" }
        runtime  "Release"
        symbols  "Off"
        optimize "Speed"
        floatingpoint "Fast"

    filter "configurations:*DirectX11"
        --defines { "AP_SUPPORT_DX11" }

    filter "configurations:*OpenGL45"
        defines { "AP_SUPPORT_OPENGL45" }

    filter { "system:windows", "configurations:Debug or Release or Test" }
        defines { "AP_SUPPORT_OPENGL45"--[[, "AP_SUPPORT_DX11"]] }
    filter { "system:linux", "configurations:Debug or Release or Test" }
        defines { "AP_SUPPORT_OPENGL45" }

    project "apparatus"
        kind "SharedLib"
        language   "C++"
        warnings   "Default"
        cppdialect "C++17"
        targetdir  ("lib/%{prj.name}")
        objdir     ("lib/%{prj.name}-int")
        vectorextensions "SSE4.1"
        location "core/apparatus"


        flags {
            "FatalWarnings",
            "MultiProcessorCompile"
        }

        defines {
            "GLFW_STATIC",
            "GLAD_STATIC",
            "AP_EXPORT"
        }
        links {
            "imgui",
            "glfw",
            "glad",
            "box2d"
        }

        files { "%{prj.location}/src/**.cpp", "%{prj.location}/include/**.h" }

        includedirs {
            "%{prj.location}/include/",
            "deps/glad/include",
            "deps/glfw/include",
            "deps/mz",
            "deps/imgui",
            "deps/spdlog/include",
            "deps/stb_image",
            "deps/entt/single_include",
            "deps/entt/src"
        }

        pchheader "pch.h"
        pchsource "%{prj.location}/src/pch.cpp"

        disablewarnings {
            "4324"
        }

        postbuildcommands {
            "if not exist %{wks.location}lib\\runtime\\ mkdir %{wks.location}lib\\runtime\\",
            "copy %{wks.location}lib\\%{prj.name}\\%{prj.name}.dll %{wks.location}lib\\launcher\\%{prj.name}.dll",
            "copy %{wks.location}lib\\%{prj.name}\\%{prj.name}.dll %{wks.location}lib\\runtime\\%{prj.name}.dll"
        }

        filter "system:windows"
            defines { "_OS_WINDOWS" }

            links {
                "opengl32",
                "glu32",
                "gdi32",
                "d3d11",
                "D3DCompiler"
            }

        filter "system:linux"
            pic "On"

            defines { "_OS_LINUX" }

            links {
                "GL",
                "GLU",
                "X11",
                "Xxf86vm",
                "Xrandr",
                "pthread",
                "Xi",
                "dl",
                "stdc++fs",
            }

    
    make_module "test_module"

    make_module "2d_viewport"
    make_module "2d_editor"
    make_module "2d_sprite_renderer"
    make_module "2d_tilemap_renderer"

    make_module "asset_manager"

    make_module "2d_physics"

    make_module "2d_particles_simulator"

    project "launcher"
        kind "ConsoleApp"
        language   "C++"
        warnings   "Default"
        cppdialect "C++17"
        targetdir  ("lib/%{prj.name}")
        objdir     ("lib/%{prj.name}-int")
        vectorextensions "SSE4.1"
        location "core/launcher"


        flags {
            "FatalWarnings",
            "MultiProcessorCompile"
        }

        defines {
            "GLFW_STATIC",
            "GLAD_STATIC"
        }
        links {
            "test_module",
            "apparatus"
        }

        files { "%{prj.location}/**.cpp", "%{prj.location}/**.h" }

        includedirs {
            "core/apparatus/include/",
            "deps/glad/include",
            "deps/glfw/include",
            "deps/mz",
            "deps/imgui",
            "deps/spdlog/include",
            "deps/stb_image",
            "deps/entt/single_include",
            "deps/entt/src"
        }

        filter "system:windows"
            defines { "_OS_WINDOWS" }

        filter "system:linux"
            pic "On"
            defines { "_OS_LINUX" }

    project "parser"
        kind "ConsoleApp"
        language   "C++"
        warnings   "Default"
        cppdialect "C++17"
        targetdir  ("lib/%{prj.name}")
        objdir     ("lib/%{prj.name}-int")
        files { "%{prj.location}/**.cpp", "%{prj.location}/**.h" }
        location "core/parser"

    project "glfw"
        toolset    "gcc"
        location   "deps/glfw"
        kind       "StaticLib"
        language   "C"
        warnings   "Off"
        targetdir  ("lib/%{prj.name}")
        objdir     ("lib/%{prj.name}-int")

        files {
            "deps/glfw/include/glfw/glfw3.h",
            "deps/glfw/include/glfw/glfw3native.h",
            "deps/glfw/src/glfw_config.h",
            "deps/glfw/src/context.c",
            "deps/glfw/src/init.c",
            "deps/glfw/src/input.c",
            "deps/glfw/src/monitor.c",
            "deps/glfw/src/vulkan.c",
            "deps/glfw/src/window.c"
        }

        filter "system:windows"
            files {
                "deps/glfw/src/win32_init.c",
                "deps/glfw/src/win32_joystick.c",
                "deps/glfw/src/win32_monitor.c",
                "deps/glfw/src/win32_time.c",
                "deps/glfw/src/win32_thread.c",
                "deps/glfw/src/win32_window.c",
                "deps/glfw/src/wgl_context.c",
                "deps/glfw/src/egl_context.c",
                "deps/glfw/src/osmesa_context.c"
            }
        
            defines { 
                "_GLFW_WIN32"
            }

        filter "system:linux"
            pic "On"

            files {
                "deps/glfw/src/x11_init.c",
                "deps/glfw/src/x11_monitor.c",
                "deps/glfw/src/x11_window.c",
                "deps/glfw/src/xkb_unicode.c",
                "deps/glfw/src/posix_time.c",
                "deps/glfw/src/posix_thread.c",
                "deps/glfw/src/glx_context.c",
                "deps/glfw/src/egl_context.c",
                "deps/glfw/src/osmesa_context.c",
                "deps/glfw/src/linux_joystick.c"
            }

            defines {
                "_GLFW_X11"
            }
    
    project "glad"
        toolset    "gcc"
        location   "deps/glad"
        kind       "StaticLib"
        language   "C"
        warnings   "Off"
        targetdir  ("lib/%{prj.name}")
        objdir     ("lib/%{prj.name}-int")

        files {
            "deps/glad/include/glad/glad.h",
            "deps/glad/include/KHR/khrplatform.h",
            "deps/glad/src/glad.c"
        }
  
        includedirs { "deps/glad/include" }

        filter "system:linux"
            pic "On"
    
    project "imgui"
        location   "deps/imgui"
        kind       "StaticLib"
        language   "C++"
        warnings   "Off"
        targetdir  ("lib/%{prj.name}")
        objdir     ("lib/%{prj.name}-int")

        files
        {
            "deps/imgui/imconfig.h",
            "deps/imgui/imgui.h",
            "deps/imgui/imgui.cpp",
            "deps/imgui/imgui_draw.cpp",
            "deps/imgui/imgui_internal.h",
            "deps/imgui/imgui_widgets.cpp",
            "deps/imgui/imstb_rectpack.h",
            "deps/imgui/imstb_textedit.h",
            "deps/imgui/imstb_truetype.h",
            "deps/imgui/imgui_demo.cpp",
            "deps/imgui/backends/imgui_impl_opengl3.cpp",
            "deps/imgui/backends/imgui_impl_glfw.cpp",
            "deps/imgui/backends/imgui_impl_dx11.cpp",
            "deps/imgui/misc/cpp/imgui_stdlib.cpp"
        }
        
        includedirs 
        { 
            "deps/imgui",
            "deps/glfw/include",
            "deps/glad/include"
        }
        
        defines "IMGUI_IMPL_OPENGL_LOADER_GLAD"

    project "box2d"
        location   "deps/box2d"
        kind       "StaticLib"
        language   "C++"
        warnings   "Off"
        targetdir  ("lib/%{prj.name}")
        objdir     ("lib/%{prj.name}-int")

        files
        {
            "deps/box2d/src/**.cpp"
        }

        includedirs {
            "deps/box2d/src",
            "deps/box2d/include",
        }

    

