// src/bindings.cpp

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "vector3.hpp"
#include "camera.hpp"
#include "mesh.hpp"
#include "scene.hpp"
#include "window.hpp"

#include <string>

namespace py = pybind11;

PYBIND11_MODULE(_blueshard, m) {
    m.doc() = "Compiled rendering core for BlueShard";

    py::class_<Vector3>(m, "Vector3")
        .def(py::init<float, float, float>(),
             py::arg("x") = 0.0f, py::arg("y") = 0.0f, py::arg("z") = 0.0f)
        .def_readwrite("x", &Vector3::x)
        .def_readwrite("y", &Vector3::y)
        .def_readwrite("z", &Vector3::z)
        .def("__repr__", [](const Vector3& v) {
            return "Vector3(" + std::to_string(v.x) + ", "
                              + std::to_string(v.y) + ", "
                              + std::to_string(v.z) + ")";
        });

    py::class_<Camera>(m, "Camera")
        .def(py::init<>())
        .def_readwrite("position", &Camera::position)
        .def_readwrite("yaw", &Camera::yaw)
        .def_readwrite("pitch", &Camera::pitch)
        .def_readwrite("move_speed", &Camera::moveSpeed)
        .def_readwrite("look_sensitivity", &Camera::lookSensitivity)
        .def("forward", &Camera::forward)
        .def("right", &Camera::right)
        .def("up", &Camera::up)
        .def("look", &Camera::look, py::arg("dx"), py::arg("dy"))
        .def("move_forward", &Camera::moveForward, py::arg("amount"))
        .def("move_right", &Camera::moveRight, py::arg("amount"))
        .def("move_up", &Camera::moveUp, py::arg("amount"));

    // shared_ptr holder: objects handed back by Scene.add_cube() stay
    // alive and mutable from Python as long as something (Python or the
    // Scene) still holds a reference to them.
    py::class_<Mesh, std::shared_ptr<Mesh>>(m, "Mesh")
        .def_readwrite("position", &Mesh::position)
        .def_readwrite("angular_velocity", &Mesh::angularVelocity)
        .def_static("cube", &Mesh::cube, py::arg("size"))
        .def_static("plane", &Mesh::plane,
             py::arg("width"), py::arg("height"),
             py::arg("r") = 200, py::arg("g") = 200, py::arg("b") = 200, py::arg("a") = 255)
        .def_static("sphere", &Mesh::sphere,
             py::arg("radius"), py::arg("segments") = 16,
             py::arg("r") = 200, py::arg("g") = 200, py::arg("b") = 200, py::arg("a") = 255)
        .def_static("cylinder", &Mesh::cylinder,
             py::arg("radius"), py::arg("height"), py::arg("segments") = 16,
             py::arg("r") = 200, py::arg("g") = 200, py::arg("b") = 200, py::arg("a") = 255)
        .def_static("from_obj", &Mesh::fromOBJ,
             py::arg("path"),
             py::arg("r") = 200, py::arg("g") = 200, py::arg("b") = 200, py::arg("a") = 255);

    py::class_<Scene>(m, "Scene")
        .def(py::init<>())
        .def_readwrite("camera", &Scene::camera)
        .def_readwrite("light_direction", &Scene::lightDirection)
        .def("add_cube", &Scene::addCube,
             py::arg("center"), py::arg("size"), py::arg("angular_velocity"))
        .def("add_mesh", &Scene::addMesh,
             py::arg("mesh"), py::arg("position") = Vector3(), py::arg("angular_velocity") = Vector3())
        .def("update", &Scene::update)
        .def("objects", &Scene::objects);

    py::enum_<Key>(m, "Key")
        .value("W", Key::W)
        .value("A", Key::A)
        .value("S", Key::S)
        .value("D", Key::D)
        .value("SPACE", Key::Space)
        .value("LCTRL", Key::LCtrl)
        .value("LSHIFT", Key::LShift)
        .value("ESCAPE", Key::Escape);

    py::class_<Window>(m, "Window")
        .def(py::init<const std::string&, int, int>(),
             py::arg("title"), py::arg("width") = 800, py::arg("height") = 600)
        .def("is_open", &Window::isOpen)
        .def("poll_events", &Window::pollEvents)
        .def("delta_time", &Window::deltaTime)
        .def("fps", &Window::fps)
        .def("key_down", &Window::keyDown, py::arg("key"))
        .def("mouse_delta", &Window::mouseDelta)
        .def("clear", &Window::clear,
             py::arg("r") = 30, py::arg("g") = 30, py::arg("b") = 30, py::arg("a") = 255)
        .def("render_scene", &Window::renderScene, py::arg("scene"))
        .def("draw_text", &Window::drawText, py::arg("text"), py::arg("x"), py::arg("y"))
        .def("present", &Window::present);
}