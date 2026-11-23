#pragma once

// Calaca que toca la guitarra en la entrada del panteón.
// Usa las piezas del esqueleto con su propio ciclo, al ritmo de la canción (b = pulsos):
//   rasgueo (codo derecho):   c(t) = C0 + A * sin(2*pi * 2b)
//   acordes (brazo izquierdo): cambia de posición cada 3 pulsos con smoothstep
//   cabeza, pie y balanceo:    senos con la misma frecuencia del pulso
// Si el jugador se acerca, la calaca gira hacia él.

// Librerías
#include <cmath>

#include "PiezaColor.h"

// Piezas del esqueleto que ya se cargan en main.cpp
struct HuesosEsqueleto
{
	Model* cuerpo; Model* pecho; Model* craneo;
	Model* brazoDer; Model* antebrazoDer; Model* manoDer;
	Model* brazoIzq; Model* antebrazoIzq; Model* manoIzq;
	Model* femurDer; Model* piernaDer; Model* pieDer;
	Model* femurIzq; Model* piernaIzq; Model* pieIzq;
};

// Pose de un brazo: rotaciones del hombro (Z, Y, X) y del codo (X), en grados
struct PoseBrazo
{
	float hombroZ, hombroY, hombroX, codo;
};

class CalacaMusico
{
public:
	glm::vec3 posicion = glm::vec3(0.0f);
	float rotYBase = 0.0f;   // hacia dónde mira cuando el jugador está lejos

	// Cargar modelos
	void cargar(const HuesosEsqueleto& h)
	{
		huesos = h;
		guitarraCaja = cargarPieza("guitarra_caja");
		guitarraMastil = cargarPieza("guitarra_mastil");
		sombrero = cargarPieza("sombrero_charro");
		rotY = rotYBase;
	}

	// Actualizar
	void actualizar(float dt, const glm::vec3& posJugador)
	{
		tiempo += dt;

		// Girar hacia el jugador si está cerca (interpolación exponencial)
		glm::vec3 d = posJugador - posicion;
		float objetivo = rotYBase;
		if (d.x * d.x + d.z * d.z < DIST_ATENCION * DIST_ATENCION)
			objetivo = glm::degrees(std::atan2(d.x, d.z));
		rotY += difAngulo(objetivo, rotY) * (1.0f - std::exp(-2.5f * dt));
	}

	// Dibujar
	void dibujar(Shader& shader)
	{
		float b = PULSOS_POR_SEG * tiempo;
		float fase = 2.0f * PI_ANIM * b;

		// Movimientos rítmicos
		float balanceo = 2.5f * std::sin(fase / 3.0f);
		float cabeceo = 6.0f * std::sin(fase);
		float s = std::sin(fase);
		float golpePie = 14.0f * (s > 0.0f ? s : 0.0f);

		// Brazo derecho: pose fija sobre la boca de la guitarra + rasgueo con el codo
		PoseBrazo der = POSE_DERECHA;
		der.codo += 10.0f * std::sin(2.0f * fase);

		// Brazo izquierdo: cambia de acorde al inicio de cada compás
		int compas = (int)std::floor(b / 3.0f);
		float u = suavizar((b / 3.0f - compas) / 0.15f);   // la transición dura 15 % del compás
		const PoseBrazo& desde = ACORDES[(compas + 3) % 4];
		const PoseBrazo& hacia = ACORDES[compas % 4];
		PoseBrazo izq;
		izq.hombroZ = desde.hombroZ + (hacia.hombroZ - desde.hombroZ) * u;
		izq.hombroY = desde.hombroY;
		izq.hombroX = desde.hombroX;
		izq.codo = desde.codo + (hacia.codo - desde.codo) * u;

		// Raíz: posición, giro hacia el jugador y balanceo sobre los pies
		glm::mat4 base = glm::translate(glm::mat4(1.0f), posicion);
		base = glm::rotate(base, glm::radians(rotY), EJE_Y);
		base = glm::rotate(base, glm::radians(balanceo), EJE_Z);

		// Tronco y piernas
		dibujarHueso(shader, huesos.pecho, base);
		dibujarHueso(shader, huesos.cuerpo, base);
		dibujarHueso(shader, huesos.femurDer, base);
		dibujarHueso(shader, huesos.piernaDer, base);
		dibujarHueso(shader, huesos.pieDer, rotarEnPivote(base, TOBILLO_DER, -golpePie, EJE_X));
		dibujarHueso(shader, huesos.femurIzq, base);
		dibujarHueso(shader, huesos.piernaIzq, base);
		dibujarHueso(shader, huesos.pieIzq, base);

		// Cabeza y sombrero
		glm::mat4 mCabeza = rotarEnPivote(base, CUELLO, cabeceo, EJE_X);
		dibujarHueso(shader, huesos.craneo, mCabeza);
		glm::mat4 mSombrero = glm::translate(mCabeza, glm::vec3(0.0f, ALTURA_SOMBRERO, 0.0f));
		mSombrero = glm::rotate(mSombrero, glm::radians(-8.0f), EJE_Z);
		dibujarPieza(shader, sombrero, mSombrero, COLOR_SOMBRERO);

		// Brazos (hombro -> codo -> mano)
		dibujarBrazo(shader, base, der, HOMBRO_DER, CODO_DER, huesos.brazoDer, huesos.antebrazoDer, huesos.manoDer);
		dibujarBrazo(shader, base, izq, HOMBRO_IZQ, CODO_IZQ, huesos.brazoIzq, huesos.antebrazoIzq, huesos.manoIzq);

		// Guitarra frente al abdomen, con el mástil hacia la izquierda
		glm::mat4 mGuitarra = glm::translate(base, POS_GUITARRA);
		mGuitarra = glm::rotate(mGuitarra, glm::radians(20.0f), EJE_Z);   // mástil un poco hacia arriba
		mGuitarra = glm::rotate(mGuitarra, glm::radians(90.0f), EJE_X);   // tapa hacia el frente
		dibujarPieza(shader, guitarraCaja, mGuitarra, COLOR_GUITARRA);
		dibujarPieza(shader, guitarraMastil, mGuitarra, COLOR_MASTIL);
	}

	void caja(float& minX, float& maxX, float& minZ, float& maxZ) const
	{
		minX = posicion.x - 0.5f; maxX = posicion.x + 0.5f;
		minZ = posicion.z - 0.5f; maxZ = posicion.z + 0.5f;
	}

private:
	HuesosEsqueleto huesos = {};
	// Modelos
	Model* guitarraCaja = nullptr;
	Model* guitarraMastil = nullptr;
	Model* sombrero = nullptr;
	float tiempo = 0.0f;
	float rotY = 0.0f;

	const float PULSOS_POR_SEG = 1.5f;   // ~90 pulsos por minuto
	const float DIST_ATENCION = 7.0f;    // m

	// Articulaciones medidas en los .obj del esqueleto
	const glm::vec3 HOMBRO_DER = glm::vec3(-0.20f, 1.59f, 0.0f);
	const glm::vec3 CODO_DER = glm::vec3(-0.282f, 1.275f, -0.055f);
	const glm::vec3 HOMBRO_IZQ = glm::vec3(0.20f, 1.59f, 0.0f);
	const glm::vec3 CODO_IZQ = glm::vec3(0.282f, 1.275f, -0.055f);
	const glm::vec3 CUELLO = glm::vec3(0.0f, 1.74f, 0.0f);
	const glm::vec3 TOBILLO_DER = glm::vec3(-0.10f, 0.08f, -0.02f);
	const float ALTURA_SOMBRERO = 1.86f;   // la copa cubre la parte alta del cráneo
	const glm::vec3 POS_GUITARRA = glm::vec3(-0.05f, 1.22f, 0.24f);

	// Poses calculadas por cinemática inversa (búsqueda de ángulos) para que
	// la mano derecha quede sobre la boca y la izquierda sobre el mástil
	const PoseBrazo POSE_DERECHA = { -55.0f, 60.0f, -35.0f, -65.0f };
	const PoseBrazo ACORDES[4] = {
		{  2.0f, 20.0f, -10.0f, -84.0f },   // cerca del cuerpo
		{ 14.0f, 20.0f, -10.0f, -80.0f },   // a la mitad del mástil
		{ 27.0f, 20.0f, -10.0f, -72.0f },   // cerca de la cabeza
		{ 14.0f, 20.0f, -10.0f, -80.0f }
	};

	const glm::vec3 COLOR_GUITARRA = glm::vec3(0.55f, 0.32f, 0.15f);
	const glm::vec3 COLOR_MASTIL = glm::vec3(0.18f, 0.10f, 0.05f);
	const glm::vec3 COLOR_SOMBRERO = glm::vec3(0.14f, 0.11f, 0.10f);

	// Los huesos tienen textura: se dibujan sin color liso
	void dibujarHueso(Shader& shader, Model* hueso, const glm::mat4& m)
	{
		glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(m));
		glUniform1i(glGetUniformLocation(shader.Program, "usarColor"), 0);
		hueso->Draw(shader);
	}

	// Jerarquía del brazo: el hombro gira (Z, Y, X) y el codo hereda esa rotación
	void dibujarBrazo(Shader& shader, const glm::mat4& base, const PoseBrazo& p,
		const glm::vec3& hombro, const glm::vec3& codo, Model* brazo, Model* antebrazo, Model* mano)
	{
		glm::mat4 mBrazo = glm::translate(base, hombro);
		mBrazo = glm::rotate(mBrazo, glm::radians(p.hombroZ), EJE_Z);
		mBrazo = glm::rotate(mBrazo, glm::radians(p.hombroY), EJE_Y);
		mBrazo = glm::rotate(mBrazo, glm::radians(p.hombroX), EJE_X);
		mBrazo = glm::translate(mBrazo, -hombro);
		dibujarHueso(shader, brazo, mBrazo);

		glm::mat4 mAntebrazo = rotarEnPivote(mBrazo, codo, p.codo, EJE_X);
		dibujarHueso(shader, antebrazo, mAntebrazo);
		dibujarHueso(shader, mano, mAntebrazo);
	}
};
