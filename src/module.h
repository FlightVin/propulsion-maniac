#ifndef _MODULE_H_
#define _MODULE_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "module.h"

#include <iostream>
#include <map>
#include <string>

float genRand(float x);

void genVertex(unsigned int* VBOAddr, unsigned int* VAOAddr, 
        float vertices[], unsigned long verticesSize);

void enableMatrices(Shader shader, glm::mat4 model, 
        glm::mat4 proj);

void renderShape(unsigned int VAO, unsigned int texture, 
        Shader shader, int numVertices,
        glm::mat4 model, glm::mat4 proj);

void genTexture(unsigned int* textureAddr, const char* imagePath);

#endif