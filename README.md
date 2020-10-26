# Apparatus
Modular game engine with fully native runtime gameplay
programming.

# Building

1. Clone the repo
    
    `git clone https://github.com/asbott/Apparatus my_repo_dir`
2. Initialize and update submodules
    
    `git submodule update --init`
3. Generate project files
    - Windows, Visual Studio 2019: 
        
        `premake5.exe vs2019`
    - Unix, makefiles: 
    
        `./premake5 gmake2`
4. Build the <i>parser</i> project
    - With Visual Studio 2019: Right click the <i>parser</i> project and hit <i>build</i>
    - With makefiles & make: 

        `make parser`


Apparatus should be fully functional on Linux systems using x11. Has been tested with Manjaro 20.1.2, compiling with g++ 10.2.0.

# Showcase
Runtime management of entites/components (entt backend)
![](repo/entitiescomponents.gif)

Asset management and dnd
![](repo/assetmanagement.gif)

Editor viewport & interactions (click to select, drag, multiselect)
![](repo/editorinteraction.gif)

2D physics (box2d backend)
![](repo/2dphysics.gif)

Multiple cameras
![](repo/multiplecameras.gif)

Runtime c++ compiling (hot reloading modules), language extension & introspection. Say no to scripting. Just pure C++ code.
![](repo/runtimecpp.gif)

Entt api for gameplay programming in modules
![](repo/runtimecppentt.gif)

Moreover, everything is pretty much fully modular with the runtime module system so it's very easy to extend and add or customize features. For example, if you don't like the default renderer you can replace it with your own DLL, as long as it is a valid apparatus module and use the same .h file.