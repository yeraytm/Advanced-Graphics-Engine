# Advanced Graphics Engine
- [GitHub Repository](https://github.com/yeraytm/Advanced-Graphics-Engine)
- [Latest Release](https://github.com/yeraytm/Advanced-Graphics-Engine/releases)

## Engine Features
- Static 3d model loading
- Embedded Geometry (Primitives): Plane, Sphere & Cube
- Light Casters: Point & Directional Lights
- Free camera roaming
- ImGui

## Deferred Renderer Features
Features:
- G-Buffer (Framebuffer Object) with different render targets attached: Position, Normals, Albedo, Specular & Depth
- Render to screen-filling quad
- Geometry & Lighting Pass

## Controls

| Action | Input |
| :---: | :---: |
| **Forwards** | W |
| **Backwards** | S |
| **Left** | A |
| **Right** | D |
| **Up** | Q |
| **Down** | E |
| **Rotation** | MOUSE LEFT |
| **Zoom** | MOUSE WHEEL |
| **Quit** | ESC |

## ImGui Windows
### Renderer
To select the render target that will be displayed in screen through a combo box. It shows up at startup.

### Scene
- Camera's parameters of position, speed and zoom.
- Lights: direction of the directional light and colors of point lights.

### OpenGL
Information about the current version of OpenGL and GLSL and GPU information. Moreover, a list of all available extension of OpenGL.

### Performance
Check current framerate (FPS), frametime (dt) and time since startup.
