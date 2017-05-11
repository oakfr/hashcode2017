#ifndef VISU_H
#define VISU_H

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glfw.h>

namespace Visu
{

// argc/argv permet de passer des options au visualiseur :
//    -v          active le visualiseur
//    -d [DELAY]  delai entre deux frames, reel en seconde
//    -p          mettre en pause au lancement
//    -f [FRAME]  aller directement a la frame numero [FRAME]
//    -h          affiche l'aide
// renderFunction : callback de rendu
// xmin, xmax, ymin, ymax coordonnees min et max des points
void init(int argc, char * argv[],
          void (*renderFunction)(),
          double xmin = 0., double xmax = 1., double ymin = 0., double ymax = 1.);

// Affichage d'une frame
// Touches :
//    [escape] ou [q]    stoppe le process
//    [right]            frame suivante
//    [space]            (des)active la pause
//    [f]                affiche le numero de la frame
void display();

};

#endif
