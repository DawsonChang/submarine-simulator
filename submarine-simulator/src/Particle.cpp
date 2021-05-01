#include "Particle.h"

#include <algorithm>
#include <random>
#include <limits>

#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"

void Particle::RenderContextParticle::initFromOBJ(obj::Model& model)
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
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    // Initialize with empty (NULL) buffer : it will be updated later, each frame.
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);

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

int Particle::RenderContextParticle::FindUnusedParticle() {

    for (int i = LastUsedParticle; i < MaxParticles; i++) {
        if (ParticlesContainer[i].life == 0.0) {
            LastUsedParticle = i;
            return i;
        }
    }

    for (int i = 0; i < LastUsedParticle; i++) {
        if (ParticlesContainer[i].life == 0.0) {
            LastUsedParticle = i;
            return i;
        }
    }

    LastUsedParticle = -1;
    return -1; // All particles are taken, make it -1 and stop generate new particles
}

void Particle::RenderContextParticle::SortParticles() {
    std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

void Particle::RenderContextParticle::UpdateGPUBuffer() {
    glBindVertexArray(vertexArray);

    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW); // Buffer orphaning, a common way to improve streaming perf. See above link for details.
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

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
        GL_UNSIGNED_BYTE, // type
        GL_TRUE, // normalized? *** YES, this means that the unsigned char[4] will be accessible with a vec4 (floats) in the shader ***
        0, // stride
        (void*)0 // array buffer offset
    );

    glVertexAttribDivisor(0, 1); // positions : one per quad (its center) -> 1
    glVertexAttribDivisor(1, 1); // color : one per quad -> 1
    
    glBindVertexArray(0);
}

void Particle::RenderContextParticle::UpdateParticles(float dt, glm::vec3 cameraPos) {
    
    // each frame add 1 new particle
    int newparticles = 1;
    if (countFrame % 20 == 0) {
        if (LastUsedParticle >= 0) { // if LastUsedParticle == -1, means reach the max number of particle. So do not make any particle live.
            for (int i = LastUsedParticle; i < LastUsedParticle + newparticles && i < MaxParticles; i++) {
                SingleParticle& p = ParticlesContainer[i];
                // make next particle live, the particle will exist 1 second
                p.life = 1.5f;
            }
        }
    }

    // this variable counts how many particles show on screen in this frame and update in VBO
    ParticlesCount = 0;

    // sort the array so the particle which is far from camera should be rendered first
    Particle::RenderContextParticle::SortParticles();
    
    for (int i = 0; i < MaxParticles; i++) {
        SingleParticle& p = ParticlesContainer[i];
        if (p.life > 0.0f) { // only care about the particle is still live
            p.life -= dt;
            if (p.life > 0.0f) {
                p.speed += glm::vec3(0.0f, 9.81f, 0.0f) * (float)dt;
                p.pos += p.speed * (float)dt;
                p.cameradistance = glm::length2(p.pos - cameraPos);

                // Fill the GPU buffer
                g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
                g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
                g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

                // Update the size (scale) here
                g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

                g_particule_color_data[4 * ParticlesCount + 0] = p.color.r;
                g_particule_color_data[4 * ParticlesCount + 1] = p.color.g;
                g_particule_color_data[4 * ParticlesCount + 2] = p.color.b;
                g_particule_color_data[4 * ParticlesCount + 3] = p.color.a;
            }
            else {
                // Particles that just died will be put at the end of the buffer in SortParticles();
                p.cameradistance = -1.0f;
            }
            ParticlesCount++;
        }
    }
    // if there is no particle live, "fin" will be true. In this way renderScene() will stop calling this function at next frame.
    if (ParticlesCount == 0) {
        fin = true;
        countFrame = 0;
    }
    else {
        countFrame++;
        if (countFrame == INT_MAX) {
            countFrame = 0;
        }
    }

    Particle::RenderContextParticle::UpdateGPUBuffer();
    Particle::RenderContextParticle::FindUnusedParticle();
}

void Particle::RenderContextParticle::ResetUsedParticle() {
    // determine how many particles should be initialized when press a key once
    int curParticles = 10;
    int i = 0;
    while (i < MaxParticles && curParticles > 0) {
        SingleParticle& p = ParticlesContainer[i];
        // make dead particles to live again - reset life, pos, speed, cameradistance to default
        if (p.life < 0.0) {
            p.life = 0;
            p.pos = glm::vec3(0.0, 0.0, 0.0) + (0.8 * glm::vec3(rand() % 10 + 1, 0, rand() % 10 + 1));
            p.speed = glm::vec3(0.0f, 20.0f, 0.0f) + (0.1 * glm::vec3(rand() % 20 + 1, rand() % 30 + 1, rand() % 20 + 1));
            p.cameradistance = 0.0f;
            curParticles--;
        }
        i++;
    }
    LastUsedParticle = 0;
    fin = false;
}
