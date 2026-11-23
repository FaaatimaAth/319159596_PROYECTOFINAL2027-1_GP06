# Proyecto Final CGeIHC — Teoría Grupo 06 — 319159596
Recorrido 3D de Día de Muertos "La Alumbrada" (C++, OpenGL 3.3, Visual Studio 2022).

## Estructura del repositorio
- `Ejecutable/` — `ProyectoFinal.exe` (versión Release) con sus DLL.
- `Manuales/` — Manual técnico y manual de usuario (español e inglés).
- `ProyectoFinal/` — Código fuente, shaders, modelos, texturas y audio.
- `External Libraries/` — GLFW, GLEW, GLM, Assimp y SOIL2.
- `ProyectoFinal.sln` — Solución de Visual Studio 2022.

## Ejecutable
Abre `Ejecutable/ProyectoFinal.exe` con doble clic.
El programa carga los modelos, shaders e imágenes desde la carpeta `ProyectoFinal/`,
así que el repositorio debe clonarse completo.
Requiere Windows 10/11 y Microsoft Visual C++ Redistributable (x86).

## Controles
| Tecla | Acción |
|---|---|
| W A S D / flechas | Caminar |
| Mouse | Mover la cámara |
| Espacio | Saltar |
| F | Encender / apagar la linterna |
| Q / E | Abrir las fichas culturales |
| O | Abrir / cerrar el portón del panteón |
| C | Tocar la campana del panteón |
| M | Pausar / reanudar las mariposas |
| ESC | Salir |

## Animaciones
1. Esqueleto que camina y salta (jerárquica).
2. Mariposas monarca (keyframes).
3. Campana del panteón (péndulo amortiguado con choque del badajo).
4. Calaca músico tocando la guitarra (jerárquica, sincronizada al pulso).
5. Pareja de catrines bailando vals (rotaciones anidadas).
6. Alebrije que vuela en forma de ocho (lemniscata de Bernoulli).
7. Xoloitzcuintle que guía por el sendero (spline de Catmull-Rom).

## Cómo compilarlo en cualquier computadora
1. Instala **Git para Windows** (ya trae Git LFS). Los modelos (.obj) y texturas (.png/.jpg) están en **Git LFS**.
2. Clona el repositorio (NO uses "Download ZIP", porque los modelos se bajan vacíos):
   ```
   git lfs install
   git clone https://github.com/FaaatimaAth/319159596_PROYECTOFINAL2027-1_GP06.git
   ```
   Si ya lo habías clonado sin LFS: `git lfs pull`
3. Abre `ProyectoFinal.sln` en Visual Studio 2022.
4. Elige la configuración **Release | x86** (las librerías son de 32 bits) y presiona **F5**.

Las librerías están en `External Libraries/` y se referencian con `$(SolutionDir)`.
Todas las rutas de modelos, shaders e imágenes son relativas a la carpeta `ProyectoFinal/`.
