#!/usr/bin/env python3
"""
10 Spinning Boxes demo (randomly placed/colored boxes, free
WASD + mouse-look camera) recreated as a scene script. Run it with:

    python3 examples/spinning_boxes.py
"""

import random
import sys
import blueshard as bs


def build_scene() -> bs.Scene:
    scene = bs.Scene()

    scene.camera.position = bs.Vector3(0.0, 0.0, -600.0)

    for _ in range(10):
        center = bs.Vector3(
            random.uniform(-300.0, 300.0),
            random.uniform(-300.0, 300.0),
            400.0 + random.uniform(-300.0, 300.0),
        )
        size = random.uniform(30.0, 90.0)
        angular_velocity = bs.Vector3(
            random.uniform(-0.015, 0.015),
            random.uniform(-0.015, 0.015),
            random.uniform(-0.015, 0.015),
        )
        scene.add_cube(center, size, angular_velocity)

    return scene


def main() -> None:
    scene = build_scene()
    window = bs.Window("Spinning Boxes", 800, 600)

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
