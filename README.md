# Advanced Graphics Engine

## Deferred Renderer
Features
- Static 3d model loading
- Embedded Geometry (Primitives): Plane, Sphere & Cube
- G-Buffer (Framebuffer Object) with different render targets attached: Position, Normals, Albedo, Specular & Depth
- Render to screen-filling quad
- Light Casters: Point & Directional Lights
- Free camera roaming
- ImGui to check engine's performance, change current render target & debugging

## Controls

| Action | Input |
| :---: | :---: |
| **Camera Forwards** | W |
| **Camera Backwards** | S |
| **Camera Left** | A |
| **Camera Right** | D |
| **Camera Up** | Q |
| **Camera Down** | E |
| **Camera Rotation** | LEFT MOUSE |
| **Quit** | ESC |

### Render Targets
The ImGui window showing up at startup allows to change the render target through a combo box.
