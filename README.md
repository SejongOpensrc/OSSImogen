# Imogen

<h3>- 2조 협업 OSS, Imogen - team repository! </h3> 
<h4><a href="https://github.com/SejongOpensrc">팀룸으로 돌아가기</a></h4>
<h3>- <a href='https://github.com/SejongOpensrc/OSSImogen/projects/1'> 프로젝트 진행 사항 살펴보기 </a></h3>

-----------

:GPU/CPU Texture Generator

WIP of a GPU Texture generator using dear imgui for UI. Not production ready and a bit messy but really fun to code.
Basically, add GPU and CPU nodes in a graph to manipulate and generate images. Nodes are hardcoded now but a discovery system is planned.
Currently nodes can be written in GLSL or C. Python  is coming next.
![Image of Imogen 0.4](https://i.imgur.com/pmliWGl.png)
![Image of Imogen 0.4](https://i.imgur.com/jNWsXD6.png)

Use CMake and VisualStudio to build it. Only Windows system supported for now.

Features:
- Node based texture editing
- material library browser
- edit/change node shaders inside the app
- bake textures to .png, .jpg, .tga, .bmp, .hdr
- PBR preview

Currently implemented nodes
- circle and square generator
- sine generator
- checker transform
- transform
- Mul/Add
- smoothstep
- pixelize
- blur
- normal map from height map
- sphere/plan previewer
- Hexagon
- Mul-Add colors
- Blend (add, mul, min, max)
- Invert color
- Circle Splatter
- Ramp
- Tile
- Polar coordinates
- ...

Check the project page for roadmap.

-----------
This software uses the following (awesome) projects:

Dear ImGui - Omar Cornut
a bloat-free graphical user interface library for C++.
https://github.com/ocornut/imgui

ImGuiColorTextEdit - Balazs Jako
Syntax highlighting text editor for ImGui.
https://github.com/BalazsJako/ImGuiColorTextEdit

stb_image, stb_image_write - Sean T. Barett 
single-file public domain (or MIT licensed) libraries for C/C++.
https://github.com/nothings/stb

imguiDock - Bentley Blanks 
An addon of imgui for support dock in the window.
https://github.com/BentleyBlanks/imgiuiDock

EnkiTS - Doug Binks 
A permissively licensed C and C++ Task Scheduler for creating parallel programs.
https://github.com/dougbinks/enkiTS

Tiny C Compiler - Fabrice Bellard 
an x86, X86-64 and ARM processor C compiler. It is designed to work for slow computers with little disk space.
https://bellard.org/tcc/

SDL2
a cross-platform development library designed to provide low level access to audio, keyboard, mouse, joystick, and graphics hardware via OpenGL and Direct3D.
https://www.libsdl.org/

NativeFileDialog - Michael Labbe 
A tiny, neat C library that portably invokes native file open, folder select and save dialogs.
https://github.com/mlabbe/nativefiledialog

gl3w - Slavomir Kaslev 
gl3w is the easiest way to get your hands on the functionality offered by the OpenGL core profile specification.
https://github.com/skaslev/gl3w

TinyDir
Lightweight, portable and easy to integrate C directory and file reader. TinyDir wraps dirent for POSIX and FindFirstFile for Windows.
https://github.com/cxong/tinydir

cmft - cubemap filtering tool - Dario Manesku 
Cross-platform open-source command-line cubemap filtering tool.
https://github.com/dariomanesku/cmft

dear imgui color scheme - codz01
https://github.com/ocornut/imgui/issues/1902#issuecomment-429445321


-------------
How to use Imogen(이모젠의 사용) : https://www.youtube.com/watch?v=84QgFdZb7eM

![Image of Imogen 0.4](https://i.imgur.com/vpNaA8h.png)
