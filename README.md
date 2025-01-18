# Simple Surface
## What is it?
Simple Surface lets you quickly add color and a bit of texture to mesh actors.

## Why should I care?
When an editor like Unreal offers me a drag-and-drop workflow for simple primitives, you might expect to be able to change those primitives' color without having to hook anything up, like you can in one of my favorite editor UXs, [Dreams](https://www.playstation.com/en-us/games/dreams/).

This is a quality-of-life plugin designed to be an easy-to-use, non-intrusive addition to your workflow.  I made it for myself and all other Unreal Engine creators who've ever wished they could simply change a cube's color after dropping it into a scene, without hunting for a material, creating a new "throwaway" shader for the nth time, or whipping up a Blueprint that connects a color picker in the Details panel to a mesh's material.

## How do I use it?
Add the Simple Surface component to any actor having one or more mesh components.

Now you can change its color and some simple surface properties to get a different look:
* Shininess / Roughness
* Waxiness / Metalness
* Texture (normal mapping) intensity, scale, and customization

If you remove the component (or deactivate it using Blueprint), the meshes' original materials will be restored.

## How do I get it?
It's not quite ready yet.  Stay tuned!
