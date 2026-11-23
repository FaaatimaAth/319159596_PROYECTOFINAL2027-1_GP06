#pragma once

// Campana del campanario del panteón (tecla C).
// La campana y el badajo son dos péndulos amortiguados:
//   theta'' = -(g / L) * sin(theta) - 2 * gamma * theta'
// Para ángulos pequeños: theta(t) = theta0 * e^(-gamma t) * cos(w t), con w = sqrt(g / L).
// Se resuelve con Euler semi-implícito en subpasos. Cuando el badajo choca con la campana
// rebota (coeficiente de restitución) y suena con un volumen según la fuerza del golpe.

// Librerías
#include <windows.h>
#include <cmath>
#include <cstdio>

#include "PiezaColor.h"

class Campana
{
public:
	// Ubicación del campanario en la escena
	glm::vec3 posicion = glm::vec3(0.0f);
	float rotY = 0.0f;

	// Cargar modelos
	void cargar()
	{
		marco = cargarPieza("campanario_marco");
		techo = cargarPieza("campanario_techo");
		campana = cargarPieza("campana");
		badajo = cargarPieza("campana_badajo");

		// Dos canales de audio con el mismo sonido: se alternan para que una
		// campanada no corte la cola (el eco) de la anterior.
		mciSendStringA("open \"assets/audio/campana.wav\" type mpegvideo alias campanaA", NULL, 0, NULL);
		mciSendStringA("open \"assets/audio/campana.wav\" type mpegvideo alias campanaB", NULL, 0, NULL);
	}

	// Cerrar el audio
	void cerrarAudio()
	{
		mciSendStringA("close campanaA", NULL, 0, NULL);
		mciSendStringA("close campanaB", NULL, 0, NULL);
	}

	// Empujón (tecla C): suma velocidad angular en el sentido en que ya se mueve
	void tocar()
	{
		float sentido = (velCampana >= 0.0f) ? 1.0f : -1.0f;
		velCampana += sentido * IMPULSO;
	}

	// Actualizar
	void actualizar(float dt)
	{
		if (dt > 0.1f) dt = 0.1f;   // evita saltos si la ventana se congela
		tiempoDesdeGolpe += dt;

		// Subpasos para que la integración sea estable
		const int pasos = 8;
		float h = dt / pasos;
		for (int i = 0; i < pasos; i++)
		{
			// Campana: péndulo amortiguado
			float aCampana = -(G / L_CAMPANA) * std::sin(angCampana) - 2.0f * GAMMA_CAMPANA * velCampana;
			velCampana += aCampana * h;
			angCampana += velCampana * h;

			// Límite físico del yugo
			float maxAng = glm::radians(ANGULO_MAXIMO);
			if (angCampana > maxAng) { angCampana = maxAng; velCampana = 0.0f; }
			if (angCampana < -maxAng) { angCampana = -maxAng; velCampana = 0.0f; }

			// Badajo: péndulo con su propio largo (oscila a otra frecuencia)
			float aBadajo = -(G / L_BADAJO) * std::sin(angBadajo) - 2.0f * GAMMA_BADAJO * velBadajo;
			velBadajo += aBadajo * h;
			angBadajo += velBadajo * h;

			// Choque contra la pared de la campana
			float rel = angBadajo - angCampana;
			float lim = glm::radians(LIMITE_BADAJO);
			if (rel > lim || rel < -lim)
			{
				float lado = (rel > 0.0f) ? 1.0f : -1.0f;
				float velRel = velBadajo - velCampana;
				angBadajo = angCampana + lado * lim;
				velBadajo = velCampana - RESTITUCION * velRel;   // rebote

				// Sonido si el golpe es fuerte
				if (std::fabs(velRel) > VEL_MIN_SONIDO && tiempoDesdeGolpe > 0.25f)
				{
					sonarGolpe(std::fabs(velRel));
					tiempoDesdeGolpe = 0.0f;
				}
			}
		}
	}

	// Dibujar
	void dibujar(Shader& shader)
	{
		glm::mat4 base = glm::translate(glm::mat4(1.0f), posicion);
		base = glm::rotate(base, glm::radians(rotY), EJE_Y);

		// Marco y techo (fijos)
		dibujarPieza(shader, marco, base, COLOR_MADERA);
		dibujarPieza(shader, techo, base, COLOR_TEJA);

		// Campana: gira sobre el eje del travesaño
		glm::mat4 mCampana = glm::translate(base, glm::vec3(0.0f, ALTURA_PIVOTE, 0.0f));
		mCampana = glm::rotate(mCampana, angCampana, EJE_X);
		dibujarPieza(shader, campana, mCampana, COLOR_BRONCE);

		// Badajo: hijo de la campana, gira con el ángulo relativo
		glm::mat4 mBadajo = glm::translate(mCampana, glm::vec3(0.0f, -0.10f, 0.0f));
		mBadajo = glm::rotate(mBadajo, angBadajo - angCampana, EJE_X);
		dibujarPieza(shader, badajo, mBadajo, COLOR_HIERRO_BADAJO);
	}

	// Caja de colisión del campanario (en coordenadas del mundo, sin rotación)
	void caja(float& minX, float& maxX, float& minZ, float& maxZ) const
	{
		minX = posicion.x - 1.4f; maxX = posicion.x + 1.4f;
		minZ = posicion.z - 0.45f; maxZ = posicion.z + 0.45f;
	}

private:
	// Modelos
	Model* marco = nullptr;
	Model* techo = nullptr;
	Model* campana = nullptr;
	Model* badajo = nullptr;

	// Estado (radianes y radianes/s)
	float angCampana = 0.0f, velCampana = 0.0f;
	float angBadajo = 0.0f, velBadajo = 0.0f;
	float tiempoDesdeGolpe = 1.0f;
	bool canalA = true;

	// Reproduce una campanada; el volumen depende de qué tan fuerte pegó el badajo
	void sonarGolpe(float fuerza)
	{
		float f = glm::clamp((fuerza - VEL_MIN_SONIDO) / 2.5f, 0.0f, 1.0f);
		int volumen = (int)(450.0f + 550.0f * f);   // MCI: 0..1000
		const char* canal = canalA ? "campanaA" : "campanaB";
		canalA = !canalA;
		char cmd[96];
		snprintf(cmd, sizeof(cmd), "setaudio %s volume to %d", canal, volumen);
		mciSendStringA(cmd, NULL, 0, NULL);
		snprintf(cmd, sizeof(cmd), "play %s from 0", canal);
		mciSendStringA(cmd, NULL, 0, NULL);
	}

	// Parámetros físicos
	const float G = 9.81f;
	const float L_CAMPANA = 1.20f;      // m  -> periodo ~2.2 s
	const float L_BADAJO = 1.60f;       // m  -> periodo ~2.5 s (por eso se desfasan y chocan)
	const float GAMMA_CAMPANA = 0.18f;  // 1/s
	const float GAMMA_BADAJO = 0.10f;   // 1/s
	const float IMPULSO = 2.3f;         // rad/s que agrega cada empujón
	const float ANGULO_MAXIMO = 75.0f;  // grados
	const float LIMITE_BADAJO = 30.0f;  // grados de separación antes de tocar la pared
	const float RESTITUCION = 0.45f;
	const float VEL_MIN_SONIDO = 0.4f;  // rad/s
	const float ALTURA_PIVOTE = 3.15f;  // m (bajo el travesaño)

	// Colores
	const glm::vec3 COLOR_MADERA = glm::vec3(0.36f, 0.22f, 0.12f);
	const glm::vec3 COLOR_TEJA = glm::vec3(0.55f, 0.18f, 0.10f);
	const glm::vec3 COLOR_BRONCE = glm::vec3(0.78f, 0.56f, 0.24f);
	const glm::vec3 COLOR_HIERRO_BADAJO = glm::vec3(0.15f, 0.14f, 0.13f);
};
