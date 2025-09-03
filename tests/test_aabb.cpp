#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include <core/aabb.hpp>

TEST_CASE("computeAABB basic") 
{
    const float v[] = { -1.f, 2.f, 3.f,  4.f, -2.f, 0.f };
    auto b = core::computeAABB(v, 2);

    CHECK(b.min.x == doctest::Approx(-1.f));
    CHECK(b.min.y == doctest::Approx(-2.f));
    CHECK(b.min.z == doctest::Approx(0.f));
    CHECK(b.max.x == doctest::Approx(4.f));
    CHECK(b.max.y == doctest::Approx(2.f));
    CHECK(b.max.z == doctest::Approx(3.f));
}
