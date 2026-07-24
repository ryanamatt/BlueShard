# python/blueshard/__init__.py
"""
BlueShard - a small 3D scene/window framework.

The heavy lifting (math, rasterizing, SDL) lives in the compiled
`_blueshard` extension (see src/*.cpp). This file is pure Python: it
re-exports those classes and adds small conveniences - like the default
camera controller below - that are meant to be easy to read and copy
into your own script and change.
"""

from ._blueshard import Vector3, Camera, Mesh, Scene, Window, Key

__all__ = [
    "Vector3", "Camera", "Mesh", "Scene", "Window", "Key",
    "free_camera_controller",
]


def free_camera_controller(window: Window, camera: Camera, dt: float,
                            speed: float | None = None, sprint_multiplier: float = 3.0) -> None:
    """
    A default WASD + mouse-look controller.
    """
    dx, dy = window.mouse_delta()
    camera.look(dx, dy)

    move_speed = camera.move_speed if speed is None else speed
    step = move_speed * dt
    if window.key_down(Key.LSHIFT):
        step *= sprint_multiplier

    if window.key_down(Key.W):     camera.move_forward(step)
    if window.key_down(Key.S):     camera.move_forward(-step)
    if window.key_down(Key.D):     camera.move_right(step)
    if window.key_down(Key.A):     camera.move_right(-step)
    if window.key_down(Key.SPACE): camera.move_up(step)
    if window.key_down(Key.LCTRL): camera.move_up(-step)
