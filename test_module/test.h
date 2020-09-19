#pragma once

// #component
struct Transform {
	// #serializable
	// the transform matrix 
	mz::fmat4 value;
};

/*

	#component

*/
struct SpriteComponent {
	float size;
};

// #component
struct AnotherComponent {
	float data;
};