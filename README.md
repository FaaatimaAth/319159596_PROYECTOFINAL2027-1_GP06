# Proyecto Final CGeIHC — Teoría Grupo 06 — 319159596
Recorrido 3D de Día de Muertos "La Alumbrada" (C++, OpenGL 3.3, Visual Studio 2022).

## Ejecutable
El ejecutable (versión Release) está en `ProyectoFinal/ProyectoFinal.exe`.
Se abre con doble clic. Debe quedarse en esa carpeta, porque usa las carpetas
`assets`, `Shader` e `Imagenes` y las DLL que están junto a él.
Requiere Windows 10/11 y Microsoft Visual C++ Redistributable (x86).

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