#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <core/bvh.hpp>

TEST_CASE("BVH picks same as brute on a cube") {
    // unit cube verts
    const float v[] = {
        -0.5f,-0.5f,-0.5f,  +0.5f,-0.5f,-0.5f,
        +0.5f,+0.5f,-0.5f,  -0.5f,+0.5f,-0.5f,
        -0.5f,-0.5f,+0.5f,  +0.5f,-0.5f,+0.5f,
        +0.5f,+0.5f,+0.5f,  -0.5f,+0.5f,+0.5f,
    };
    const unsigned idx[] = {
        0,1,2, 2,3,0,  5,4,7, 7,6,5,  4,0,3, 3,7,4,
        1,5,6, 6,2,1,  4,5,1, 1,0,4,  3,2,6, 6,7,3,
    };
    const std::size_t tris = sizeof(idx) / sizeof(idx[0]) / 3;

    core::BVH bvh = core::buildBVH(v, idx, tris, 4);

    // A ray from +Z toward origin should hit front face
    core::Ray r; r.origin = { 0,0, 5 }; r.dir = glm::normalize(glm::vec3{ 0,0,-1 });
    core::RayHit hb{}, hr{};
    bool okB = core::raycastBVH(r, v, bvh, hb);
    bool okR = core::raycastMesh(r, v, idx, tris, hr);
    CHECK(okB == okR);
    if (okB && okR) CHECK(hb.t == doctest::Approx(hr.t));
}
