///////////////////////////////////////////////////////////////////////////////
///  @file main.cpp
///  @brief   le main du programme ProjetNuanceur pour le cours INF8702 de Polytechnique
///  @author  Frédéric Plourde (2007)
///  @author  Félix Gingras Harvey (2016)
///  @date    2007 / 2016
///  @version 2.0
///
///////////////////////////////////////////////////////////////////////////////

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_inverse.hpp>
#include <gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/matrix_cross_product.hpp>
#include <gtx/transform.hpp>

#include "Cst.h"
#include "Laser.h"
#include "GrilleQuads.h"
#include "Materiau.h"
#include "Modele3DOBJ.h"
#include "NuanceurProg.h"
#include "Skybox.h"
#include "Texture2D.h"
#include "Var.h"
#include "textfile.h"
#include "compute.h"

///////////////////////////////////////////////
// LES OBJETS                                //
///////////////////////////////////////////////

// les programmes de nuanceurs
static CNuanceurProg progNuanceurCarte("Nuanceurs/carteSommets.glsl", "Nuanceurs/carteFragments.glsl", false);
static CNuanceurProg progNuanceurSkybox("Nuanceurs/skyBoxSommets.glsl", "Nuanceurs/skyBoxFragments.glsl", false);
//static CNuanceurProg progNuanceurGazon("Nuanceurs/gazonSommets.glsl", "Nuanceurs/gazonFragments.glsl", false);
GLuint computeHandle;
// les différents matériaux utilisés dans le programme
static CMateriau front_mat_ambiant_model(0.1f, 0.1f, 0.1f, 1.0f, 0.9f, 0.8f, 1.0f, 1.0f, 0.5f, 0.5f, 0.5f, 1.0f, 0.0f,
                                         0.0f, 0.0f, 1.0f, 100.0f);
CMateriau back_mat_ambiant_model(0.1, 0.1, 0.1, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 5.0);
CMateriau nurbs_mat_ambiant_model(0.8, 0.8, 0.8, 1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0);

// Les objets 3D graphiques (à instancier plus tard parce que les textures demandent le contexte graphique)
static CGrilleQuads* cartePoly;
//static CGazon*       gazon;
// CGrilleQuads *gazon;
static CSkybox* skybox;

// Vecteurs caméra
glm::vec3 cam_position = glm::vec3(0, 0, -25);
glm::vec3 direction;
glm::vec3 cam_right;
glm::vec3 cam_up;

// Angle horizontale de la caméra: vers les Z
float horizontalAngle = 0.f;
// Angle vertical: vers l'horizon
float verticalAngle = 0.f;
// "Field of View" initial
float initialFoV = 45.0f;

float vitesseCamera = 15.0f; // 15 unités / seconde
float vitesseSouris = 0.05f;

double sourisX = 0;
double sourisY = 0;

// resolution of the diffraction figure : more means more precise but longer calculations
const int resX = 128;
const int resY = 128;
//Arrays corresponding to the textures of the diffraction figure and the mask
float* texDiffColor = new float[resX * resY * 4];
float* texMaskColor = new float[resX * resY * 4];
//Array defining what part of the mask lets light through
float* mask = new float[resX * resY];
// Previous time storage for laser animation and calculating fps
double previous_time = 0;
int nbFrames = 0;
// Laser for diffraction
CLaser laser0 ;
//key controls
bool rotating = false;
// display mask
bool maskOn = false;


///////////////////////////////////////////////
// PROTOTYPES DES FONCTIONS DU MAIN          //
///////////////////////////////////////////////
void initialisation(void);
void dessinerSkybox(void);
void dessinerScene(void);
void dessinerCarte(void);
//void dessinerGazon(void);
void attribuerValeursLumieres(GLuint progNuanceur);
void clavier(GLFWwindow* fenetre, int touche, int scancode, int action, int mods);
void mouvementSouris(GLFWwindow* window, double deltaT, glm::vec3& direction, glm::vec3& right, glm::vec3& up);
void redimensionnement(GLFWwindow* fenetre, int w, int h);
void rafraichirCamera(GLFWwindow* window, double deltaT);
void compilerNuanceurs();
// Diffraction functions
void calculDiffraction();
void laserIntersect(CLaser laser);
void initDiffraction();
void updateLaser();
void setWindowFPS(GLFWwindow* fenetre);

// Changing mask layout
void setMask1();
void setMask2();
void setMask3();
void setMask4();

// le main
int main(int argc, char* argv[])
{
    // start GL context and O/S window using the GLFW helper library
    if (!glfwInit())
    {
        fprintf(stderr, "ERREUR: impossible d'initialiser GLFW3\n");
        return 1;
    }

    GLFWwindow* fenetre = glfwCreateWindow(CVar::currentW, CVar::currentH, "INF8702 - Labo", NULL, NULL);
    if (!fenetre)
    {
        fprintf(stderr, "ERREUR: impossibe d'initialiser la fenêtre avec GLFW3\n");
        glfwTerminate();
        return 1;
    }
    glfwSetWindowPos(fenetre, 600, 100);

    // Rendre le contexte openGL courrant celui de la fenêtre
    glfwMakeContextCurrent(fenetre);

    // Combien d'updates d'écran on attend après l'appel à glfwSwapBuffers()
    // pour effectivement échanger les buffers et retourner
    glfwSwapInterval(1);

    // Définir la fonction clavier
    glfwSetKeyCallback(fenetre, clavier);

    // Reset mouse position for next frame
    glfwSetCursorPos(fenetre, CVar::currentW / 2, CVar::currentH / 2);

    // Définire le comportement du curseur
    glfwSetInputMode(fenetre, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    // Définir la fonction de redimensionnement
    glfwSetWindowSizeCallback(fenetre, redimensionnement);

    // vérification de la version 4.X d'openGL
    glewInit();
    if (glewIsSupported("GL_VERSION_4_5"))
        printf("Pret pour OpenGL 4.5\n\n");
    else
    {
        printf("\nOpenGL 4.5 n'est pas supporte! \n");
        exit(1);
    }

    // Specifier le context openGL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // recueillir des informations sur le système de rendu
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* version  = glGetString(GL_VERSION);
    printf("Materiel de rendu graphique: %s\n", renderer);
    printf("Plus récente vversion d'OpenGL supportee: %s\n\n", version);

    GLint max;
    glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max);
    printf("GL_MAX_TEXTURE_UNITS = %d\n", max);

    glGetIntegerv(GL_MAX_VARYING_FLOATS, &max);
    printf("GL_MAX_VARYING_FLOATS = %d\n\n", max);

    if (!glfwExtensionSupported("GL_EXT_framebuffer_object"))
    {
        printf("Objets 'Frame Buffer' NON supportes! :( ... Je quitte !\n");
        exit(1);
    }
    else
    {
        printf("Objets 'Frame Buffer' supportes :)\n\n");
    }

    // compiler et lier les nuanceurs
    compilerNuanceurs();


	//gestion du compute shader
	computeHandle = genComputeProg("Nuanceurs/computeshader.glsl");

    // initialisation de variables d'état openGL et création des listes
    initialisation();

    double dernierTemps = glfwGetTime();
    int    nbFrames     = 0;

    // boucle principale de gestion des evenements
    while (!glfwWindowShouldClose(fenetre))
    {
        glfwPollEvents();
		setWindowFPS(fenetre);
        // Temps ecoule en secondes depuis l'initialisation de GLFW
        double temps  = glfwGetTime();
        double deltaT = temps - CVar::temps;
        CVar::temps   = temps;

        nbFrames++;
        // Si ça fait une seconde que l'on a pas affiché les infos
        if (temps - dernierTemps >= 1.0)
        {
            if (CVar::showDebugInfo)
            {
                printf("%f ms/frame\n", 1000.0 / double(nbFrames));
                printf("Position: (%f,%f,%f)\n", cam_position.x, cam_position.y, cam_position.z);
            }
            nbFrames = 0;
            dernierTemps += 1.0;
        }

        // Rafraichir le point de vue selon les input clavier et souris
        rafraichirCamera(fenetre, deltaT);

        // Afficher nos modèlests
        dessinerScene();

        // put the stuff we've been drawing onto the display
        glfwSwapBuffers(fenetre);
    }
    // close GL context and any other GLFW resources
    glfwTerminate();

    // on doit faire le ménage... !
    //delete gazon;
    delete CVar::lumieres[ENUM_LUM::LumPonctuelle];
    delete CVar::lumieres[ENUM_LUM::LumDirectionnelle];
    delete CVar::lumieres[ENUM_LUM::LumSpot];
    delete cartePoly;
    delete skybox;
	delete texDiffColor;
	delete texMaskColor;
	delete mask;

    // le programme n'arrivera jamais jusqu'ici
    return EXIT_SUCCESS;
}

// initialisation d'openGL
void initialisation(void)
{
    ////////////////////////////////////////////////////
    // CONSTRUCTION DES LUMIÈRES
    ////////////////////////////////////////////////////

    // LUMIÈRE PONCTUELLE ORKENTÉE (enum : LumPonctuelle - 0)
    CVar::lumieres[ENUM_LUM::LumPonctuelle] =
        new CLumiere(0.1f, 0.1f, 0.1f, 0.4f, 0.4f, 0.9f, 0.7f, 0.7f, 0.7f, 0.0f, -3.0f, -10.0f, 1.0f, false);
    CVar::lumieres[ENUM_LUM::LumPonctuelle]->modifierConstAtt(0.4);
    CVar::lumieres[ENUM_LUM::LumPonctuelle]->modifierLinAtt(0.0);
    CVar::lumieres[ENUM_LUM::LumPonctuelle]->modifierQuadAtt(0.0);

    // LUMIÈRE SPOT (enum : LumSpot - 1)
    CVar::lumieres[ENUM_LUM::LumSpot] = new CLumiere(0.2f, 0.2f, 0.2f, 0.9f, 0.8f, 0.4f, 1.0f, 1.0f, 1.0f, 10.0f, 10.0f,
                                                     -10.0f, 1.0f, false, -0.5f, -1.0f, 1.0f, 0.f, 30.0);

    // LUMIÈRE DIRECTIONNELLE (enum : LumDirectionnelle - 2)
    CVar::lumieres[ENUM_LUM::LumDirectionnelle] =
        new CLumiere(0.1f, 0.1f, 0.1f, 0.8f, 0.8f, 0.8f, 0.4f, 0.4f, 0.4f, 5.0f, -5.0f, 5.0f, 0.0f, false);

    // les noms de fichier de la texture de la carte.
    std::vector<const char*> texturesCarte;
    texturesCarte.push_back("Textures/white.bmp");
    texturesCarte.push_back("Textures/white.bmp");

	initDiffraction();

	calculDiffraction();

    cartePoly = new CGrilleQuads(&texturesCarte, 20.f, 20.f, 25, 25, 1.0f, false, true);
 
    // construire le skybox avec les textures
    skybox = new CSkybox("Textures/uffizi_cross_LDR.bmp", CCst::grandeurSkybox);


    // fixer la couleur de fond
    glClearColor(0.0, 0.0, 0.5, 1.0);

    // activer les etats openGL
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    // fonction de profondeur
    glDepthFunc(GL_LEQUAL);
}

void dessinerCarte(void)
{
	updateLaser();
	laserIntersect(laser0);
	calculDiffraction();

    GLenum err = 0;
    progNuanceurCarte.activer();
    // Création d'une matrice-modèle.
    // Défini la translation/rotaion/grandeur du modèle.
    float     scale = cartePoly->obtenirEchelle();
    glm::vec3 s(1, 1, 1);
    glm::mat4 scalingMatrix = glm::scale(s);

    glm::mat4 rotationMatrix;

    glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
    float     a            = glm::radians(180.f);
    rotationMatrix *= glm::rotate(a, rotationAxis);


    glm::vec3 t(0.f, 0.f, 0.f);
    glm::mat4 translationMatrix = glm::translate(t);

    glm::mat4 modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;

    // Matrice Model-Vue-Projection:
    glm::mat4 mv = CVar::vue * modelMatrix;

    // Matrice Model-Vue-Projection:
    glm::mat4 mvp = CVar::projection * CVar::vue * modelMatrix;

    // Matrice pour normales (world matrix):
    glm::mat3 mv_n = glm::inverseTranspose(glm::mat3(CVar::vue * modelMatrix));
    err            = glGetError();
    GLuint handle;

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "M");
    glUniformMatrix4fv(handle, 1, GL_FALSE, &modelMatrix[0][0]);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "V");
    glUniformMatrix4fv(handle, 1, GL_FALSE, &CVar::vue[0][0]);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "MVP");
    glUniformMatrix4fv(handle, 1, GL_FALSE, &mvp[0][0]);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "MV");
    glUniformMatrix4fv(handle, 1, GL_FALSE, &mv[0][0]);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "MV_N");
    glUniformMatrix3fv(handle, 1, GL_FALSE, &mv_n[0][0]);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "time");
    glUniform1f(handle, CVar::temps);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "animOn");
    glUniform1i(handle, (int)CVar::animModeleOn);
    err = glGetError();

    ////////////////    Fournir les valeurs de matériaux: //////////////////////////
    GLfloat component[4];
    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Ambient");
    front_mat_ambiant_model.obtenirKA(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Diffuse");
    front_mat_ambiant_model.obtenirKD(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Specular");
    front_mat_ambiant_model.obtenirKS(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Exponent");
    front_mat_ambiant_model.obtenirKE(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Shininess");
    glUniform1f(handle, front_mat_ambiant_model.obtenirShininess());

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Ambient");
    back_mat_ambiant_model.obtenirKA(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Diffuse");
    back_mat_ambiant_model.obtenirKD(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Specular");
    back_mat_ambiant_model.obtenirKS(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Exponent");
    back_mat_ambiant_model.obtenirKE(component);
    glUniform4fv(handle, 1, component);

    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Shininess");
    glUniform1f(handle, back_mat_ambiant_model.obtenirShininess());

    /////////////////////////////////////////////////////////////////////////////////////

    attribuerValeursLumieres(progNuanceurCarte.getProg());
    err = glGetError();
    // ajouts d'autres uniforms
    if (CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee())
        progNuanceurCarte.uniform1("pointLightOn", 1);
    else
        progNuanceurCarte.uniform1("pointLightOn", 0);

    if (CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee())
        progNuanceurCarte.uniform1("dirLightOn", 1);
    else
        progNuanceurCarte.uniform1("dirLightOn", 0);

    if (CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee())
        progNuanceurCarte.uniform1("spotLightOn", 1);
    else
        progNuanceurCarte.uniform1("spotLightOn", 0);
    err = glGetError();

    // glVertexAttrib3f(CCst::indexTangente, 0.0, 0.0, 1.0);
    handle = glGetUniformLocation(progNuanceurCarte.getProg(), "Tangent");
    glUniform3f(handle, -1.0f, 1.0f, 0.0f);

    err = glGetError();
    cartePoly->dessiner(CVar::diffTex);
    err = glGetError();
}
void dessinerSkybox()
{
    // Ajouter une modification dans la matrice Modèle pour éliminer les artéfacts de perspectives.
    progNuanceurSkybox.activer();
	// Ici, j'ai étendu la skybox pour qu'elle paraisse à l'infini relativement au reste.
	int expansion = 15;
	glm::vec3 s(expansion* CCst::grandeurSkybox, expansion* CCst::grandeurSkybox, expansion*CCst::grandeurSkybox);
    glm::mat4 scalingMatrix = glm::scale(s);

    // Effectuer la rotation pour être dans le même sense que le gazon et la caméra (Y+ = UP)
    glm::mat4 rotationMatrix;
    glm::vec3 rotationAxis(1.0f, 0.0f, 0.0f);
    float     a    = glm::radians(-90.f);
    rotationMatrix = glm::rotate(a, rotationAxis);

    glm::mat4 translationMatrix = glm::translate(glm::vec3(0.0f, 0.0f, 0.0f));

    glm::mat4 modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;

    // Matrice Model-Vue-Projection:
    glm::mat4 mvp = CVar::projection * CVar::vue * modelMatrix;

    GLuint handle;

    handle = glGetUniformLocation(progNuanceurSkybox.getProg(), "MVP");
    glUniformMatrix4fv(handle, 1, GL_FALSE, &mvp[0][0]);

    skybox->dessiner();
}

void dessinerMasque(void)
{
	GLenum err = 0;
	progNuanceurCarte.activer();
	// Création d'une matrice-modèle.
	// Défini la translation/rotaion/grandeur du modèle.
	float     scale = cartePoly->obtenirEchelle();
	glm::vec3 s(1, 1, 1);
	glm::mat4 scalingMatrix = glm::scale(s);

	glm::mat4 rotationMatrix;

	glm::vec3 rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
	float     a = glm::radians(180.f);
	rotationMatrix *= glm::rotate(a, rotationAxis);


	glm::vec3 t(0.f, 0.f, -30.f);
	glm::mat4 translationMatrix = glm::translate(t);

	glm::mat4 modelMatrix = translationMatrix * rotationMatrix * scalingMatrix;

	// Matrice Model-Vue-Projection:
	glm::mat4 mv = CVar::vue * modelMatrix;

	// Matrice Model-Vue-Projection:
	glm::mat4 mvp = CVar::projection * CVar::vue * modelMatrix;

	// Matrice pour normales (world matrix):
	glm::mat3 mv_n = glm::inverseTranspose(glm::mat3(CVar::vue * modelMatrix));
	err = glGetError();
	GLuint handle;

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "M");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &modelMatrix[0][0]);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "V");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &CVar::vue[0][0]);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "MVP");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &mvp[0][0]);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "MV");
	glUniformMatrix4fv(handle, 1, GL_FALSE, &mv[0][0]);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "MV_N");
	glUniformMatrix3fv(handle, 1, GL_FALSE, &mv_n[0][0]);

	////////////////    Fournir les valeurs de matériaux: //////////////////////////
	GLfloat component[4];
	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Ambient");
	front_mat_ambiant_model.obtenirKA(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Diffuse");
	front_mat_ambiant_model.obtenirKD(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Specular");
	front_mat_ambiant_model.obtenirKS(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Exponent");
	front_mat_ambiant_model.obtenirKE(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "frontMat.Shininess");
	glUniform1f(handle, front_mat_ambiant_model.obtenirShininess());

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Ambient");
	back_mat_ambiant_model.obtenirKA(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Diffuse");
	back_mat_ambiant_model.obtenirKD(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Specular");
	back_mat_ambiant_model.obtenirKS(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Exponent");
	back_mat_ambiant_model.obtenirKE(component);
	glUniform4fv(handle, 1, component);

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "backMat.Shininess");
	glUniform1f(handle, back_mat_ambiant_model.obtenirShininess());

	/////////////////////////////////////////////////////////////////////////////////////

	attribuerValeursLumieres(progNuanceurCarte.getProg());
	err = glGetError();
	// ajouts d'autres uniforms
	if (CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee())
		progNuanceurCarte.uniform1("pointLightOn", 1);
	else
		progNuanceurCarte.uniform1("pointLightOn", 0);

	if (CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee())
		progNuanceurCarte.uniform1("dirLightOn", 1);
	else
		progNuanceurCarte.uniform1("dirLightOn", 0);

	if (CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee())
		progNuanceurCarte.uniform1("spotLightOn", 1);
	else
		progNuanceurCarte.uniform1("spotLightOn", 0);
	err = glGetError();

	handle = glGetUniformLocation(progNuanceurCarte.getProg(), "Tangent");
	glUniform3f(handle, -1.0f, 1.0f, 0.0f);

	err = glGetError();
	cartePoly->dessiner(CVar::maskTex);
	err = glGetError();
}

void dessinerScene()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    dessinerCarte();
	if (maskOn)
		dessinerMasque();
    // flush les derniers vertex du pipeline graphique
    glFlush();
}

///////////////////////////////////////////////////////////////////////////////
///  global public  clavier \n
///
///  fonction de rappel pour la gestion du clavier
///
///  @param [in]       pointeur GLFWwindow	Référence à la fenetre GLFW
///  @param [in]       touche	int			ID de la touche
///  @param [in]       scancode int			Code spécifique à la plateforme et à l'ID
///  @param [in]       action	int			Action appliquée à la touche
///  @param [in]       mods		int			Modifier bits
///
///  @return Aucune
///
///  @author Félix G. Harvey
///  @date   2016-06-03
///
///////////////////////////////////////////////////////////////////////////////
void clavier(GLFWwindow* fenetre, int touche, int scancode, int action, int mods)
{
    switch (touche)
    {
    case GLFW_KEY_Q:
    {
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(fenetre, GL_TRUE);
        break;
    }
    case GLFW_KEY_ESCAPE:
    {
        if (action == GLFW_PRESS)
            glfwSetWindowShouldClose(fenetre, GL_TRUE);
        break;
    }
    case GLFW_KEY_P:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::isPerspective)
                CVar::isPerspective = false;
            else
                CVar::isPerspective = true;
        }
        break;
    }
    case GLFW_KEY_R:
    {
        if (action == GLFW_PRESS)
        {
            if (rotating)
                rotating = false;
            else
                rotating = true;
        }
        break;
    }
    case GLFW_KEY_I:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::showDebugInfo)
                CVar::showDebugInfo = false;
            else
                CVar::showDebugInfo = true;
        }
        break;
    }

    case GLFW_KEY_C:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::mouseControl)
            {
                CVar::mouseControl = false;
                glfwSetInputMode(fenetre, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            }
            else
            {
                CVar::mouseControl = true;
                glfwSetInputMode(fenetre, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            }
        }

        break;
    }
    case GLFW_KEY_1:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee())
                CVar::lumieres[ENUM_LUM::LumDirectionnelle]->eteindre();
            else
                CVar::lumieres[ENUM_LUM::LumDirectionnelle]->allumer();
        }

        break;
    }
    case GLFW_KEY_2:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee())
                CVar::lumieres[ENUM_LUM::LumPonctuelle]->eteindre();
            else
                CVar::lumieres[ENUM_LUM::LumPonctuelle]->allumer();
        }
        break;
    }
    case GLFW_KEY_3:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee())
                CVar::lumieres[ENUM_LUM::LumSpot]->eteindre();
            else
                CVar::lumieres[ENUM_LUM::LumSpot]->allumer();
        }
        break;
    }

    // permuter le minFilter
    case GLFW_KEY_N:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::minFilter >= 5)
                CVar::minFilter = 0;
            else
                CVar::minFilter++;
        }
        break;
    }

    // permuter le magFilter
    case GLFW_KEY_M:
    {
        if (action == GLFW_PRESS)
        {
            if (CVar::magFilter >= 1)
                CVar::magFilter = 0;
            else
                CVar::magFilter++;
        }
        break;
    }

	// move laser
	case GLFW_KEY_UP:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			if (rotating)
				laser0.turn(0.001,0 , 0);
			else
				laser0.move(-0.2, 0, 0);
		}
		break;
	}

	// move laser
	case GLFW_KEY_DOWN:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			if (rotating)
				laser0.turn(-0.001,0 , 0);
			else
				laser0.move(0.2, 0, 0);
		}
		break;
	}

	// move laser
	case GLFW_KEY_RIGHT:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			if (rotating)
				laser0.turn(0, 0.001, 0);
			else
				laser0.move(0, -0.2, 0);
		}
		break;
	}

	// move laser
	case GLFW_KEY_LEFT:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			if (rotating)
				laser0.turn(0,-0.001, 0);
			else
				laser0.move(0, 0.2, 0);
		}
		break;
	}

	//display mask
	case GLFW_KEY_E:
	{
		if (action == GLFW_PRESS)
		{
			if (maskOn)
				maskOn=false;
			else
				maskOn=true;
		}
		break;
	}

	//change wave length
	case GLFW_KEY_KP_ADD:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			laser0.changeWL(0.02);
		}
		break;
	}

	//change wave length
	case GLFW_KEY_KP_SUBTRACT	:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			laser0.changeWL(-0.02);
		}
		break;
	}
	//change wave length
	case GLFW_KEY_KP_MULTIPLY:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			laser0.changeCos(.999);
		}
		break;
	}
	//change wave length
	case GLFW_KEY_KP_DIVIDE:
	{
		if (action == GLFW_PRESS || action == GLFW_REPEAT)
		{
			laser0.changeCos(1/.999);
		}
		break;
	}
	//choose mask
	case GLFW_KEY_KP_1:
	{
		setMask1();
		break;
	}
	//choose mask
	case GLFW_KEY_KP_2:
	{
		setMask2();
		break;
	}
	//choose mask
	case GLFW_KEY_KP_3:
	{
		setMask3();
		break;
	}
	//choose mask
	case GLFW_KEY_KP_4:
	{
		setMask4();
		break;
	}
    }
}

///////////////////////////////////////////////////////////////////////////////
///  global public  redimensionnement \n
///
///  fonction de rappel pour le redimensionnement de la fenêtre graphique
///
///  @param [in]       w GLsizei    nouvelle largeur "w" en pixels
///  @param [in]       h GLsizei    nouvelle hauteur "h" en pixels
///
///  @return Aucune
///
///  @author Frédéric Plourde
///  @date   2007-12-14
///
///////////////////////////////////////////////////////////////////////////////
void redimensionnement(GLFWwindow* fenetre, int w, int h)
{
    CVar::currentW = w;
    CVar::currentH = h;
    glViewport(0, 0, w, h);
    dessinerScene();
}

void attribuerValeursLumieres(GLuint progNuanceur)
{
    GLenum error = glGetError();

    // Handle pour attribut de lumiere
    GLuint li_handle;

    li_handle = glGetUniformLocation(progNuanceur, "dirLightOn");
    error     = glGetError();
    glUniform1i(li_handle, CVar::lumieres[ENUM_LUM::LumDirectionnelle]->estAllumee());
    error     = glGetError();
    li_handle = glGetUniformLocation(progNuanceur, "pointLightOn");
    glUniform1i(li_handle, CVar::lumieres[ENUM_LUM::LumPonctuelle]->estAllumee());
    li_handle = glGetUniformLocation(progNuanceur, "spotLightOn");
    glUniform1i(li_handle, CVar::lumieres[ENUM_LUM::LumSpot]->estAllumee());
    error = glGetError();

    // Fournir les valeurs d'éclairage au nuanceur.
    // Les directions et positions doivent être en référenciel de caméra.
    for (int i = 0; i < CVar::lumieres.size(); i++)
    {
        // Placeholders pour contenir les valeurs
        GLfloat   temp3[3];
        GLfloat   temp4[4];
        glm::vec4 pos;
        glm::vec4 pos_cam;

        // Creer un descripteur basé sur l'index de lumière
        std::string begin      = "Lights[";
        int         l_idx      = i;
        std::string end        = "]";
        std::string light_desc = begin + std::to_string(l_idx) + end;

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Ambient").c_str());
        CVar::lumieres[i]->obtenirKA(temp3);
        glUniform3fv(li_handle, 1, &temp3[0]);
        error = glGetError();

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Diffuse").c_str());
        CVar::lumieres[i]->obtenirKD(temp3);
        glUniform3fv(li_handle, 1, &temp3[0]);

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Specular").c_str());
        CVar::lumieres[i]->obtenirKS(temp3);
        glUniform3fv(li_handle, 1, &temp3[0]);

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Position").c_str());
        CVar::lumieres[i]->obtenirPos(temp4);

        // Transformer ici la direction/position de la lumière vers un référenciel de caméra
        pos = glm::vec4(temp4[0], temp4[1], temp4[2], temp4[3]);
        pos = CVar::vue * pos;

        temp4[0] = pos.x;
        temp4[1] = pos.y;
        temp4[2] = pos.z;
        temp4[3] = pos.w;
        glUniform4fv(li_handle, 1, &temp4[0]);

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".SpotDir").c_str());
        CVar::lumieres[i]->obtenirSpotDir(temp3);
        // Transformer ici la direction du spot
        pos      = glm::vec4(temp3[0], temp3[1], temp3[2], 0.0f);
        pos      = CVar::vue * pos;
        temp3[0] = pos.x;
        temp3[1] = pos.y;
        temp3[2] = pos.z;
        glUniform3fv(li_handle, 1, &temp3[0]);

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".SpotExp").c_str());
        glUniform1f(li_handle, CVar::lumieres[i]->obtenirSpotExp());

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".SpotCutoff").c_str());
        glUniform1f(li_handle, CVar::lumieres[i]->obtenirSpotCutOff());

        li_handle = glGetUniformLocation(progNuanceur, (light_desc + ".Attenuation").c_str());
        glUniform3f(li_handle, CVar::lumieres[i]->obtenirConsAtt(), CVar::lumieres[i]->obtenirLinAtt(),
                    CVar::lumieres[i]->obtenirQuadAtt());
    }
}

//////////////////////////////////////////////////////////
////////////  FONCTIONS POUR LA SOURIS ///////////////////
//////////////////////////////////////////////////////////

void mouvementSouris(GLFWwindow* window, double deltaT, glm::vec3& direction, glm::vec3& right, glm::vec3& up)
{
    if (CVar::mouseControl)
    {
        // Déplacement de la souris:
        // Taille actuelle de la fenetre
        int mid_width, mid_height;
        glfwGetWindowSize(window, &mid_width, &mid_height);
        mid_width /= 2;
        mid_height /= 2;

        // Get mouse position
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Reset mouse position for next frame
        glfwSetCursorPos(window, mid_width, mid_height);

        // Nouvelle orientation
        horizontalAngle += vitesseSouris * deltaT * float(mid_width - xpos);
        verticalAngle += vitesseSouris * deltaT * float(mid_height - ypos);
    }
    // Direction : Spherical coordinates to Cartesian coordinates conversion
    direction = glm::vec3(cos(verticalAngle) * sin(horizontalAngle), sin(verticalAngle),
                          cos(verticalAngle) * cos(horizontalAngle));

    // Right vector
    right = glm::vec3(sin(horizontalAngle - 3.14f / 2.0f), 0, cos(horizontalAngle - 3.14f / 2.0f));

    // Up vector : perpendicular to both direction and right
    up = glm::cross(right, direction);
}

//////////////////////////////////////////////////////////
////////////  FONCTIONS POUR LA CAMÉRA ///////////////////
//////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///  global public  rafraichirCamera \n
///
///  Fonction de gestion de la position de la caméra en coordonnées sphériques.
///  Elle s'occuper de trouver les coordonnées x et y de la caméra à partir
///  des theta et phi courants, puis fixe dans openGL la position de la caméra
///  à l'aide de gluLookAt().
///
///  @return Aucune
///
///  @author Frédéric Plourde
///  @date   2007-12-14
///
///////////////////////////////////////////////////////////////////////////////
void rafraichirCamera(GLFWwindow* fenetre, double deltaT)
{
    mouvementSouris(fenetre, deltaT, direction, cam_right, cam_up);

    // Move forward
    if (glfwGetKey(fenetre, GLFW_KEY_W) == GLFW_PRESS)
    {
        cam_position += direction * (float)deltaT * vitesseCamera;
    }
    // Move backward
    if (glfwGetKey(fenetre, GLFW_KEY_S) == GLFW_PRESS)
    {
        cam_position -= direction * (float)deltaT * vitesseCamera;
    }
    // Strafe right
    if (glfwGetKey(fenetre, GLFW_KEY_D) == GLFW_PRESS)
    {
        cam_position += cam_right * (float)deltaT * vitesseCamera;
    }
    // Strafe left
    if (glfwGetKey(fenetre, GLFW_KEY_A) == GLFW_PRESS)
    {
        cam_position -= cam_right * (float)deltaT * vitesseCamera;
    }

    // Matrice de projection:
    float ratio = (float)CVar::currentW / (float)CVar::currentH;
    if (CVar::isPerspective)
    {
        // Caméra perspective:
        //

        CVar::projection = glm::perspective(glm::radians(45.0f), ratio, 0.001f, 3000.0f);
    }
    else
    {
        // Caméra orthographique :
        CVar::projection =
            glm::ortho(-5.0f * ratio, 5.0f * ratio, -5.0f, 5.0f, 0.001f, 3000.0f); // In world coordinates
    }

    // Matrice de vue:
    CVar::vue = glm::lookAt(cam_position,             // Position de la caméra
                            cam_position + direction, // regarde vers position + direction
                            cam_up                    // Vecteur "haut"
    );
}

//////////////////////////////////////////////////////////
//////////  FONCTIONS POUR LES NUANCEURS /////////////////
//////////////////////////////////////////////////////////
void compilerNuanceurs()
{
    // on compiler ici les programmes de nuanceurs qui furent prédéfinis

    progNuanceurCarte.compilerEtLier();
    progNuanceurCarte.enregistrerUniformInteger("frontColorMap", CCst::texUnit_0);
    progNuanceurCarte.enregistrerUniformInteger("backColorMap", CCst::texUnit_1);
    progNuanceurCarte.enregistrerUniformInteger("normalMap", CCst::texUnit_2);

    progNuanceurSkybox.compilerEtLier();
    progNuanceurSkybox.enregistrerUniformInteger("colorMap", CCst::texUnit_0);

}


void initDiffraction(void) {
	for (int i = 0; i < resX*resY; i++) {
		texDiffColor[i * 4 + 1] = 0.0;
		texDiffColor[i * 4 + 2] = 0.0;
		texDiffColor[i * 4] = 0.0;
		texDiffColor[i * 4 + 3] = 0.0;
		
		texMaskColor[i * 4 + 1] = 0.0;
		texMaskColor[i * 4 + 2] = 0.0;
		texMaskColor[i * 4] = 0.0;
		texMaskColor[i * 4 + 3] = 0.0;
		mask[i] = 0.0;
	}
	//mask definition : here two points
	for (int i = resY/2; i < resY/2+1; i++) {
		mask[(11 * resX / 21) + i * resX ] = 1.0;
		mask[(10* resX / 21) + i * resX ] = 1.0;
	}
	// mask definition : here a square
	//for (int j = 2 * resY / 5; j < 3 * resY / 5; j++) {
	//	for (int i = 2*resX/5; i < 3 * resX / 5; i++) {
	//		mask[ j *resX + i] = 1.0;
	//	}
	//}

	//here two circles
	//for (int j = 0; j <  resY ; j++) {
	//	for (int i = 0; i < resX; i++) {
	//		int j2 = j - resY / 2;
	//		int i2 = i - 2*resX / 5;
	//		int i3 = i - 3*resX / 5;
	//		if(sqrt(i2*i2+j2*j2)<6 || sqrt(i3*i3 + j2 * j2) < 6)
	//			mask[ j *resX + i] = 1.0;
	//	}
	//}
	// here we define the laser caracteristics : position, angle, wavelength and cosine of aperture angle
	laser0 = CLaser(0, 0, 100, 0, 0, 1, 0.2,0.9);

}

void setMask1(void) {
	for (int i = 0; i < resX*resY; i++) {
		texDiffColor[i * 4 + 1] = 0.0;
		texDiffColor[i * 4 + 2] = 0.0;
		texDiffColor[i * 4] = 0.0;
		texDiffColor[i * 4 + 3] = 0.0;

		texMaskColor[i * 4 + 1] = 0.0;
		texMaskColor[i * 4 + 2] = 0.0;
		texMaskColor[i * 4] = 0.0;
		texMaskColor[i * 4 + 3] = 0.0;
		mask[i] = 0.0;
	}
	//mask definition : here two points
	for (int i = resY/2; i < resY/2 +1 ; i++) {
		mask[(11 * resX / 21) + i * resX] = 1.0;
		mask[(10 * resX / 21) + i * resX] = 1.0;
	}

}

void setMask2(void) {
	for (int i = 0; i < resX*resY; i++) {
		texDiffColor[i * 4 + 1] = 0.0;
		texDiffColor[i * 4 + 2] = 0.0;
		texDiffColor[i * 4] = 0.0;
		texDiffColor[i * 4 + 3] = 0.0;

		texMaskColor[i * 4 + 1] = 0.0;
		texMaskColor[i * 4 + 2] = 0.0;
		texMaskColor[i * 4] = 0.0;
		texMaskColor[i * 4 + 3] = 0.0;
		mask[i] = 0.0;
	}
	//mask definition : here two slits
	for (int i = 0; i < resY; i++) {
		mask[(11 * resX / 21) + i * resX] = 1.0;
		mask[(10 * resX / 21) + i * resX] = 1.0;
	}

}
void setMask3(void) {
	for (int i = 0; i < resX*resY; i++) {
		texDiffColor[i * 4 + 1] = 0.0;
		texDiffColor[i * 4 + 2] = 0.0;
		texDiffColor[i * 4] = 0.0;
		texDiffColor[i * 4 + 3] = 0.0;

		texMaskColor[i * 4 + 1] = 0.0;
		texMaskColor[i * 4 + 2] = 0.0;
		texMaskColor[i * 4] = 0.0;
		texMaskColor[i * 4 + 3] = 0.0;
		mask[i] = 0.0;
	}
	// mask definition : here a square
	for (int j = 2 * resY / 5; j < 3 * resY / 5; j++) {
		for (int i = 2*resX/5; i < 3 * resX / 5; i++) {
			mask[ j *resX + i] = 1.0;
		}
	}

}

void setMask4(void) {
	for (int i = 0; i < resX*resY; i++) {
		texDiffColor[i * 4 + 1] = 0.0;
		texDiffColor[i * 4 + 2] = 0.0;
		texDiffColor[i * 4] = 0.0;
		texDiffColor[i * 4 + 3] = 0.0;

		texMaskColor[i * 4 + 1] = 0.0;
		texMaskColor[i * 4 + 2] = 0.0;
		texMaskColor[i * 4] = 0.0;
		texMaskColor[i * 4 + 3] = 0.0;
		mask[i] = 0.0;
	}
	//here two circles
	for (int j = 0; j <  resY ; j++) {
		for (int i = 0; i < resX; i++) {
			int j2 = j - resY / 2;
			int i2 = i - 2*resX / 5;
			int i3 = i - 3*resX / 5;
			if(sqrt(i2*i2+j2*j2)<6 || sqrt(i3*i3 + j2 * j2) < 6)
				mask[ j *resX + i] = 1.0;
		}
	}
}

void updateLaser() {

	glGenTextures(1, &CVar::maskTex);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, CVar::maskTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resX, resY, 0, GL_RGBA, GL_FLOAT, texMaskColor);
	//laser animation is possible 
	//Change variable name if used with the FPS counter 
	/*
	double current_time = glfwGetTime();
	if (current_time - previous_time > .1) {
		laser0.move(0, .3, 0);
		laser0.turn(-0.01,0 , 0);

		previous_time = current_time;

	}*/
	
}
void laserIntersect(CLaser laser) {
	GLfloat cosAngle = laser.getCosAngle();
	GLfloat position[3];
	laser.getPos(position);
	GLfloat direction[3];
	laser.getDir(direction);
	REAL cos2;
	CVecteur3 vec;
	for (int i = 0; i < resY; i++) {
		for (int j = 0; j < resX; j++) {
			vec = CVecteur3::Normaliser(CVecteur3(position[0] + i - resY / 2, position[1] + j - resX / 2, position[2]));
			
			cos2 = CVecteur3::ProdScal(vec, CVecteur3::Normaliser(CVecteur3(direction[0], direction[1], direction[2])));
			texMaskColor[i*resX * 4 + j * 4 + 1] = mask[i*resX + j];
			if (cos2 > cosAngle) {
				texDiffColor[i*resX * 4 + j * 4 + 1] = mask[i*resX + j];
				texMaskColor[i*resX * 4 + j * 4 ] = 1.0;
			}
			else {
				texDiffColor[i*resX * 4 + j * 4 + 1] = 0;
				texMaskColor[i*resX * 4 + j * 4] = 0;
			}
		}
	}
}


void calculDiffraction()
{
    // construire la texture
    // calculer la texture de la figure de diffraction

	float distance = 400.0;
	float dir[3];
	float waveLen = laser0.getWL();
	laser0.getDir(dir);
	
	glGenTextures(1, &CVar::diffTex);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, CVar::diffTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, resX, resY, 0, GL_RGBA, GL_FLOAT, texDiffColor);

	glBindImageTexture(0, CVar::diffTex, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

	glUseProgram(computeHandle);
	glUniform1f(glGetUniformLocation(computeHandle, "resX"), resX);
	glUniform1f(glGetUniformLocation(computeHandle, "resY"), resY);
	glUniform1f(glGetUniformLocation(computeHandle, "wavelen"), waveLen);
	glUniform1f(glGetUniformLocation(computeHandle, "distance"), distance);
	glUniform1f(glGetUniformLocation(computeHandle, "dirx"), dir[0]);
	glUniform1f(glGetUniformLocation(computeHandle, "diry"), dir[1]);
	glUniform1f(glGetUniformLocation(computeHandle, "dirz"), dir[2]);
	glDispatchCompute(resX /16, resY /16, 1); // blocks of 16^2

	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

void
setWindowFPS(GLFWwindow* fen)
{
	// Measure speed
	double currentTime = glfwGetTime();
	nbFrames++;

	if (currentTime - previous_time >= 0.5) { // If last cout was more than .5 sec ago
		previous_time = currentTime;
		char title[256];
		title[255] = '\0';

		snprintf(title, 255,
			"Diffraction wavelen %2.2f - [FPS: %d]" ,
			laser0.getWL(), nbFrames*2);

		glfwSetWindowTitle(fen, title);

		nbFrames = 0;
	}
}
