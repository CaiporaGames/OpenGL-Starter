#include <doctest/doctest.h>
#include <core/camera/OrbitCamera3D.hpp>
using core::OrbitCamera3D;

TEST_CASE("orbit position axes") {
    OrbitCamera3D cam;
    cam.setTarget({ 0,0,0 });
    cam.setDistance(5.f);
    cam.setViewport(800, 600);
    cam.setLens(60.f, 0.1f, 1000.f);

    cam.setAngles(0.f, 0.f);
    CHECK(cam.position().x == doctest::Approx(0.f));
    CHECK(cam.position().y == doctest::Approx(0.f));
    CHECK(cam.position().z == doctest::Approx(5.f));

    cam.setAngles(90.f, 0.f);
    CHECK(cam.position().x == doctest::Approx(5.f));
    CHECK(cam.position().y == doctest::Approx(0.f));
    CHECK(cam.position().z == doctest::Approx(0.f));
}

TEST_CASE("pitch clamp") {
    OrbitCamera3D cam;
    cam.setAngles(0.f, 120.f);
    // internal clamp to 89 deg; we can only assert position finite (no gimbal NaN)
    auto p = cam.position();
    CHECK(std::isfinite(p.x));
    CHECK(std::isfinite(p.y));
    CHECK(std::isfinite(p.z));
}
