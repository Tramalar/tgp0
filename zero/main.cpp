#include <cmath>
#include "GL/glew.h"
#include <iostream>
#include <sstream>
#include <vector>
#include "GL/freeglut.h"
#include "vecmath.h"
using namespace std;

// Globals

// This is the list of points (3D vectors)
vector<Vector3f> vecv;

// This is the list of normals (also 3D vectors)
vector<Vector3f> vecn;

// This is the list of faces (indices into vecv and vecn)
vector<unsigned int> vecfv;
vector<unsigned int> vecfn;
vector<float> vertdata;


// You will need more global variables to implement color and position changes

//valitun värin indeksi
int next_col=0;

//valon sijainti
GLfloat lightpos[4]={1.0f, 1.0f, 5.0f, 1.0f};

//pyörimisnopeus
int rot_speed=0;

//kulma, jossa esine on tällä hetkellä
int curr_angle_Y=0;
int curr_angle_X=0;
	

// Here are some colors you might use - feel free to add more
GLfloat diffColors[4][4] = { {0.5, 0.5, 0.9, 1.0},
                             {0.3, 0.8, 0.9, 1.0},
                             {0.9, 0.5, 0.5, 1.0},
                             {0.5, 0.9, 0.3, 1.0} };
//Käsittelee välissä esiintyvät värit väriä vaihdettaessa
GLfloat trans_col[4]={0.5, 0.5, 0.9, 1.0};
   	
//selvittää kuinka lähellä tai kaukana kamera on
GLdouble zoom_value=5.0;

//selvittää onko hiiren vasen näppäin påainettuna
bool mouse_down=false;

// These are convenience functions which allow us to call OpenGL 
// methods on Vec3d objects
inline void glVertex(const Vector3f &a) 
{ glVertex3fv(a); }

inline void glNormal(const Vector3f &a) 
{ glNormal3fv(a); }


//muuttaa esineen värin pehmeästi siirtämällä RGB-arvoja oikeaan suuntaan 0.01:n verran
void transit_color(int value){
	for(int i=0;i<4;i++){
		float diff=diffColors[next_col][i]-trans_col[i];
		if(abs(diff)>=.01){
			trans_col[i]+=((diff<0)? -.01: .01);
		}
		else{
			trans_col[i]=diffColors[next_col][i];
		}
	}
    glutPostRedisplay();
	glutTimerFunc(25,transit_color,0);
}


// This function is called whenever a "Normal" key press is received.
void keyboardFunc( unsigned char key, int x, int y )
{
    switch ( key )
    {
    case 27: // Escape key
        exit(0);
        break;
	//vaihdetaan väri seuraavaksi vuorossa olevaksi, palataan listan alkuun mikäli ollaan saavuttu loppuun
    case 'c':
		for(int i =0;i<4;i++){
			trans_col[i]=diffColors[next_col][i];
		}
		next_col=(next_col+1)%4;
        break;
    case 'r':
        // asettaa pyörähdysnopeudeksi 2, jos se on 0 ja toisinpäin
		rot_speed=(rot_speed==2) ? 0 : 2;
        break;
    default:
        cout << "Unhandled key press " << key << "." << endl;        
    }

	// this will refresh the screen so that the user sees the color change
    glutPostRedisplay();
}

// This function is called whenever a "Special" key press is received.
// Right now, it's handling the arrow keys.

// Siirretään valon sijaintia nuolinäppäimellä osoitettuun suuntaan
void specialFunc( int key, int x, int y )
{
    switch ( key )
    {
    case GLUT_KEY_UP:
		lightpos[1]+=.5;
		break;
    case GLUT_KEY_DOWN:
		lightpos[1]-=.5;
		break;
    case GLUT_KEY_LEFT:
		lightpos[0]-=.5;
		break;
    case GLUT_KEY_RIGHT:
		lightpos[0]+=.5;
		break;
    }

	// this will refresh the screen so that the user sees the light position
    glutPostRedisplay();
}

void mouseWheel(int button, int dir, int x, int y)
{
    if (dir > 0)
    {
		if(zoom_value>2){
			zoom_value-=.5;
		}
    }
    else
    {
		if(zoom_value<50){
			zoom_value+=.5;
		}
    }
	
    glutPostRedisplay();
}

int clickX=0;
int clickY=0;

void mouseClick(int button, int state, int x, int y){
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		mouse_down = true;
		clickX=x;
		clickY=y;
	}
	else
		mouse_down = false;
}

void mouseMove(int x, int y){
	if(mouse_down){
		curr_angle_X=(curr_angle_X+y-clickY)%360;
		curr_angle_Y=(curr_angle_Y+x-clickX)%360;
		clickX=x;
		clickY=y;
	}
    glutPostRedisplay();
}

//päivittää kulman, pitää myös huolta ettei mene yli 360:n
void update_angle(int value){
	curr_angle_Y=(curr_angle_Y+rot_speed)%360;
    glutPostRedisplay();
	glutTimerFunc(25,update_angle,0);
}




// This function is responsible for displaying the object.
void drawScene(void)
{
    // Clear the rendering window
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	// Rotate the image
	glMatrixMode( GL_MODELVIEW );  // Current matrix affects objects positions
	glLoadIdentity();              // Initialize to the identity  
	
	// Position the camera at [0,0,5], looking at [0,0,0],
	// with [0,1,0] as the up direction.
	gluLookAt(0.0, 0.0, zoom_value,
	          0.0, 0.0, 0.0,
	          0.0, 1.0, 0.0);
	
	// Set material properties of object
	// Here we use the first color entry as the diffuse color
	//muutettu valitsemaan väriksi vuorossa oleva väri


    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, trans_col);
   	
	// Define specular color and shininess
    GLfloat specColor[] = {1.0, 1.0, 1.0, 1.0};
    GLfloat shininess[] = {100.0};
    
	// Note that the specular color and shininess can stay constant
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specColor);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shininess);
   	
    // Set light properties
   	
    // Light color (RGBA)
    GLfloat Lt0diff[] = {1.0,1.0,1.0,1.0};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, Lt0diff);
    //Light position
    //käyttää nyt globaalia lightpos-muuttujaa, joka muuttuu nuolinäppäimien toimesta
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    //
	// This GLUT method draws a teapot.  You should replace
	// it with code which draws the object you loaded.
	glRotatef(curr_angle_Y, 0.0f,1.0f,0.0f);
	glRotatef(curr_angle_X, 1.0f,0.0f,0.0f);
	//glutSolidTeapot(1.0);

	GLuint VBO;
	GLuint IBO;

	glGenBuffers(1,&VBO);
	glBindBuffer(GL_ARRAY_BUFFER,VBO);
	glBufferData(GL_ARRAY_BUFFER,sizeof(vertdata[0])*vertdata.size(),&vertdata[0],GL_STATIC_DRAW);
	
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,6 * sizeof(GLfloat),0);
	
  glEnableClientState(GL_NORMAL_ARRAY);
  glNormalPointer(GL_FLOAT, 6 * sizeof(GLfloat), (void*)(3*sizeof(float)));  

	glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vecfv[0])*vecfv.size(), vecfv.data(), GL_STATIC_DRAW);

	
    glDrawElements(GL_TRIANGLES, vecfv.size(), GL_UNSIGNED_INT, 0);
	glutSwapBuffers();


}
// Initialize OpenGL's rendering modes
void initRendering()
{
    glEnable(GL_DEPTH_TEST);   // Depth testing must be turned on
    glEnable(GL_LIGHTING);     // Enable lighting calculations
    glEnable(GL_LIGHT0);       // Turn on light #0.
}

// Called when the window is resized
// w, h - width and height of the window in pixels.
void reshapeFunc(int w, int h)
{
    // Always use the largest square viewport possible
    if (w > h) {
        glViewport((w - h) / 2, 0, h, h);
    } else {
        glViewport(0, (h - w) / 2, w, w);
    }

    // Set up a perspective view, with square aspect ratio
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    // 50 degree fov, uniform aspect ratio, near = 1, far = 100
    gluPerspective(50.0, 1.0, 1.0, 100.0);
	glutSwapBuffers();
}


void loadInput()
{
	char buffer[256];
    
	while (cin.getline(buffer, 256)){
		stringstream ss(buffer);
		string s;
		ss>>s;
	    Vector3f v;
		if(s=="v"){
	        ss >> v[0] >> v[1] >> v[2];
			vecv.push_back(v);
		}
		if(s=="vn"){
	        ss >> v[0] >> v[1] >> v[2];
			vecn.push_back(v);
		}
		if(s=="f"){
			vector<string> vstr(3,"");
			vector<unsigned> v1(3);
			vector<unsigned> v2(3);
			//ensin tallennetaan kautta-viivoilla erotellut arvot merkkijonovektoriin
			ss >> vstr[0] >> vstr[1] >> vstr[2];
			    int j=0;
			for(int i=0;i<3;i++){
				//luodaan streami kustakin merkkijonosta
				stringstream ss1(vstr[i]);
				string ind;
				//erotellaan luotu streami kauttaviivojen kohdalta ja lisätään saadut luvut lopulliseen vektoriin
				while(getline(ss1,ind,'/')){
					stringstream ssind(ind);
					if(j%3==0){ssind>>v1[j/3];}
					if(j%3==2){ssind>>v2[j/3];}
					j++;
				}
			}
			for(int i=0;i<3;i++){
			vecfv.push_back(v1[i]);
			}
		}
	}
	// load the OBJ file here
}
void formatData(){
	for (int i =0; i< vecv.size();i++){
		for(int j=0;j<3;j++){
			vertdata.push_back(vecv[i][j]);
		}
		for(int j=0;j<3;j++){;
			vertdata.push_back(vecn[i][j]);
		}
	}
}

// Main routine.
// Set up OpenGL, define the callbacks and start the main loop
int main( int argc, char** argv )
{
    loadInput();
	formatData();

    glutInit(&argc,argv);

    // We're going to animate it, so double buffer 
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH );

    // Initial parameters for window position and size
    glutInitWindowPosition( 60, 60 );
    glutInitWindowSize( 360, 360 );
    glutCreateWindow("Assignment 0");
	glewInit();


    // Initialize OpenGL parameters.
    initRendering();
    // Set up callback functions for key presses
	glutKeyboardFunc(keyboardFunc); // Handles "normal" ascii symbols
	glutSpecialFunc(specialFunc);   // Handles "special" keyboard keys
	glutMouseWheelFunc(mouseWheel); //zoomi hiiren rullaan
	glutMouseFunc(mouseClick);
	glutMotionFunc(mouseMove);
	glutTimerFunc(25,transit_color,0);
	glutTimerFunc(25,update_angle,0);


     // Set up the callback function for resizing windows
    glutReshapeFunc( reshapeFunc );

    // Call this whenever window needs redrawing
    glutDisplayFunc( drawScene );

    // Start the main loop.  glutMainLoop never returns.
    glutMainLoop( );

    return 0;	// This line is never reached.
}
