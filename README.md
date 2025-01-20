# Simple Surface
## What is it?
Simple Surface lets you quickly add color and a bit of texture to actors' mesh components.  Useful for visual variation during blockout and prototyping.

https://github.com/user-attachments/assets/4d05b12b-522a-4a56-9002-380fa7f082ef

This is a quality-of-life plugin designed to be an easy-to-use, non-intrusive addition to your workflow, to help you achieve some visual variety without needing to create throwaway assets.

## How do I use it?
Add the Simple Surface component to any actor having one or more mesh components.

![SimpleSurface](https://github.com/user-attachments/assets/a972d4a1-959a-43bc-a616-216e5b9d8e00)

Now you can change its color and some simple surface properties to get a different look:
* Shininess / Roughness
* Waxiness / Metalness
* Texture (normal mapping) intensity, scale, and customization

If you remove the component (or deactivate it using Blueprint), the meshes' original materials will be restored.

Works with most mesh components available in Unreal Engine 5.5.
