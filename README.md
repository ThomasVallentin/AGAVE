<div align="center">
  <img src="https://user-images.githubusercontent.com/43669549/228212940-6f01061f-e457-4e71-b081-89d19aa4b53b.png" width=128 height=128>
  <H1>AGAVE - Animated Geometric Algebra Viewer for Experiments</H1>
</div>

**AGAVE** is a procedural tool to generate shapes using Conformal Geometric Algebra. It is my end-of-the-year project for the Geometric Algebra course I took at Université Gustave Eiffel.

[![Demo](https://user-images.githubusercontent.com/43669549/228780772-f63ac6d8-2f67-44ed-93f0-40e8f85d3ea9.png)](https://youtu.be/BwJDvLdODqc)

## Dependencies

AGAVE relies on some dependencies, they are not stored as submodules for now but this will come soon :

- Garamon (C3GA)
- Glad (OpenGL 4.6)
- GLFW3
- glm
- Dear ImGui
- stb

## Install

```
git clone --recurse-submodules https://github.com/ThomasVallentin/AGAVE.git
cd AGAVE
mkdir build
cd build
cmake ..
make
```

