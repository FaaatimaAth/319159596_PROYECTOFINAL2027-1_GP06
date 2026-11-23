#pragma once

// Xoloitzcuintle que guía al jugador por el sendero de la ofrenda al panteón.
// Camino: spline de Catmull-Rom por puntos del sendero
//   C(u) = 0.5 * [2P1 + (-P0 + P2)u + (2P0 - 5P1 + 4P2 - P3)u^2 + (-P0 + 3P1 - 3P2 + P3)u^3]
// Velocidad constante con una tabla de longitud de arco.
// Patas en diagonal: +/- A * sin(fase), y la fase avanza con la distancia recorrida.
// Si el jugador se aleja se detiene, voltea a verlo y mueve la cola más rápido.

// Librerías
#include <vector>
#include <cmath>

#include "PiezaColor.h"
#include "Sendero.h"

class Xolo
{
public:
	// Cargar modelos
	void cargar()
	{
		cuerpo = cargarPieza("xolo_cuerpo");
		cabeza = cargarPieza("xolo_cabeza");
		pata = cargarPieza("xolo_pata");
		colaM = cargarPieza("xolo_cola");
		collar = cargarPieza("xolo_collar");

		// Puntos de control: de la ofrenda, por el corredor, al pasillo del panteón
		puntos = {
			glm::vec3(-20.0f, 0.0f, 4.9f),
			glm::vec3(-13.0f, 0.0f, 5.3f),
			glm::vec3(-5.0f, 0.0f, 4.5f),
			glm::vec3(3.0f, 0.0f, 2.7f),
			glm::vec3(9.0f, 0.0f, 1.0f),
			glm::vec3(15.0f, 0.0f, 0.7f),
			glm::vec3(24.0f, 0.0f, 0.7f),
			glm::vec3(31.5f, 0.0f, 1.2f),
			glm::vec3(31.5f, 0.0f, 10.0f),
			glm::vec3(31.5f, 0.0f, 22.0f)
		};
		construirTabla();
		pos = puntoEnDistancia(0.0f);
		rotY = 90.0f;
	}

	// Actualizar
	void actualizar(float dt, const glm::vec3& posJugador)
	{
		if (dt > 0.1f) dt = 0.1f;
		tiempo += dt;

		// Estado: esperar si el jugador está lejos
		glm::vec3 d = posJugador - pos;
		float distJugador = std::sqrt(d.x * d.x + d.z * d.z);
		esperando = distJugador > DIST_ESPERA;

		// Velocidad con aceleración suave
		float vObjetivo = esperando ? 0.0f : V_MAX;
		vel += (vObjetivo - vel) * (1.0f - std::exp(-3.0f * dt));

		// Avanzar por el sendero (ida y vuelta)
		distancia += sentido * vel * dt;
		if (distancia >= largoTotal) { distancia = largoTotal; sentido = -1.0f; }
		if (distancia <= 0.0f) { distancia = 0.0f; sentido = 1.0f; }

		glm::vec3 nueva = puntoEnDistancia(distancia);
		glm::vec3 delante = puntoEnDistancia(glm::clamp(distancia + sentido * 0.3f, 0.0f, largoTotal));
		pos = nueva;

		// Hacia dónde mira: la tangente del camino (o hacia el jugador si espera)
		glm::vec3 dir = delante - nueva;
		float objetivo = rotY;
		if (glm::length(dir) > 1e-4f) objetivo = glm::degrees(std::atan2(dir.x, dir.z));
		rotY += difAngulo(objetivo, rotY) * (1.0f - std::exp(-6.0f * dt));

		// Cabeza: voltea hacia el jugador cuando lo espera (máx. 70 grados)
		float giroCab = 0.0f;
		if (esperando)
			giroCab = glm::clamp(difAngulo(glm::degrees(std::atan2(d.x, d.z)), rotY), -70.0f, 70.0f);
		giroCabeza += (giroCab - giroCabeza) * (1.0f - std::exp(-4.0f * dt));

		// Fase del paso según la distancia recorrida
		fasePaso += 2.0f * PI_ANIM * (vel * dt) / ZANCADA;
	}

	// Dibujar
	void dibujar(Shader& shader)
	{
		float intensidad = vel / V_MAX;                    // 0 parado, 1 trotando
		float sp = std::sin(fasePaso);
		float anguloPata = 28.0f * intensidad * sp;
		float rebote = 0.025f * intensidad * std::fabs(sp);
		float y = alturaPiso(pos.x, pos.z);

		glm::mat4 m = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, y + rebote, pos.z));
		m = glm::rotate(m, glm::radians(rotY), EJE_Y);
		dibujarPieza(shader, cuerpo, m, COLOR_PIEL);

		// Patas en diagonal
		const glm::vec3 caderas[4] = {
			glm::vec3(0.09f, 0.45f, 0.22f),    // delantera izquierda
			glm::vec3(-0.09f, 0.45f, 0.22f),   // delantera derecha
			glm::vec3(0.09f, 0.45f, -0.24f),   // trasera izquierda
			glm::vec3(-0.09f, 0.45f, -0.24f)   // trasera derecha
		};
		const float signo[4] = { 1.0f, -1.0f, -1.0f, 1.0f };
		for (int i = 0; i < 4; i++)
		{
			glm::mat4 mp = glm::translate(m, caderas[i]);
			mp = glm::rotate(mp, glm::radians(signo[i] * anguloPata), EJE_X);
			dibujarPieza(shader, pata, mp, COLOR_PIEL);
		}

		// Cabeza y collar
		glm::mat4 mc = glm::translate(m, glm::vec3(0.0f, 0.72f, 0.42f));
		mc = glm::rotate(mc, glm::radians(giroCabeza), EJE_Y);
		mc = glm::rotate(mc, glm::radians(4.0f * intensidad * std::sin(2.0f * fasePaso)), EJE_X);
		dibujarPieza(shader, cabeza, mc, COLOR_PIEL);

		glm::mat4 mCollar = glm::translate(m, glm::vec3(0.0f, 0.62f, 0.34f));
		mCollar = glm::rotate(mCollar, glm::radians(40.0f), EJE_X);
		dibujarPieza(shader, collar, mCollar, COLOR_CEMPASUCHIL);

		// Cola: más rápida y amplia cuando espera (está contento de verte)
		float ac = esperando ? 35.0f : 15.0f;
		float wc = esperando ? 14.0f : 6.0f;
		glm::mat4 mt = glm::translate(m, glm::vec3(0.0f, 0.55f, -0.34f));
		mt = glm::rotate(mt, glm::radians(ac * std::sin(wc * tiempo)), EJE_Y);
		dibujarPieza(shader, colaM, mt, COLOR_PIEL);
	}

private:
	// Modelos
	Model* cuerpo = nullptr; Model* cabeza = nullptr; Model* pata = nullptr;
	Model* colaM = nullptr; Model* collar = nullptr;

	std::vector<glm::vec3> puntos;
	std::vector<glm::vec3> muestras;    // puntos de la curva
	std::vector<float> largos;          // longitud de arco acumulada
	float largoTotal = 0.0f;

	glm::vec3 pos = glm::vec3(0.0f);
	float distancia = 0.0f, sentido = 1.0f, vel = 0.0f;
	float rotY = 0.0f, giroCabeza = 0.0f, fasePaso = 0.0f, tiempo = 0.0f;
	bool esperando = true;

	const float V_MAX = 1.6f;          // m/s
	const float ZANCADA = 0.8f;        // m por ciclo de paso
	const float DIST_ESPERA = 8.0f;    // m
	const glm::vec3 COLOR_PIEL = glm::vec3(0.20f, 0.18f, 0.22f);
	const glm::vec3 COLOR_CEMPASUCHIL = glm::vec3(1.0f, 0.55f, 0.05f);

	// Catmull-Rom entre P1 y P2
	static glm::vec3 catmullRom(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& p2, const glm::vec3& p3, float u)
	{
		float u2 = u * u, u3 = u2 * u;
		return 0.5f * (2.0f * p1 + (-p0 + p2) * u + (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * u2
			+ (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * u3);
	}

	// Tabla de longitud de arco
	void construirTabla()
	{
		const int porTramo = 40;
		int n = (int)puntos.size();
		muestras.clear(); largos.clear();
		for (int i = 0; i < n - 1; i++)
		{
			const glm::vec3& p0 = puntos[i > 0 ? i - 1 : 0];
			const glm::vec3& p1 = puntos[i];
			const glm::vec3& p2 = puntos[i + 1];
			const glm::vec3& p3 = puntos[i + 2 < n ? i + 2 : n - 1];
			for (int k = 0; k < porTramo; k++)
				muestras.push_back(catmullRom(p0, p1, p2, p3, (float)k / porTramo));
		}
		muestras.push_back(puntos[n - 1]);

		largoTotal = 0.0f;
		largos.push_back(0.0f);
		for (size_t i = 1; i < muestras.size(); i++)
		{
			largoTotal += glm::length(muestras[i] - muestras[i - 1]);
			largos.push_back(largoTotal);
		}
	}

	// Punto de la curva a cierta distancia del inicio
	glm::vec3 puntoEnDistancia(float d) const
	{
		int lo = 0, hi = (int)largos.size() - 1;
		while (hi - lo > 1)
		{
			int mid = (lo + hi) / 2;
			if (largos[mid] < d) lo = mid; else hi = mid;
		}
		float tramo = largos[hi] - largos[lo];
		float f = (tramo > 1e-6f) ? (d - largos[lo]) / tramo : 0.0f;
		return muestras[lo] + (muestras[hi] - muestras[lo]) * f;
	}
};
