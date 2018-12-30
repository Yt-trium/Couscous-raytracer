TODO: Expliquer ou est l'executable (seulement dans le rendu finale) et exporter en PDF
TODO: Ajouter le prof en viewer sur le git pour qu'il puisse accéder aux liens

## Démo en vidéo

TODO: Inserer videos

## Quelques rendus

Cornell Box
![Cornell Box](results/cornell_box_1920_1080_512_12_12_25000_36271.png)

Cornell Box Metal
![Cornell Box](results/cornell_box_metal_1920_1080_512_12_12_25000_13350.png)

![Sphere](results/sphere_1920_1080_1024_12_12_50000_1903.png)

sphere_1920_1080_1024_12_12_50000_1903.png

## Lancer de rayons stochastique

> **Cliquez sur **:book:** pour accéder au code source associé à la fonctionnalitée.**

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/visualobject.cpp#L131) Utilisation de l'algorithme [Möller–Trumbore](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm) pour le test d'intersection rayon-triangle. Permet de facilement interpoler les normales pour faire du **Smooth Shading**. 

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/visualobject.cpp#L61) Les normales non interpolées sont calculés et stockés avant le rendu.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/visualobject.cpp#L38) Les sources de lumières sont des maillages qui ont un matériel avec une émission.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/render.cpp#L420) Utilisation du modèle Phong pour l'éclairage direct.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/gridaccelerator.cpp#L136) Accélération des tests d'intersection avec une grille de voxels. Parcourt de la grille par *Digital Differential Analyser*.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/samplegenerator.cpp#L23) *Anti-Aliasing* par *Jittered sampling*.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/render.cpp#L403) Réflexions avec coéfficient de lissage réglable.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/photonMapping.cpp#L127) Lancement de photons et *scattering*.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/photonMapping.cpp#L222) Stockage des photons dans un kd-tree. Utilisation de [*nanoflann*](https://github.com/jlblancoc/nanoflann).

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/render.cpp#L470) Utilisation des photons pour l'éclairage indirect.

## Fonctionnalités supplémentaires

- Rendu de différents buffers (*Normals*, *Direct Diffuse*, *Dirrect Specular*, *Photon Map*, ...) disponible dans *Debug > View*
- Personalisation de la scène depuis la GUI
	- Modification et ajout de matériaux
	- Modification et ajout de primitves
	- Modification et ajout d'instances de fichier *.OFF*
	- Modification de la caméra et des paramètres de rendu
- Plusieurs scènes par défaut disponibles dans *Presets*

## Références

- [Ray Tracing in One Weekend by Peter Shirley](https://github.com/petershirley/raytracinginoneweekend)
- [Ray Tracing The Next Week by Peter Shirley](https://github.com/petershirley/raytracingthenextweek)
- [Physically Based Rendering: From Theory To Implementation](https://github.com/mmp/pbrt-v3/)
- [Photon Mapping by Zack Waters ](https://web.cs.wpi.edu/~emmanuel/courses/cs563/write_ups/zackw/photon_mapping/PhotonMapping.html)
