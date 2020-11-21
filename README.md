# Apparatus
Modular game engine with fully native runtime gameplay
programming.

# Note
I haven't been able to put much work into this as it's a hobby project and I have
a very high rent to pay with a very low pay. It's likely that I won't be able to
put any work in at all soon as it seems that I'm going to have to sell my laptop
to pay the rent. Feel free to contribute while I'm gone, I'll have a crappy laptop
to at least check in on the repo.

# Plan
I got tired of keeping long physical lists of TODO's, so I created a trello board
to just dump everything I can think of in there so I don't forget about it and
always have something to do. The board can be found <a href="https://trello.com/b/xe6pIKCK/apparatus">here</a>.

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
(These gifs are of a very outdated version1)

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