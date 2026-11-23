#pragma once

// Pareja de catrines bailando un vals sobre una base giratoria.
// El movimiento son tres rotaciones anidadas:
//   1. la base gira:                    alfa(t) = wb * t
//   2. el centro de la pareja gira:     C(t) = Rc * (cos(wc t), sin(wc t))
//   3. la pareja gira sobre su centro:  P(t) = C(t) +/- r * (cos(wp t), sin(wp t))
// Rebote del vals: y = A * |sin(pi * b)|, más fuerte en el primer pulso del compás.
// Cada 6 pulsos la catrina da una vuelta completa con smoothstep y se le abre el vestido.

// Librerías
#include <cmath>

#include "PiezaColor.h"

class Catrines
{
public:
	glm::vec3 posicion = glm::vec3(0.0f);

	// Cargar modelos
	void cargar()
	{
		base = cargarPieza("catrines_base");
		cabeza = cargarPieza("catrin_cabeza");
		vestido = cargarPieza("catrina_vestido");
		manosCatrina = cargarPieza("catrina_manos");
		cara = cargarPieza("catrin_cara");
		camisa = cargarPieza("catrin_camisa");
		sombreroCatrina = cargarPieza("catrina_sombrero");
		flores = cargarPieza("catrina_flores");
		traje = cargarPieza("catrin_traje");
		sombreroCatrin = cargarPieza("catrin_sombrero");
		mono = cargarPieza("catrin_mono");
	}

	// Actualizar
	void actualizar(float dt) { tiempo += dt; }

	// Dibujar
	void dibujar(Shader& shader)
	{
		float t = tiempo;
		float b = PULSOS * t;

		// 1. Base que gira
		glm::mat4 mBase = glm::translate(glm::mat4(1.0f), posicion);
		mBase = glm::rotate(mBase, OMEGA_BASE * t, EJE_Y);
		dibujarPieza(shader, base, mBase, COLOR_BASE);

		// 2. Centro de la pareja sobre la base
		glm::vec2 c(RC * std::cos(OMEGA_C * t), RC * std::sin(OMEGA_C * t));

		// 3. Posición de cada figura alrededor del centro
		glm::vec2 dir(std::cos(OMEGA_P * t), std::sin(OMEGA_P * t));
		glm::vec2 p1 = c + R_PAREJA * dir;   // catrina
		glm::vec2 p2 = c - R_PAREJA * dir;   // catrín

		// Paso de vals: rebote con acento en el primer pulso del compás
		int pulso = (int)std::floor(b);
		float acento = (pulso % 3 == 0) ? 1.0f : 0.5f;
		float salto = ALTURA_PASO * std::fabs(std::sin(PI_ANIM * b)) * acento;

		// Vuelta de la catrina en los últimos 2 pulsos de cada 6
		float enCiclo = std::fmod(b, 6.0f);
		float giro = 0.0f, escalaVestido = 1.0f;
		if (enCiclo > 4.0f)
		{
			float u = (enCiclo - 4.0f) / 2.0f;
			giro = 360.0f * suavizar(u);
			escalaVestido = 1.0f + 0.25f * std::sin(PI_ANIM * u);
		}

		// Cada figura mira a su pareja
		float yaw1 = std::atan2(p2.x - p1.x, p2.y - p1.y);
		float yaw2 = std::atan2(p1.x - p2.x, p1.y - p2.y);

		// Catrina
		glm::mat4 m1 = glm::translate(mBase, glm::vec3(p1.x, ALTURA_BASE + salto, p1.y));
		m1 = glm::rotate(m1, yaw1 + glm::radians(giro), EJE_Y);
		glm::mat4 mVestido = glm::scale(m1, glm::vec3(escalaVestido, 1.0f, escalaVestido));
		dibujarPieza(shader, vestido, mVestido, COLOR_VESTIDO);
		dibujarPieza(shader, manosCatrina, m1, COLOR_HUESO);
		dibujarPieza(shader, cabeza, m1, COLOR_HUESO);
		dibujarPieza(shader, cara, m1, COLOR_CARA);
		dibujarPieza(shader, sombreroCatrina, m1, COLOR_SOMBRERO_CATRINA);
		dibujarPieza(shader, flores, m1, COLOR_CEMPASUCHIL);

		// Catrín (rebota medio pulso después, como si la guiara)
		float salto2 = ALTURA_PASO * std::fabs(std::sin(PI_ANIM * (b + 0.5f))) * 0.6f;
		glm::mat4 m2 = glm::translate(mBase, glm::vec3(p2.x, ALTURA_BASE + salto2, p2.y));
		m2 = glm::rotate(m2, yaw2, EJE_Y);
		dibujarPieza(shader, traje, m2, COLOR_TRAJE);
		dibujarPieza(shader, camisa, m2, COLOR_CAMISA);
		dibujarPieza(shader, cabeza, m2, COLOR_HUESO);
		dibujarPieza(shader, cara, m2, COLOR_CARA);
		dibujarPieza(shader, sombreroCatrin, m2, COLOR_TRAJE);
		dibujarPieza(shader, mono, m2, COLOR_MONO);
	}

	void caja(float& minX, float& maxX, float& minZ, float& maxZ) const
	{
		minX = posicion.x - 1.35f; maxX = posicion.x + 1.35f;
		minZ = posicion.z - 1.35f; maxZ = posicion.z + 1.35f;
	}

private:
	// Modelos
	Model* base = nullptr; Model* cabeza = nullptr;
	Model* vestido = nullptr; Model* sombreroCatrina = nullptr; Model* flores = nullptr;
	Model* traje = nullptr; Model* sombreroCatrin = nullptr; Model* mono = nullptr;
	Model* manosCatrina = nullptr; Model* cara = nullptr; Model* camisa = nullptr;
	float tiempo = 0.0f;

	const float PULSOS = 1.5f;          // pulsos por segundo (vals)
	const float OMEGA_BASE = 0.25f;     // rad/s
	const float OMEGA_C = 0.6f;         // rad/s
	const float OMEGA_P = 1.8f;         // rad/s
	const float RC = 0.45f;             // m
	const float R_PAREJA = 0.24f;       // m
	const float ALTURA_PASO = 0.05f;    // m
	const float ALTURA_BASE = 0.24f;    // m (arriba de la base)

	const glm::vec3 COLOR_BASE = glm::vec3(0.85f, 0.30f, 0.55f);
	const glm::vec3 COLOR_VESTIDO = glm::vec3(0.55f, 0.15f, 0.62f);
	const glm::vec3 COLOR_HUESO = glm::vec3(0.95f, 0.93f, 0.88f);
	const glm::vec3 COLOR_SOMBRERO_CATRINA = glm::vec3(0.18f, 0.10f, 0.20f);
	const glm::vec3 COLOR_CEMPASUCHIL = glm::vec3(1.0f, 0.55f, 0.05f);
	const glm::vec3 COLOR_TRAJE = glm::vec3(0.10f, 0.10f, 0.13f);
	const glm::vec3 COLOR_MONO = glm::vec3(0.80f, 0.10f, 0.10f);
	const glm::vec3 COLOR_CARA = glm::vec3(0.06f, 0.05f, 0.07f);
	const glm::vec3 COLOR_CAMISA = glm::vec3(0.97f, 0.97f, 0.95f);
};
