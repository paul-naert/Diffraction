#version 430 core
struct Light
{
        vec3 Ambient; 
        vec3 Diffuse;
        vec3 Specular;
        vec4 Position;  // Si .w = 1.0 -> Direction de lumiere directionelle.
        vec3 SpotDir;
        float SpotExp;
        float SpotCutoff;
        vec3 Attenuation; //Constante, Lineraire, Quadratique
};

struct Mat
{
        vec4 Ambient; 
        vec4 Diffuse;
        vec4 Specular;
        vec4 Exponent;
        float Shininess;
};

layout(location = 0) in vec2 vt;
layout(location = 1) in vec3 vn;
layout(location = 2) in vec3 vp;

uniform int pointLightOn;
uniform int spotLightOn;
uniform int dirLightOn;
uniform mat4 MVP;
uniform mat4 MV;
uniform mat3 MV_N;
uniform mat4 M;
uniform mat4 V;
uniform Light Lights[3]; // 0:ponctuelle  1:spot  2:directionnelle
uniform Mat Material;

in vec2 fragTexCoord;

//Accumulateurs de contributions Ambiantes et Diffuses:
vec4 Ambient;
vec4 Diffuse;

// Calcul pour une lumi�re ponctuelle
void pointLight(in vec3 lightVect, in vec3 normal)
{
   float nDotVP;       // Produit scalaire entre VP et la normale
   float attenuation;  // facteur d'att�nuation calcul�
   float d;            // distance entre lumi�re et fragment
   vec3  VP;           // Vecteur lumi�re
   vec3 lightatt;	   // Vecteur attenuation

   // Calculer vecteur lumi�re
   VP = lightVect;

   // Calculer distance � la lumi�re
   d = length (VP);

   // Normaliser VP
   VP = normalize(VP);

   // Calculer l'att�nuation due � la distance
   lightatt= Lights[0].Attenuation;
   attenuation = 1/(lightatt.x+d*lightatt.y+d*d*lightatt.z);
   
   //Produit scalaire lumi�re/normale
   nDotVP = max(0,dot(VP,normal));

   // Calculer les contributions ambiantes et diffuses
   Ambient.xyz  += Lights[0].Ambient*attenuation;
   Diffuse.xyz  += Lights[0].Diffuse*nDotVP*attenuation;
}


// Calcul pour une lumi�re directionnelle
void directionalLight(in vec3 lightVect, in vec3 normal)
{
   vec3  VP = lightVect;             // Vecteur lumi�re
   float nDotVP;         // Produit scalaire entre VP et la normale

   nDotVP = max(0,dot(normalize(VP),normal));

   // Calculer les contributions ambiantes et diffuses
   Ambient.xyz  += Lights[2].Ambient;
   Diffuse.xyz  += Lights[2].Diffuse*nDotVP;
}


// Calcul pour une lumi�re "spot"
void spotLight(in vec3 lightVect, in vec3 normal)
{
   float nDotVP;             // Produit scalaire entre VP et la normale
   float spotAttenuation;    // Facteur d'att�nuation du spot
   float attenuation;        // Facteur d'att�nuation du � la distance
   float angleEntreLumEtSpot;// Angle entre le rayon lumieux et le milieu du cone
   float d;                  // Distance � la lumi�re
   vec3  VP;                 // Vecteur lumi�re
   vec3 lightatt;

   // Calculer le vecteur Lumi�re
   VP = lightVect;

   // Calculer la distance � al lumi�re
   d = length(VP);

   // Normaliser VP
   VP = normalize(VP);

   // Calculer l'att�nuation due � la distance
   lightatt= Lights[1].Attenuation;
   attenuation = 1/(lightatt[0]+d*lightatt[1]+d*d*lightatt[2]);

   // Le fragment est-il � l'int�rieur du c�ne de lumi�re ?
   vec3 spotDir = Lights[1].SpotDir;
   vec3 lightDir = -normalize(spotDir);
   angleEntreLumEtSpot = acos(max(0,dot(lightDir,VP)))*180/3.14159;

   if (angleEntreLumEtSpot > Lights[1].SpotCutoff)
   {
       spotAttenuation = 0.0; // en dehors... aucune contribution
   }
   else
   {
       spotAttenuation = 1; //pleine lumi�re dans le spot

   }

   // Combine les att�nuation du spot et de la distance
   attenuation *= spotAttenuation;

    nDotVP = dot(normal,VP);

   // Calculer les contributions ambiantes et diffuses
   Ambient.xyz  += Lights[1].Ambient*attenuation;
   Diffuse.xyz  += Lights[1].Diffuse*attenuation*nDotVP;
}

vec4 flight(in vec3 light0Vect, in vec3 light1Vect, in vec3 light2Vect, in vec3 normal)
{
    vec4 color;
    vec3 ecPosition3;

    // R�initialiser les accumulateurs
    Ambient  = vec4 (0.0);
    Diffuse  = vec4 (0.0);

    if (pointLightOn == 1) {
        pointLight(light0Vect, normal);
    }
    if (spotLightOn == 1) {
        spotLight(light1Vect, normal);
    }
    if (dirLightOn == 1) {
        directionalLight(light2Vect, normal);
    }
    
    //� d�-commenter!
    color = (Ambient * Material.Ambient + Diffuse  * Material.Diffuse);
    color = clamp( color, 0.0, 1.0 );
    
    // � supprimer !
    //color = vec4(0.0, 1.0, 0.0, 1.0);
    
    return color;
}
in vec3 light0Vect;
in vec3 light1Vect;
in vec3 light2Vect;
in vec3 normal_cameraSpace;

uniform sampler2D colorMap;

out vec4 color;
void main (void)
{
	// Compl�ter afin d'ajouter la contribution de la texture:
    vec4 out_color;
    out_color = flight(light0Vect, light1Vect, light2Vect, normal_cameraSpace);
    
    //Contribution de la texture:
    out_color*=texture2D(colorMap,fragTexCoord);
    
    color = clamp(out_color, 0.0, 1.0);
}
