#pragma once

// Funciones de apoyo para las animaciones nuevas (campana, músico, catrines, alebrije y xolo).
// Sus modelos no tienen textura: cada pieza se dibuja con un color liso usando
// usarColor y colorPlano de lighting.frag, así les afecta la misma iluminación de la escena.

// Librerías
#include <string>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Model.h"

// Constantes
const float PI_ANIM = 3.14159265f;
const std::string RUTA_ANIMACIONES = "assets/models/animaciones/";

// Ejes
const glm::vec3 EJE_X(1.0f, 0.0f, 0.0f);
const glm::vec3 EJE_Y(0.0f, 1.0f, 0.0f);
const glm::vec3 EJE_Z(0.0f, 0.0f, 1.0f);

// Cargar una pieza por nombre (ejemplo: "campana" carga assets/models/animaciones/campana.obj)
inline Model* cargarPieza(const std::string& nombre)
{
	std::string ruta = RUTA_ANIMACIONES + nombre + ".obj";
	return new Model(&ruta[0]);
}

// Dibujar una pieza con su matriz de modelo y un color
inline void dibujarPieza(Shader& shader, Model* pieza, const glm::mat4& modelo, const glm::vec3& color)
{
	glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(modelo));
	glUniform1i(glGetUniformLocation(shader.Program, "transparency"), 0);
	glUniform1i(glGetUniformLocation(shader.Program, "usarColor"), 1);
	glUniform3f(glGetUniformLocation(shader.Program, "colorPlano"), color.x, color.y, color.z);
	pieza->Draw(shader);
	glUniform1i(glGetUniformLocation(shader.Program, "usarColor"), 0);
}

// Rotar una pieza sobre su pivote: T(pivote) * R * T(-pivote)
inline glm::mat4 rotarEnPivote(const glm::mat4& m, const glm::vec3& pivote, float grados, const glm::vec3& eje)
{
	glm::mat4 r = glm::translate(m, pivote);
	r = glm::rotate(r, glm::radians(grados), eje);
	return glm::translate(r, -pivote);
}

// Suavizado smoothstep: s(u) = 3u^2 - 2u^3, con u entre 0 y 1
inline float suavizar(float u)
{
	u = glm::clamp(u, 0.0f, 1.0f);
	return u * u * (3.0f - 2.0f * u);
}

// Diferencia entre dos ángulos en grados, en el rango [-180, 180]
inline float difAngulo(float destino, float actual)
{
	float d = destino - actual;
	while (d > 180.0f) d -= 360.0f;
	while (d < -180.0f) d += 360.0f;
	return d;
}
