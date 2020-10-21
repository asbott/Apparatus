kind "SharedLib"
language   "C++"
warnings   "Default"
cppdialect "C++17"
targetdir  ("lib/%{prj.name}")
objdir     ("lib/%{prj.name}-int")
vectorextensions "SSE4.1"
location "modules/%{prj.name}"

flags {
    "FatalWarnings",
    "MultiProcessorCompile"
}

files { "modules/%{prj.name}/**.cpp", "modules/%{prj.name}/**.h" }


includedirs {
    "core/apparatus/include/",
    "deps/glad/include",
    "deps/glfw/include",
    "deps/mz",
    "deps/imgui",
    "deps/spdlog/include",
    "deps/stb_image",
    "deps/entt/single_include",
    "deps/entt/src",
    "deps/box2d/include",
    "deps",
    "%{wks.location}/",
    "%{prj.location}",
    "%{wks.location}/modules"
}

links {
    "apparatus",
    "imgui",
    "box2d"
}

filter "system:windows"
    prebuildcommands {
        "start /wait %{wks.location}lib\\parser\\parser.exe \"%{wks.location}\\modules\\%{prj.name}\\_generated.cpp\" \"%{prj.location}\"",
        "echo AP parser finished with exit code %ERRORLEVEL%"
    }

    postbuildcommands {
        "copy %{wks.location}lib\\%{prj.name}\\%{prj.name}.dll %{wks.location}lib\\launcher\\%{prj.name}.dll",
        "copy %{wks.location}lib\\%{prj.name}\\%{prj.name}.dll %{wks.location}lib\\runtime\\%{prj.name}.dll"
    }
    defines { "_OS_WINDOWS" }

filter "system:linux"
    prebuildcommands {
        "%{wks.location}/lib/parser/parser \"%{wks.location}/modules/%{prj.name}/_generated.cpp\" \"%{prj.location}\""
    }

    postbuildcommands {
        "mkdir -p %{wks.location}/lib/launcher",
        "mkdir -p %{wks.location}/lib/runtime",
        "cp -f %{wks.location}/lib/%{prj.name}/lib%{prj.name}.so %{wks.location}/lib/launcher/%{prj.name}.so",
        "cp -f %{wks.location}/lib/%{prj.name}/lib%{prj.name}.so %{wks.location}/lib/runtime/%{prj.name}.so"
    }
    pic "On"
    defines { "_OS_LINUX" }