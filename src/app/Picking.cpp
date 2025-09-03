#include "app/Picking.hpp"
#include "app/App.hpp" // to access your fields
#include <chrono>

namespace picking 
{
    void handle3D(App& app, double sx, double sy, bool useBVH, bool* outHit) 
    {
        const glm::mat4 invVP = glm::inverse(app.scene()->activeCameraVP());
        core::Ray rayW = core::screenRayFromInvVP(invVP, sx, sy, app.fbw(), app.fbh());

        const glm::mat4 model = app.hasLoadedMesh() ? app.loadedModel() : app.cubeModel();
        const glm::mat4 invM = glm::inverse(model);
        core::Ray rM;
        rM.origin = glm::vec3(invM * glm::vec4(rayW.origin, 1.0));
        rM.dir = glm::normalize(glm::vec3(invM * glm::vec4(rayW.dir, 0.0)));

        const float* pos = app.hasLoadedMesh() ? app.loadedGL().positions() : app.cube().cpuPositions();
        const unsigned* idx = app.hasLoadedMesh() ? app.loadedGL().indices() : app.cube().cpuIndices();
        const std::size_t tris = app.hasLoadedMesh() ? app.loadedGL().triCount() : app.cube().triCount();
        const core::BVH& bvh = app.hasLoadedMesh() ? app.loadedBVH() : app.bvh();

        core::RayHit hitBVH{}, hitBrute{};
        bool okBVH = false, okBrute = false;

        core::BVHStats st{};
        auto t0 = std::chrono::high_resolution_clock::now();
        okBVH = core::raycastBVH(rM, pos, bvh, hitBVH, &st);
        auto t1 = std::chrono::high_resolution_clock::now();
        double msBVH = std::chrono::duration<double, std::micro>(t1 - t0).count() / 1000.0;

        t0 = std::chrono::high_resolution_clock::now();
        okBrute = core::raycastMesh(rM, pos, idx, tris, hitBrute);
        t1 = std::chrono::high_resolution_clock::now();
        double msBrute = std::chrono::duration<double, std::micro>(t1 - t0).count() / 1000.0;

        const bool ok = useBVH ? okBVH : okBrute;
        const core::RayHit& hit = useBVH ? hitBVH : hitBrute;

        if (!ok) {
            app.triLines().clear();
            app.hitCross().clear();
            app.setPickHasHit(false);
            std::printf("Pick: none (BVH=%.3f ms, brute=%.3f ms)\n", msBVH, msBrute);
            if (outHit) *outHit = false;
            return;
        }

        unsigned i0, i1, i2;
        if (useBVH) {
            const unsigned base = 3u * hit.triIndex;
            i0 = bvh.indices[base + 0];
            i1 = bvh.indices[base + 1];
            i2 = bvh.indices[base + 2];
        }
        else {
            i0 = idx[3u * hit.triIndex + 0];
            i1 = idx[3u * hit.triIndex + 1];
            i2 = idx[3u * hit.triIndex + 2];
        }
        auto P = [&](unsigned vi) { return glm::vec3(pos[3 * vi + 0], pos[3 * vi + 1], pos[3 * vi + 2]); };
        const glm::vec3 v0 = P(i0), v1 = P(i1), v2 = P(i2);
        const float w = 1.0f - hit.u - hit.v;
        const glm::vec3 pM = w * v0 + hit.u * v1 + hit.v * v2;

        const glm::vec3 aW = glm::vec3(model * glm::vec4(v0, 1));
        const glm::vec3 bW = glm::vec3(model * glm::vec4(v1, 1));
        const glm::vec3 cW = glm::vec3(model * glm::vec4(v2, 1));
        const glm::vec3 pW = glm::vec3(model * glm::vec4(pM, 1));

        app.triLines().setTriangle(aW, bW, cW);
        app.hitCross().setCross(pW, 0.15f);
        app.setPickHasHit(true);

        std::printf("Pick tri=%d t=%.3f (u=%.3f,v=%.3f)  BVH=%.3f ms | brute=%.3f ms\n",
            hit.triIndex, hit.t, hit.u, hit.v, msBVH, msBrute);
        if (outHit) *outHit = true;
    }
} // ns
