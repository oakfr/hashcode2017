// exemple d'utilisation du visualiseur
// necessite libglfw-dev

#include <iostream>
#include "Visu.h"

using namespace std;

#define N 10

double X[N];
double Y[N];

// callback de rendu
void render()
{
    for (int n = 0; n < N; ++n)
    {
        glColor3f(1., 0., 0.); // couleur en RGB

        glBegin(GL_QUADS); // debut du dessin d'un carre
        glVertex2f(X[n]-.02, Y[n]-.02); // les 4 sommets (par default les coordonnees vont de 0. a 1.)
        glVertex2f(X[n]+.02, Y[n]-.02);
        glVertex2f(X[n]+.02, Y[n]+.02);
        glVertex2f(X[n]-.02, Y[n]+.02);
        glEnd(); // fin du carre
        // pour d'autres formes voir http://3dgep.com/wp-content/uploads/2011/02/OpenGL-Primitives.png
    }
}

int main(int argc, char * argv[])
{
    Visu::init(argc, argv, render); // initialisation du visualiseur

    for (int k = 0; k < 60; ++k)
    {
        cout << k << endl;

        // modifie des donnes au hasard
        int n = rand() % N;
        X[n] = rand() / (double) RAND_MAX;
        Y[n] = rand() / (double) RAND_MAX;

        // affichage d'une frame
        Visu::display();
    }
}
