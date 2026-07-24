#!/usr/bin/env python3
"""
Opening an Obj file demo. Run it with:

    python3 examples/open_obj.py
"""

import sys
import blueshard as bs

def main() -> None:
    scene = bs.Scene()
    scene.light_direction = bs.Vector3(0.2, -1.0, 0.3)

    statue = bs.Mesh.from_obj("assets/statue.obj", r=180, g=170, b=160)
    scene.add_mesh(statue, position=bs.Vector3(0, 0, 300), angular_velocity=bs.Vector3(0, 0.01, 0))

    window = bs.Window("Loading an Obj", 800, 600)

    while window.is_open():
        window.poll_events()
        dt = window.delta_time()

        if window.key_down(bs.Key.ESCAPE):
            break

        bs.free_camera_controller(window, scene.camera, dt)

        scene.update()

        window.clear(30, 30, 30)
        window.render_scene(scene)
        window.draw_text(f"FPS: {window.fps():.1f}", 10, 10)
        window.present()

if __name__ == "__main__":
    sys.exit(main() or 0)