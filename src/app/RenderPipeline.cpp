#include "app/RenderPipeline.hpp"
#include "app/App.hpp"
#include <glad/glad.h>
#include <imgui.h>

namespace renderpipe {

    void draw3D(App& app)
    {
        // GL state for 3D
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Frustum culling (uses ECS camera VP)
        {
            const core::Frustum fr = core::extractFrustum(app.scene()->activeCameraVP());
            app.setObjTotal(1);
            if (app.hasLoadedMesh()) {
                app.setCubeVisible(!core::aabbOutsideFrustum(fr, app.loadedAABBWorld()));
            }
            else {
                app.setCubeVisible(!core::aabbOutsideFrustum(fr, app.cubeAABBWorld()));
            }
            app.setObjVisible(app.cubeVisible() ? 1 : 0);
        }

        app.basic3D().use();

        if (!app.cubeVisible()) return;

        const glm::mat4 model = app.hasLoadedMesh() ? app.loadedModel() : app.cubeModel();
        const glm::mat4 VP = app.scene()->activeCameraVP();
        const glm::mat4 MVP = VP * model;

        if (app.uMVP() != -1) glUniformMatrix4fv(app.uMVP(), 1, GL_FALSE, &MVP[0][0]);
        if (app.uColor() != -1) glUniform3f(app.uColor(), 0.25f, 0.6f, 0.85f);

        if (app.hasLoadedMesh()) app.loadedGL().draw();
        else                    app.cube().draw();

        // --- Debug overlays in world space ---
        if (app.pickHasHit() && app.showHitViz()) {
            const glm::mat4 MVPw = VP;
            if (app.uMVP() != -1) glUniformMatrix4fv(app.uMVP(), 1, GL_FALSE, &MVPw[0][0]);
            if (app.uColor() != -1) glUniform3f(app.uColor(), 1.0f, 0.95f, 0.2f);
            app.triLines().draw();

            if (app.uColor() != -1) glUniform3f(app.uColor(), 1.0f, 0.25f, 1.0f);
            app.hitCross().draw();
        }

        if (app.showAABB()) {
            const glm::mat4 MVPw = VP;
            if (app.uMVP() != -1) glUniformMatrix4fv(app.uMVP(), 1, GL_FALSE, &MVPw[0][0]);
            if (app.uColor() != -1) glUniform3f(app.uColor(), 0.9f, 0.9f, 0.9f);
            app.box().draw();
        }
    }

    void draw2D(App& app)
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        if (app.scene()) 
        {
            // Text pass (world-space 2D)
            app.spriteBatch().beginWithVP(app.scene()->uiCamera().vp());
            app.spriteBatch().setTexture(app.uiFont().text);
            app.spriteBatch().setSampleMode(2);
            app.scene()->render2D(&app, app.spriteBatch());
            app.spriteBatch().endAndDraw();

            // Screen-space overlay (pixel ortho)
            {
                app.spriteBatch().begin(app.fbw(), app.fbh());
                app.spriteBatch().setTexture(app.uiFont().text);
                app.spriteBatch().setSampleMode(2);

                char buf[256];
                const bool AM = app.hasLoadedMesh();
                const auto& aabb = AM ? app.loadedAABBWorld() : app.cubeAABBWorld();
                const std::size_t vtx = AM ? app.loadedGL().vertexCount() : 8;
                const std::size_t tri = AM ? app.loadedGL().triCount() : 12;
                std::snprintf(buf, sizeof(buf),
                    "FPS: %.1f\nVerts: %zu  Tris: %zu\nVisible: %d / %d\nBVH nodes: %d\n"
                    "AABB min: [%.2f %.2f %.2f]\nAABB max: [%.2f %.2f %.2f]",
                    app.fps(), vtx, tri, app.objVisible(), app.objTotal(),
                    AM ? (int)app.loadedBVH().nodes.size() : app.bvhNodes(),
                    aabb.min.x, aabb.min.y, aabb.min.z,
                    aabb.max.x, aabb.max.y, aabb.max.z);


                glm::vec2 glyph = { 8.0f, 12.0f };
                glm::vec4 col = { 0.95f, 0.95f, 0.95f, 1.0f };
                float pad = 8.0f;
                glm::vec2 bl = { pad, float(app.fbh()) - pad - glyph.y };
                app.uiFont().drawTextBL(app.spriteBatch(), buf, bl, glyph, col, 1.0f, 2.0f);
                app.spriteBatch().endAndDraw();
            }
        }

        if (app.showImGui()) {
            if (ImGui::Begin("Debug"))
            {
                ImGui::Text("FPS: %0.1f", app.fps());
                app.fpsHistory()[app.fpsHead()] = app.fps();
                app.setFpsHead((app.fpsHead() + 1) % 120);
                ImGui::PlotLines("fps", app.fpsHistory(), 120, app.fpsHead(), nullptr, 0.0f, 240.0f, ImVec2(0, 60));

                ImGui::SeparatorText("Render");
                bool vs = app.vsyncOn();         // const getter
                if (ImGui::Checkbox("VSync (V)", &vs)) {
                    app.setVsyncOn(vs);
                    glfwSwapInterval(vs ? 1 : 0);
                }

                if (ImGui::IsItemEdited()) glfwSwapInterval(app.vsyncOn() ? 1 : 0);

                bool wf = app.wireframe();
                if (ImGui::Checkbox("Wireframe (W)", &wf)) {
                    app.setWireframe(wf);
                    glPolygonMode(GL_FRONT_AND_BACK, wf ? GL_LINE : GL_FILL);
                }
                if (ImGui::IsItemEdited())
                    glPolygonMode(GL_FRONT_AND_BACK, app.wireframe() ? GL_LINE : GL_FILL);

                bool aabb = app.showAABB();
                if (ImGui::Checkbox("Show AABB", &aabb)) app.setShowAABB(aabb);
                bool hv = app.showHitViz();
                if (ImGui::Checkbox("Show Hit viz", &hv)) app.setShowHitViz(hv);

                ImGui::SeparatorText("Picking");
                bool bvh = app.useBVH();
                if (ImGui::Checkbox("Use BVH (B)", &bvh)) app.setUseBVH(bvh);
                ImGui::Text("Has hit: %s", app.pickHasHit() ? "yes" : "no");

                ImGui::SeparatorText("Camera");
                ImGui::Text("Active VP source: ECS CameraComponent");
                if (app.wantScreenshot()) {
                    char name[128];
                    std::snprintf(name, sizeof(name), "screenshot_%dx%d.tga", app.fbw(), app.fbh());
                    if (Screenshot::saveTGA(name, app.fbw(), app.fbh())) std::printf("[Screenshot] saved %s\n", name);
                    else                                               std::printf("[Screenshot] failed\n");
                    app.clearScreenshotRequest();
                }
            }
            ImGui::End();

            if (ImGui::Begin("GL Log"))
            {
                const auto& msgs = GLDebug::messages();
                ImGui::BeginChild("glmsg", ImVec2(0, 0), true);
                for (const auto& m : msgs) {
                    ImGui::Text("[%s] id=%u %s",
                        m.severity == GL_DEBUG_SEVERITY_HIGH ? "HIGH" :
                        m.severity == GL_DEBUG_SEVERITY_MEDIUM ? "MEDIUM" :
                        m.severity == GL_DEBUG_SEVERITY_LOW ? "LOW" : "NOTE",
                        m.id, m.text.c_str());
                }
                ImGui::EndChild();
                if (ImGui::Button("Clear")) GLDebug::clear();
            }
            ImGui::End();
            app.imgui().end();
        }
    }

} // namespace renderpipe
