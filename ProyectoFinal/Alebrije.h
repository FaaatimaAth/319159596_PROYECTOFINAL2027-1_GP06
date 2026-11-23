#pragma once

// Alebrije (jaguar con alas) que sobrevuela el panteón en forma de ocho.
// Trayectoria: lemniscata de Bernoulli
//   z(s) = a * cos(s) / (1 + sin^2(s)),  x(s) = a * sin(s) * cos(s) / (1 + sin^2(s))
//   y(s) = h + b * sin(2s)
// Velocidad constante: ds = v * dt / |P'(s)|.
// Orientación con la tangente: rumbo = atan2(Tx, Tz), cabeceo = -asin(Ty).
// Inclinación en las curvas: alabeo = atan(v^2 * k / g), con k la curvatura.
// Alas: A * sin(w t). Cola de 3 segmentos con onda: psi_i = Ac * sin(wc t - k i).

// Librerías
#include <cmath>

#include "PiezaColor.h"

class Alebrije
{
public:
	glm::vec3 centro = glm::vec3(32.3f, 7.5f, -5.0f);
	float a = 18.0f;          // m, mitad del largo del ocho
	float bAltura = 1.5f;     // m, cuánto sube y baja
	float velocidad = 4.0f;   // m/s
	bool pausado = false;

	// Cargar modelos
	void cargar()
	{
		cuerpo = cargarPieza("alebrije_cuerpo");
		patas = cargarPieza("alebrije_patas");
		motas = cargarPieza("alebrije_motas");
		cabeza = cargarPieza("alebrije_cabeza");
		ojos = cargarPieza("alebrije_ojos");
		alaDer = cargarPieza("alebrije_ala_der");
		alaIzq = cargarPieza("alebrije_ala_izq");
		plumasDer = cargarPieza("alebrije_plumas_der");
		plumasIzq = cargarPieza("alebrije_plumas_izq");
		cola = cargarPieza("alebrije_cola");
		colaPunta = cargarPieza("alebrije_cola_punta");
	}

	// Actualizar
	void actualizar(float dt)
	{
		if (pausado) return;
		if (dt > 0.1f) dt = 0.1f;
		tiempo += dt;

		// Avance con rapidez constante
		glm::vec3 d1 = derivada(s);
		float rapidezParam = glm::length(d1);
		if (rapidezParam > 1e-4f) s += velocidad * dt / rapidezParam;
		if (s > 2.0f * PI_ANIM) s -= 2.0f * PI_ANIM;

		// Alabeo según la curvatura (suavizado para que no tiemble)
		glm::vec3 d2 = (derivada(s + H) - derivada(s - H)) / (2.0f * H);
		float den = std::pow(d1.x * d1.x + d1.z * d1.z, 1.5f);
		float kappa = (den > 1e-6f) ? (d1.x * d2.z - d1.z * d2.x) / den : 0.0f;
		float objetivo = glm::degrees(std::atan(velocidad * velocidad * kappa / 9.81f));
		objetivo = glm::clamp(objetivo, -35.0f, 35.0f);
		alabeo += (objetivo - alabeo) * (1.0f - std::exp(-3.0f * dt));
	}

	// Dibujar
	void dibujar(Shader& shader)
	{
		glm::vec3 p = posicion(s);
		glm::vec3 tg = glm::normalize(derivada(s));
		float rumbo = std::atan2(tg.x, tg.z);
		float cabeceo = -std::asin(glm::clamp(tg.y, -1.0f, 1.0f));

		// Aleteo y rebote del cuerpo
		float w = 2.0f * PI_ANIM * FREC_ALETEO;
		float aleteo = 10.0f + 38.0f * std::sin(w * tiempo);
		float rebote = -0.12f * std::cos(w * tiempo);

		glm::mat4 m = glm::translate(glm::mat4(1.0f), p + glm::vec3(0.0f, rebote, 0.0f));
		m = glm::rotate(m, rumbo, EJE_Y);
		m = glm::rotate(m, cabeceo, EJE_X);
		m = glm::rotate(m, glm::radians(alabeo), EJE_Z);

		// Cuerpo
		dibujarPieza(shader, cuerpo, m, COLOR_CUERPO);
		dibujarPieza(shader, patas, m, COLOR_PATAS);
		dibujarPieza(shader, motas, m, COLOR_AMARILLO);

		// Cabeza: mira un poco hacia dentro de la curva y cabecea con el aleteo
		glm::mat4 mCabeza = glm::translate(m, PIVOTE_CABEZA);
		mCabeza = glm::rotate(mCabeza, glm::radians(-0.6f * alabeo), EJE_Y);
		mCabeza = glm::rotate(mCabeza, glm::radians(5.0f * std::sin(w * tiempo + 1.0f)), EJE_X);
		dibujarPieza(shader, cabeza, mCabeza, COLOR_NARANJA);
		dibujarPieza(shader, ojos, mCabeza, COLOR_TURQUESA);

		// Alas (giran sobre el eje Z en su raíz)
		glm::mat4 mAlaDer = glm::translate(m, glm::vec3(0.30f, 0.20f, 0.10f));
		mAlaDer = glm::rotate(mAlaDer, glm::radians(aleteo), EJE_Z);
		dibujarPieza(shader, alaDer, mAlaDer, COLOR_TURQUESA);
		dibujarPieza(shader, plumasDer, mAlaDer, COLOR_AMARILLO);

		glm::mat4 mAlaIzq = glm::translate(m, glm::vec3(-0.30f, 0.20f, 0.10f));
		mAlaIzq = glm::rotate(mAlaIzq, glm::radians(-aleteo), EJE_Z);
		dibujarPieza(shader, alaIzq, mAlaIzq, COLOR_TURQUESA);
		dibujarPieza(shader, plumasIzq, mAlaIzq, COLOR_AMARILLO);

		// Cola: 3 segmentos encadenados con onda viajera
		glm::mat4 mCola = glm::translate(m, PIVOTE_COLA);
		for (int i = 0; i < 3; i++)
		{
			float psi = 18.0f * std::sin(3.0f * tiempo - 0.9f * i);
			mCola = glm::rotate(mCola, glm::radians(psi), EJE_Y);
			mCola = glm::rotate(mCola, glm::radians(i == 0 ? -12.0f : 8.0f), EJE_X);
			dibujarPieza(shader, cola, mCola, (i % 2 == 0) ? COLOR_AMARILLO : COLOR_CUERPO);
			mCola = glm::translate(mCola, glm::vec3(0.0f, 0.0f, -0.45f));
		}
		dibujarPieza(shader, colaPunta, mCola, COLOR_TURQUESA);
	}

private:
	// Modelos
	Model* cuerpo = nullptr; Model* patas = nullptr; Model* motas = nullptr;
	Model* cabeza = nullptr; Model* ojos = nullptr;
	Model* alaDer = nullptr; Model* alaIzq = nullptr;
	Model* plumasDer = nullptr; Model* plumasIzq = nullptr;
	Model* cola = nullptr; Model* colaPunta = nullptr;

	float s = 0.0f;         // parámetro de la curva
	float tiempo = 0.0f;
	float alabeo = 0.0f;    // grados

	const float H = 0.01f;            // paso para las diferencias finitas
	const float FREC_ALETEO = 1.1f;   // aleteos por segundo
	const glm::vec3 PIVOTE_CABEZA = glm::vec3(0.0f, 0.25f, 0.75f);
	const glm::vec3 PIVOTE_COLA = glm::vec3(0.0f, 0.05f, -0.80f);

	const glm::vec3 COLOR_CUERPO = glm::vec3(0.90f, 0.10f, 0.55f);
	const glm::vec3 COLOR_PATAS = glm::vec3(0.20f, 0.78f, 0.30f);
	const glm::vec3 COLOR_AMARILLO = glm::vec3(1.00f, 0.85f, 0.10f);
	const glm::vec3 COLOR_NARANJA = glm::vec3(1.00f, 0.50f, 0.05f);
	const glm::vec3 COLOR_TURQUESA = glm::vec3(0.05f, 0.72f, 0.82f);

	// Lemniscata de Bernoulli (en el mundo)
	glm::vec3 posicion(float t) const
	{
		float sn = std::sin(t), cs = std::cos(t);
		float den = 1.0f + sn * sn;
		float u = a * cs / den;        // a lo largo de Z
		float wv = a * sn * cs / den;  // a lo largo de X
		return centro + glm::vec3(wv, bAltura * std::sin(2.0f * t), u);
	}

	// Derivada por diferencias centrales
	glm::vec3 derivada(float t) const
	{
		return (posicion(t + H) - posicion(t - H)) / (2.0f * H);
	}
};
