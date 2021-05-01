#include "Boids.h"
#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <random>
#include <cstdlib>

void Boids::RenderContextBoids::initFromOBJ(obj::Model& model)
{
    unsigned int vertexDataBufferSize = sizeof(float) * model.vertex.size();
    unsigned int vertexNormalBufferSize = sizeof(float) * model.normal.size();
    unsigned int vertexTexBufferSize = sizeof(float) * model.texCoord.size();

    size = model.faces["default"].size();
    unsigned int vertexElementBufferSize = sizeof(unsigned short) * size;

    // VAO
    glGenVertexArrays(1, &vertexArray);
    glBindVertexArray(vertexArray);

    // First VBO for data from obj file
    glGenBuffers(1, &vertexIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertexIndexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, vertexElementBufferSize, &model.faces["default"][0], GL_STATIC_DRAW);

    // The VBO containing the positions(center) and sizes of the particles
    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, NumBoids * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, NumBoids * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

    // VBO for data in obj::Model
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);

    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glBufferData(GL_ARRAY_BUFFER, vertexDataBufferSize + vertexNormalBufferSize + vertexTexBufferSize, NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexDataBufferSize, &model.vertex[0]);
    glBufferSubData(GL_ARRAY_BUFFER, vertexDataBufferSize, vertexNormalBufferSize, &model.normal[0]);
    glBufferSubData(GL_ARRAY_BUFFER, vertexDataBufferSize + vertexNormalBufferSize, vertexTexBufferSize, &model.texCoord[0]);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)(0));
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 0, (void*)(vertexDataBufferSize));
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, (void*)(vertexNormalBufferSize + vertexDataBufferSize));

}

void Boids::RenderContextBoids::SortParticles() {
    std::sort(&boids[0], &boids[NumBoids]);
}

void Boids::RenderContextBoids::UpdateGPUBuffer() {
    glBindVertexArray(vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, NumBoids * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, NumBoids * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_rotation_data);

    // 2nd attribute buffer : positions of particles' centers
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glVertexAttribPointer(
        0, // attribute. No particular reason for 1, but must match the layout in the shader.
        4, // size : x + y + z + size => 4
        GL_FLOAT, // type
        GL_FALSE, // normalized?
        0, // stride
        (void*)0 // array buffer offset
    );

    // 3rd attribute buffer : particles' colors
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glVertexAttribPointer(
        1, // attribute. No particular reason for 1, but must match the layout in the shader.
        4, // size : r + g + b + a => 4
        GL_FLOAT, // type
        GL_FALSE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
        0, // stride
        (void*)0 // array buffer offset
    );

    glVertexAttribDivisor(0, 1); // positions : one per quad (its center) -> 1
    glVertexAttribDivisor(1, 1); // color : one per quad -> 1

    glBindVertexArray(0);
}

// Rule 1: Boids try to fly towards the centre of mass of neighbouring boids.
glm::vec3 Boids::RenderContextBoids::rule1(int curIndex) {
    glm::vec3 pcj;
    int mx = 0, my = 0, mz = 0;

    for (int i = 0; i < NumBoids; i++) {
        if (i != curIndex) {
            mx += boids[i].pos.x;
            my += boids[i].pos.y;
            mz += boids[i].pos.z;
        }
    }
    pcj = glm::vec3(mx, my, mz) / (NumBoids - 1);
    return (pcj - boids[curIndex].pos) / 100.0f;
}
// Rule 2: Boids try to keep a small distance away from other objects (including other boids).
glm::vec3 Boids::RenderContextBoids::rule2(int curIndex) {
    glm::vec3 c = glm::vec3(0,0,0);
    for (int i = 0; i < NumBoids; i++) {
        if (i != curIndex) {
            if (glm::distance(boids[i].pos, boids[curIndex].pos) < 5) {
                c -= boids[i].pos - boids[curIndex].pos;
            }
        }
    }
    return c;
}

// Rule 3: Boids try to match velocity with near boids.
glm::vec3 Boids::RenderContextBoids::rule3(int curIndex) {
    glm::vec3 pvj;
    int vx = 0, vy = 0, vz = 0;

    for (int i = 0; i < NumBoids; i++) {
        if (i != curIndex) {
            vx += boids[i].speed.x;
            vy += boids[i].speed.y;
            vz += boids[i].speed.z;
        }
    }
    pvj = glm::vec3(vx, vy, vz) / (NumBoids - 1);
    return (pvj - boids[curIndex].speed) / 8.0f;
}

// Rule 4: Bounding the position
glm::vec3 Boids::RenderContextBoids::rule4(int curIndex) {
    glm::vec3 v;

    if (boids[curIndex].pos.x < (iPosX - lenX) / 2) v.x = 10;
    else if (boids[curIndex].pos.x > (iPosX + lenX) / 2) v.x = -10;

    if (boids[curIndex].pos.y < iPosY - lenY) v.y = 10;
    else if (boids[curIndex].pos.y > iPosY + lenY) v.y = -10;

    if (boids[curIndex].pos.z < iPosZ - lenZ) v.z = 10;
    else if (boids[curIndex].pos.z > iPosZ + lenZ) v.z = -10;

    return v;
}

// Rule 5: Limiting the speed
void Boids::RenderContextBoids::rule5(int curIndex) {
    int vlim = 10;
    if (boids[curIndex].speed.x > vlim) boids[curIndex].speed.x = vlim;
    else if (boids[curIndex].speed.x < -vlim) boids[curIndex].speed.x = -vlim;

    if (boids[curIndex].speed.y > vlim) boids[curIndex].speed.y = vlim;
    else if (boids[curIndex].speed.y < -vlim) boids[curIndex].speed.y = -vlim;

    if (boids[curIndex].speed.z > vlim) boids[curIndex].speed.z = vlim;
    else if (boids[curIndex].speed.z < -vlim) boids[curIndex].speed.z = -vlim;
}

void Boids::RenderContextBoids::UpdateParticles(float dt, glm::vec3 cameraPos) {

    // this variable counts how many particles show on screen in this frame and update in VBO
    ParticlesCount = 0;

    // sort the array so the particle which is far from camera should be rendered first
    Boids::RenderContextBoids::SortParticles();

    for (int i = 0; i < NumBoids; i++) {
        SingleObject& p = boids[i];
        
        glm::vec3 v1 = rule1(i);
        glm::vec3 v2 = rule2(i);
        glm::vec3 v3 = rule3(i);
        glm::vec3 v4 = rule4(i);

        p.speed += (v1 + v2 + v3 + v4) * 0.5;
        rule5(i);
        p.pos += p.speed * (float) dt;
        p.cameradistance = glm::length2(p.pos - cameraPos);

        // rotate the direction of fish, thus they could move toward the direction of tail to head
        if (glm::distance(p.prePos, p.pos) > 2) {
            glm::vec3 to_vector_boid = glm::normalize(p.speed);
            float thetaBoid = glm::dot(from_vector_boid, to_vector_boid) / (glm::length(from_vector_boid) * glm::length(to_vector_boid));
            p.axisBoid = glm::normalize(glm::cross(from_vector_boid, to_vector_boid));
            p.angleBoid = glm::acos(thetaBoid);
            p.prePos = p.pos;
        }

        // Fill the GPU buffer
        g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
        g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
        g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;
        // Update the size (scale) here
        g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

        g_particule_rotation_data[4 * ParticlesCount + 0] = 0;
        g_particule_rotation_data[4 * ParticlesCount + 1] = p.axisBoid.y;
        g_particule_rotation_data[4 * ParticlesCount + 2] = 0;
        g_particule_rotation_data[4 * ParticlesCount + 3] = p.angleBoid;
        ParticlesCount++;
    }

    countFrame++;
    Boids::RenderContextBoids::UpdateGPUBuffer();
}