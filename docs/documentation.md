# Apparatus documentation

## Table of content
- [Module function callbacks](#module-function-callbacks)
    - [on_load](#on-load)
    - [on_unload](#on-unload)
    - [on_update](#on-update)
    - [on_render](#on-render)
    - [on_gui](#on-gui)
    - [on_play_begin](#on-play-begin)
    - [on_play_stop](#on-play-end)
    - [save_to_disk](#save-to-disk)
    - [load_from_disk](#load-from-disk)
    - [get_function_library](#get_function_library)
- [Module system](#module-system)
    - [Core Module](#core-module)
    - [Standard modules](#standard-modules)
        - [2D Viewport](#2d-viewport)
        - [2D Editor](#2d-editor)
        - [2D Sprite Renderer](#2d-sprite-renderer)
        - [2D Tilemap Renderer](#2d-tilemap-renderer)
        - [2D Particles Simulator](#2d-particles-simulator)
        - [2D Physics](#2d-physics)
        - [Asset Manager](#asset-manaer)
        - [Scene Manager](#scene-manager)
- [Project Generation](#project-generation)
- [Functions](#functions)
- [Types](#types)
    - [Components](#components)
- [Asset Loaders](#asset-loaders)
- [Known bugs and limitations](#known-bugs-and-limitations)



# Module function callbacks

Signature:

`module_function(RETURN_TYPE) FUNCTION_NAME(ARGS)`

Example:

`module_function(void) save_to_disk(const char* project_path)`

To be detected by core, the function MUST be in a module scope like so:

    module_scope {
        module_function(void) save_to_disk(const char* project_path) {
            ...
        }
    }

Alternatively:

    module_scope module_function(void) save_to_disk(const char* project_path) {
        ...
    }

## Available callbacks
<h3 id ="on-load"><code>on_load</code></h3>

- Return type: `void`
- Arguments: <i>None</i>
- Invoked: when module is loaded
- Use case: Generally to register things to core and initialize data in the module.
- Signature: `module_function(void) on_load() {}`

<h3 id ="on-unload"><code>on_unload</code></h3>

- Return type: `void`
- Arguments: <i>None</i>
- Invoked: when module is unloaded
- Use case: Generally to unregister things registered in core and similar cleanup.
- Signature: `module_function(void) on_unload() {}`

<h3 id ="on-update"><code>on_update</code></h3>

- Return type: `void`
- Arguments: 
    - `float delta_time` - time passed last frame
- Invoked: Once every cycle in the application loop <b>while the game is running</b>
- Use case: To run game logic
- Signature: `module_function(void) on_update(float) {}`

<h3 id ="on-render"><code>on_render</code></h3>

- Return type: `void`
- Arguments: <i>None</i>
- Invoked: once at the end of every cycle in the application loop
- Use case: to render things to the screen at the end of the cycle
- Signature: `module_function(void) on_render() {}`

<h3 id ="on-gui"><code>on_gui</code></h3>

- Return type: `void`
- Arguments: <i>None</i>
- Invoked: once at the end of every cycle in the application loop just after on_render
- Use case: To submit things to ImGui to generate GUI. <b>Running ImGui code outside of this function is UB and likely to crash the application.</b>
- Signature: `module_function(void) on_gui() {}`

<h3 id ="on-play-begin"><code>on_play_begin</code></h3>

- Return type: `void`
- Arguments: <i>None</i>
- Invoked: when the game starts (in the editor: when you press play)
- Use case: To initialize things for the game
- Signature: `module_function(void) on_play_begin() {}`

<h3 id ="on-play-stop"><code>on_play_stop</code></h3>

- Return type: `void`
- Arguments: <i>None</i>
- Invoked: when the game stops (in the editor: when you press stop)
- Use case: To cleanup things from running the game
- Signature: `module_function(void) on_play_stop() {}`

<h3 id ="save-to-disk"><code>save_to_disk</code></h3>

- Return type: `void`
- Arguments: 
    - `const char* project_path` - the path of the current project being saved
- Invoked: when the project is saved in the editor
- Use case: To save state to disk when the current project is to be saved
- Signature: `module_function(void) save_to_disk(const char*) {}`

<h3 id ="load-from-disk"><code>load_from_disk</code></h3>

- Return type: `void`
- Arguments: 
    - `const char* project_path` - the path of the current project being loaded
- Invoked: when a project is loaded in the editor
- Use case: To load state from the disk for a project
- Signature: `module_function(void) load_from_disk(const char*) {}`

<h3 id ="get-function-library"><code>get_function_library</code></h3>

- Return type: `void*`
- Arguments: <i>None</i>
- Invoked: whenever by other modules
- Use case: To expose compiled functions to other modules
- Signature: `module_function(void*) get_function_library() { ... return X; }`

# Module system
Apparatus is entirely divided into modules (shared libraries) which are loaded dynamically in the runtime, with the exception of the core module. With this system Apparatus can be as modular as possible and very easily extended, all in native C++. 

For this to work, all modules are linked to a runtime (the Launcher) and the core module is also linked to all other modules which means that everything goes through the core module. This means that the non-core modules cannot communicate directly with each other but they must request information about other modules via the core. This system allows for all non-core modules to not be directly dependent on each other, making it easy to load and unload any single module without necessarily affecting the other modules. However some form of dependencies cannot be avoided, which is why there is a [module callback function](#get-function-library) that can be implemented to expose a type containing function pointers to compiled functions.

A visualization of Apparatus linkage:
![](../repo/linkage.png)

## Core Module
utility,
containers,
file management,
image loading,
graphics api,
thread server,
imgui extension

## Standard modules
module system designed for independency, but sometimes communication is needed I.E. asset system, scene system

### 2D Viewport
- Purpose:
    - Basic logic for 2D views & 2D transforms
    - To manage and clear 2D view render targets each render cycle
- Dependencies: <i>None</i>
- Types:
    - Components:
        - [`View2D`](#view2d)
        - [`Transform2D`](#transform2d)
- Constants: <i>None</i>
- Inline functions: <i>None</i>
- Exposed functions: <i>None</i>
- Asset Loaders: <i>None</i>
### 2D Editor
- Purpose:
    - Provides an editor viewport
    - Draws extra debug information over for things like physics, selection etc.
- Dependencies:
    - [2D Physics](#2d-physics)
    - [2D Viewport](#2d-viewport)
    - [2D Sprite Renderer](#2d-sprite-renderer)
    - [Asset Manager](#asset-manager)
- Types:
    - [`Editor_View`](#editor-view)
    - Components: <i>None</i>
- Constants: <i>None</i>
- Inline functions: <i>None</i>
- Exposed functions:
    - [`get_view`](#get-view)
- Asset Loaders: <i>None</i>
### 2D Sprite Renderer
- Purpose:
- Dependencies:
    - [2D Viewport](#2d-viewport)
    - [2D Editor](#2d-editor)
    - [Asset Manager](#asset-manager)
- Types:
    - Components:
        - [`Sprite2D`](#sprite2d)
        - [`SpriteAnimation2D`](#spriteanimation2d)
- Constants: <i>None</i>
- Inline functions: <i>None</i>
- Exposed functions: <i>None</i>
- Asset Loaders: 
    - [Texture Sheet 2D](#texture-sheet-2d)
    - [Sprite Animation 2D Preset](#sprite-animation-2d-preset)
    - [Texture](#texture)
### 2D Tilemap Renderer
- Purpose: TBD
- Dependencies:
- Types:
    - Components:
- Constants:
- Inline functions:
- Exposed functions:
- Asset Loaders:
### 2D Particles Simulator
- Purpose:
- Dependencies: 
    - [2D Viewport](#2d-viewport)
    - [2D Editor](#2d-editor)
    - [2D Sprite Renderer](#2d-sprite-renderer)
- Types:
    - [Particle_Simulation_State](#particle_simulation_state)
    - Components:
        - [ParticleSimulation2D](#particlesimulation2d)
- Constants: <i>None</i>
- Inline functions: <i>None</i>
- Exposed functions: <i>None</i>
- Asset Loaders: <i>None</i>
### 2D Physics
- Purpose:
- Dependencies:
- Types:
    - Components:
- Constants:
- Inline functions:
- Exposed functions:
- Asset Loaders: <i>None</i>
### Asset Manager
- Purpose:
- Dependencies:
- Types:
    - Components:
- Constants:
- Inline functions:
- Exposed functions:
- Asset Loaders: <i>None</i>
### Scene Manager
- Purpose:
- Dependencies:
- Types:
    - Components:
- Constants:
- Inline functions:
- Exposed functions:
- Asset Loaders: <i>None</i>

# Project Generation

# Functions

# Types

## Components

# Asset Loaders

# Known bugs and limitations
- The Apparatus Parser will only properly detect members in structs if the type is a single word

Example:

    typedef Data* data_ptr_t;
    struct SomeComponent {
        tag(property)
        Data* my_data1; // won't be detected properly by parser, UB

        tag(property)
        data_ptr_t my_data2; // ok
    };