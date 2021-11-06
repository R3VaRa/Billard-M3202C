#include <corecrt_math_defines.h>
#include <SFML/Graphics.hpp>
#include <windows.h>
#include <iostream>
#include <stdlib.h>

using namespace std;
using namespace sf;

bool Start = false;

Vector2f ResolutionFS = { (float)VideoMode::getDesktopMode().width, (float)VideoMode::getDesktopMode().height };
RenderWindow window(VideoMode(ResolutionFS.x, ResolutionFS.y), "Billard", Style::Fullscreen);

/*******************************************************************************
********************************* Structure ************************************
********************************************************************************/
struct Mur
{
	float X = 0;
	float Y = 0;
	float L = 0;
	float l = 0;
	RectangleShape Draw;
};
struct Trou
{
	float X = 0;
	float Y = 0;
	float D = 0;
	CircleShape Draw;
};
struct Terrain
{
	float X = 0;
	float Y = 0;
	Mur M[4];
	Trou Tr[4];
	RectangleShape Draw;
};
Terrain T;

struct Vitesse
{
	float X = 0;
	float Y = 0;
	float R = 0;
};
struct Balle
{
	float X = 0;
	float Y = 0;
	float D = 0;
	float A = 0;
	Vitesse V;
	CircleShape Draw;
};
Balle B;
struct Force
{
	float F = 0;
	const float Fmax = 1.0;
	bool moins = false;
	float X = 0;
	float Y = 0;
	float L = 0;
	float l = 0;
	Mur M[4];
	RectangleShape Draw;
};
struct Queue
{
	float X = 0;
	float Y = 0;
	float L = 0;
	float l = 0;
	Force F;
	RectangleShape Draw;
};
Queue Q;
/*******************************************************************************
********************************** Fonction ************************************
********************************************************************************/
#pragma region Init

/**********************************************************************
 *  Placement de la balle dans le terrain définition de son diametre  *
 **********************************************************************/
void initBalle()
{
	B.X = T.X / 4.0;
	B.Y = T.Y / 2.0;
	B.D = (T.X / 45.0);
	B.V.R = 0.0;
	CircleShape Bdraw(B.D / 2.0);
	B.Draw = Bdraw;
	B.Draw.setFillColor(Color::White);
	B.Draw.setPosition(B.Y, B.X);
}

/*************************************************************************
 *  Calcule de la taille du terrain En fonction de la taille de l'ecran  *
 *     initalisation de la balle en fonction de la taille du terrain     *
 *        placement des trous/murs et dimmentionement de ceux-ci         *
 *************************************************************************/
void initTerrain()
{
	if (ResolutionFS.y < (ResolutionFS.x/2))
	{
		T.X = (ResolutionFS.y * 2.0);
		T.Y = ResolutionFS.y;
	}
	else
	{
		T.X = ResolutionFS.x;
		T.Y = (ResolutionFS.x / 2.0);
	}
	T.Draw.setSize(Vector2f(T.X, T.Y));
	T.Draw.setFillColor(Color(30, 54, 40, 255));
	T.Draw.setPosition(Vector2f(0.0, 0.0));

	initBalle();

	for (int i = 0; i < 4; i++)
	{
		T.Tr[i].D = B.D * 1.8;
		switch (i)
		{
		case 0:
			T.Tr[i].X = 0.0; T.Tr[i].Y = 0.0;
			T.M[i].X = 0.0; T.M[i].Y = 0.0;
			T.M[i].L = 1; T.M[i].l = T.Y;
			break;
		case 1:
			T.Tr[i].X = 0.0; T.Tr[i].Y = T.Y - T.Tr[i].D;
			T.M[i].X = 0.0; T.M[i].Y = T.Y;
			T.M[i].L = T.X; T.M[i].l = 1;
			break;
		case 2:
			T.Tr[i].X = T.X - T.Tr[i].D; T.Tr[i].Y = 0.0;
			T.M[i].X = T.X; T.M[i].Y = T.Y;
			T.M[i].L = -1; T.M[i].l = -T.Y;
			break;
		case 3:
			T.Tr[i].X = T.X - T.Tr[i].D; T.Tr[i].Y = T.Y - T.Tr[i].D;
			T.M[i].X = T.X; T.M[i].Y = 0.0;
			T.M[i].L = -T.X; T.M[i].l = 1;
			break;
		}
		CircleShape tmpTrou(T.Tr[i].D / 2.0);
		T.Tr[i].Draw = tmpTrou;
		T.Tr[i].Draw.setFillColor(Color(10, 30, 10, 255));
		T.Tr[i].Draw.setPosition(T.Tr[i].X, T.Tr[i].Y);

		T.M[i].Draw.setSize(Vector2f(T.M[i].L, T.M[i].l));
		T.M[i].Draw.setFillColor(Color::Blue);
		T.M[i].Draw.setPosition(Vector2f(T.M[i].X, T.M[i].Y));
	}
}

/*******************************************************************************************
 *  Initalisation de la taille de la queue de billard en fonction de la taille du terrain  *
 *******************************************************************************************/
void initQueue()
{
	Q.L = 1;
	Q.l = T.X * 0.55;
	Q.Draw.setSize(Vector2f(Q.L, Q.l));
	Q.Draw.setFillColor(Color(222, 184, 135, 255));
}

/***********************************************************
 *  Initalisation du rectange de force pour l'utilisateur  *
 ***********************************************************/
void initForce()
{
	Q.F.L = Q.F.Fmax * 100;
	Q.F.l = Q.F.Fmax * 10;
	for (int i = 0; i < 4; i++)
	{
		Q.F.M[i].Draw.setFillColor(Color::Blue);
	}
	Q.F.M[0].Draw.setSize(Vector2f(Q.F.L, -1));
	Q.F.M[1].Draw.setSize(Vector2f(-1, -Q.F.l));
	Q.F.M[2].Draw.setSize(Vector2f(-Q.F.L, 1));
	Q.F.M[3].Draw.setSize(Vector2f(-1, Q.F.l));
}
#pragma endregion

/*******************************************************************************************************
 *  Calcul de l'angle de départ selon ou est positionner la souris par rapport au milieu  de la balle  *
 *******************************************************************************************************/
float angleQueueBillard()
{
	Vector2f S;
	S.x = Mouse::getPosition(window).x;
	S.y = Mouse::getPosition(window).y;
	if (S.x <= B.X + B.D / 2)
	{
		return (atan((S.y -( B.Y + B.D / 2.0)) / ((B.X + B.D / 2.0) - S.x)) * 180.0 / M_PI) * -1;
	}
	if (S.x > B.X + B.D / 2)
	{
		return 180.0 - (atan((S.y - (B.Y + B.D / 2.0)) / ((B.X + B.D / 2.0) - S.x)) * 180.0 / M_PI);
	}
}

/**********************************************************************************
 *  Calcule du vecteur vitesse selon la force et la ralentis rebond et air/tapis  *
 **********************************************************************************/
void calculeVitesse()
{
	B.V.X = cos(B.A * M_PI / 180.0) * B.V.R * Q.F.F;
	B.V.Y = sin(B.A * M_PI / 180.0) * B.V.R * Q.F.F;
}

/*************************************************************************
 *  Variation de la force entre 0 et 1 (correspondant a un pourcentage)  * 
 *************************************************************************/
void forceQueue()
{
	if (!Q.F.moins)
	{
		Q.F.F = Q.F.F + 0.001;
		if (Q.F.F >= Q.F.Fmax)
			Q.F.moins = true;
	}
	else if (Q.F.moins)
	{
		Q.F.F = Q.F.F - 0.001;
		if (Q.F.F <= 0.05)
			Q.F.moins = false;
	}
}

/*************************************************************
 *  Lancement de la balle remise a 1 le facteur de ralentie  *
 *************************************************************/
void Lancer()
{
	B.V.R = 1.0;
	calculeVitesse();
}

/***********************************************************************
 *  Reduction de la Vitesse en réduisant Vitesse.Ralentisement de 15%  *
 *   Re-calcule de la vitesse du au ralentisement la balle ralentie    *
 ***********************************************************************/
void ralentissement()
{
	B.V.R = B.V.R * 0.85;
}

/********************************************************************************************** 
 *  Teste si les coordonnee de la balle rentre en colision avec les coordonnee d'un des murs  *
 *     Si Oui l'angle de la balle change pourqu'elle continue son chemin dans le terrein      *
 *                          et elle ralentie du a l'echage d'energie                          *
 **********************************************************************************************/
void rebondMur()
{
	/* Teste du mur Droit et Gauche */
	if ((B.Y <= 0 && B.V.Y < 0) || (B.Y >= (T.Y - B.D) && B.V.Y > 0))
	{
		B.A = B.A * -1.0;
		ralentissement();
	}
	/* Teste du mur Haut et Bas */
	if ((B.X <= 0 && B.V.X < 0) || (B.X >= (T.X - B.D) && B.V.X > 0))
	{
		B.A = 180.0 - B.A;
		ralentissement();
	}
}

/*************************************************************************
 *  Reduction de la Vitesse en réduisant Vitesse.Ralentisement de 0.05%  *
 *                        Facteur air/tapis                              *
 *************************************************************************/
void facteurAirTapis()
{
	B.V.R = B.V.R * 0.9995;
}

/************************************************************************************************
 *    Le deplacement se fait avec le vecteur vitesse en pisitif sur la position de la balle     *
 *         Aplication de la reduction air/tapis verification s'il doit avoir un rebond          *
 *  Recalcule de la vitesse (car reduction de la vitesse du au facteur air/tapis et au rebond)  *
 ************************************************************************************************/
void deplacement()
{
	B.X = B.X + B.V.X;
	B.Y = B.Y + B.V.Y;
	B.Draw.setPosition(B.X, B.Y);
	facteurAirTapis();
	rebondMur();
	calculeVitesse();
}

/**************************************************************
 *  Affichage du terrain, des murs, des trous et de la balle  *
 **************************************************************/
void mursTrousDraw()
{
	window.draw(T.Draw);
	for (int i = 0; i < 4; i++)
	{
		window.draw(T.M[i].Draw);
		window.draw(T.Tr[i].Draw);
	}
	window.draw(B.Draw);
}

/**************************************************
 *  Affichage de la force dans un rectangle bleu  *
 **************************************************/
void forceDraw()
{
	for (int i = 0; i < 4; i++)
	{
		window.draw(Q.F.M[i].Draw);
	}
	window.draw(Q.F.Draw);
}

/********************************************************
 *  Actualisation de la position du rectangle de force  *
 ********************************************************/
void actualisationForceDraw()
{
	Q.F.X = Mouse::getPosition().x + 15;
	Q.F.Y = Mouse::getPosition().y;
	Q.F.M[0].X = Q.F.X;
	Q.F.M[0].Y = Q.F.Y;
	Q.F.M[1].X = Q.F.X;
	Q.F.M[1].Y = Q.F.Y + Q.F.l;
	Q.F.M[2].X = Q.F.X + Q.F.L;
	Q.F.M[2].Y = Q.F.Y + Q.F.l;
	Q.F.M[3].X = Q.F.X + Q.F.L;
	Q.F.M[3].Y = Q.F.Y;
	for (int i = 0; i < 4; i++)
	{
		Q.F.M[i].Draw.setPosition(Vector2f(Q.F.M[i].X, Q.F.M[i].Y));
	}
	Q.F.Draw.setSize(Vector2f(Q.F.F * 100, Q.F.l));
	Q.F.Draw.setPosition(Vector2f(Q.F.X, Q.F.Y));
	Q.F.Draw.setFillColor(Color(255*Q.F.F, 255 * (1 - Q.F.F), 0, 255));
}

/*******
 *  T  *
 *******/
void SCR_Init();
void dansLeTrou()
{
	if ((B.X + B.D / 2.0) <= T.Tr[0].X + T.Tr[0].D && B.Y + B.D / 2.0 <= T.Tr[0].Y + T.Tr[0].D)
	{
		Start = false;
		SCR_Init();
	}
	if (B.X + B.D / 2.0 <= T.Tr[1].X + T.Tr[1].D && B.Y + B.D / 2.0 >= T.Tr[1].Y)
	{
		Start = false;
		SCR_Init();
	}
	if (B.X + B.D / 2.0 >= T.Tr[2].X && B.Y + B.D / 2.0 <= T.Tr[2].Y + T.Tr[2].D)
	{
		Start = false;
		SCR_Init();
	}
	if (B.X + B.D / 2.0 >= T.Tr[3].X && B.Y + B.D / 2.0 >= T.Tr[3].Y)
	{
		Start = false;
		SCR_Init();
	}
}

/*******************************************************************************
*********************************** SCRIPT *************************************
********************************************************************************/
/*******
 *  D  *
 *******/
void SCR_draw()
{
	window.clear();
	window.draw(T.Draw);
	for (int i = 0; i < 4; i++) { window.draw(T.M[i].Draw); window.draw(T.Tr[i].Draw); }
	if (!Start)
	{
		window.draw(Q.Draw);
	}
	window.draw(B.Draw);
	if (!Start)
	{
		for (int i = 0; i < 4; i++) { window.draw(Q.F.M[i].Draw); }
		window.draw(Q.F.Draw);
	}
	window.display();
}

/*******
 *  C  *
 *******/
void SCR_calcul()
{
	if (!Start)
	{
		Q.Draw.setPosition(B.X + B.D / 2, B.Y + B.D / 2);
		Q.Draw.setRotation(angleQueueBillard() + 90);
		forceQueue();
		actualisationForceDraw();
	}
	if (Mouse::isButtonPressed(Mouse::Right) && !Start)
	{
		B.A = angleQueueBillard();
		Lancer();
		Start = true;
	}
	else if (Start)
	{
		deplacement();
		dansLeTrou();
		if (B.V.R * Q.F.F <= 0.02)
		{
			B.V.R = 0.0;
			Start = false;
		}
	}
}

/*******
 *  E  *
 *******/
void SCR_Event()
{
	// Process events
	Event event;
	while (window.pollEvent(event))
	{
		// Close window: exit
		if (event.type == Event::Closed) {
			window.close();
		}
		// Escape pressed: exit
		if (event.type == Event::KeyPressed &&
			event.key.code == Keyboard::Escape) {
			window.close();
		}
	}
}

/*******
 *  I  *
 *******/
void SCR_Init()
{
	initTerrain();
	initQueue();
	initForce();
}

/*******************************************************************************
************************************ MAIN **************************************
********************************************************************************/

/*******
 *  M  *
 *******/
int main(int, char const**)
{
	window.setFramerateLimit(400);

	window.clear();
	SCR_Init();
	window.display();

	while (window.isOpen())
	{
		SCR_Event();
		SCR_calcul();
		SCR_draw();
	};
	return EXIT_SUCCESS;
}