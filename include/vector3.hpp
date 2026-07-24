// include/vector3.hpp

#pragma once

class Vector3 {
public:
    float x, y, z;

    Vector3(float x = 0.0f, float y = 0.0f, float z = 0.0f) 
        : x(x), y(y), z(z) {}
};