kind "SharedLib"
language   "C++"
warnings   "Extra"
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

prebuildcommands {
    "start /wait %{wks.location}lib\\parser\\parser.exe \"%{wks.location}\\modules\\%{prj.name}\\_generated.cpp\" \"%{prj.location}\"",
    "echo AP parser finished with exit code %ERRORLEVEL%"
}

postbuildcommands {
    "copy %{wks.location}lib\\%{prj.name}\\%{prj.name}.dll %{wks.location}lib\\launcher\\%{prj.name}.dll",
    "copy %{wks.location}lib\\%{prj.name}\\%{prj.name}.dll %{wks.location}lib\\runtime\\%{prj.name}.dll"
}

disablewarnings {
    "4324"
}

includedirs {
    "core/apparatus/include/",
    "deps/glad/include",
    "deps/glfw/include",
    "deps/mz",
    "deps/imgui",
    "deps/spdlog/include",
    "deps/stb_image",
    "deps/entt/single_include",
    "deps",
    "%{wks.location}/",
    "%{prj.location}",
    "%{wks.location}/modules"
}

links {
    "apparatus",
    "imgui"
}

filter "system:windows"
    defines { "_OS_WINDOWS" }

filter "system:linux"
    pic "On"
    defines { "_OS_LINUX" }