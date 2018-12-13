## Démo en vidéo

## Quelques rendus

## Lancer de rayons stochastique

> **Cliquez sur **:book:** pour accéder au code source associé à la fonctionnalitée.**

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/visualobject.cpp#L131) Utilisation de l'algorithme [Möller–Trumbore](https://en.wikipedia.org/wiki/M%C3%B6ller%E2%80%93Trumbore_intersection_algorithm) pour le test d'intersection rayon-triangle. Permet de facilement interpoler les normales pour faire du **Smooth Shading**. 

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/visualobject.cpp#L61) Les normales non interpolées sont calculés et stockés avant le rendu.

[:book:](https://github.com/Yt-trium/Couscous-raytracer/blob/master/src/renderer/visualobject.cpp#L38) Les sources de lumières sont des maillages qui ont un matériel avec une émission.

## Fonctionnalités supplémentaires

- Rendu de différents buffers (*Normals*, *Direct Diffuse*, *Dirrect Specular*, *Photon Map*, ...) disponible dans *Debug > View*
- Personalisation de la scène depuis la GUI
	- Modification et ajout des matériaux
	- Modification et ajout des primitves
	- Modification et ajout des instances de fichier *OFF*
	- Modification de la caméra et des paramètres de rendu
- Plusieurs scènes par défaut disponibles dans *Presets*

## Références

- [Ray Tracing in One Weekend by Peter Shirley](https://github.com/petershirley/raytracinginoneweekend)
- [Ray Tracing The Next Week by Peter Shirley](https://github.com/petershirley/raytracingthenextweek)
- [Physically Based Rendering: From Theory To Implementation](https://github.com/mmp/pbrt-v3/)
