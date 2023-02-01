#include "transformations.h"

void translate(glm::mat4& matrix, float x, float y, float z){
    matrix = glm::translate(matrix, glm::vec3(x, y, z));
}

void rotate(glm::mat4& matrix, float degreeAngle){
    matrix = glm::rotate(matrix, glm::radians(degreeAngle), 
        glm::vec3(0.0f, 0.0f, 1.0f));
}


void identify(glm::mat4& matrix){
    matrix = glm::mat4(1.0f);
} 