#include <fstream>
#include <sstream>
#include <string>
#include <cassert>
#include <set>
#include <vector>
#include <iostream>
#include <windows.h> 
#include <stdlib.h> 
#include <GL/freeglut.h>
#include <GL/glui.h>
#include <math.h>

using namespace std;

static int press_x, press_y;
static int obj_mode = 0;
static int xform_mode = 0;
static float x_angle = 0.0;
static float y_angle = 0.0;
static float x_translate = 0.0;
static float y_translate = 0.0;
static float scale_size = 1;

#define TRANSFORM_NONE    0 
#define TRANSFORM_ROTATE  1
#define TRANSFORM_SCALE   2 
#define TRANSFORM_TRANSLATE   3 

#define OBJ_WIREFRAME	0
#define OBJ_SOLID		1
#define OBJ_EDGE		2 

struct vector3f
{
	float x, y, z;
};
struct vector3i
{
	int a, b, c;
};

typedef vector<vector3f> pointVec;
typedef vector<vector3i> faceVec;

pointVec vertList;

faceVec faceList;

struct HE_vert;
struct HE_face;

struct HE_edge
{
	HE_vert* vert; // vertex at the end of the half-edge
	HE_edge* pair; // oppositely oriented half-edge
	HE_face* face; // the incident face
	HE_edge* prev; // previous half-edge around the face
	HE_edge* next; // next half-edge around the face
};

struct HE_vert
{
	float x; // the vertex coordinates
	float y;
	float z;
	HE_edge* edge; // one of the half-edges emanating from the vertex
	float nx, ny, nz;
	
	pointVec adjacentList;
	vector<float> area;
	int add = 0;
};

struct HE_face {
	HE_edge* edge; // one of the half-edges bordering the face
};

typedef vector<HE_vert> HE_points;
typedef vector<HE_edge> HE_lines;
typedef vector<HE_face> HE_faces;

HE_points HEvertList;
HE_points HENvertList;
HE_lines  HEedgeList;
HE_faces HEfaceList;
pointVec NvertList;
pointVec NfaceList;

int numofVec = 0;
int numofEdge = 0;
int numofFace = 0;
float xmax=0, xmin=0, ymax=0, ymin=0, zmax=0, zmin=0;
std::string s;
std::string g;
string filename;
//--------------------------------START M FILE PARSER----------------------------------------------
int strToint(char *str) {
	int n;
	std::stringstream ss(str);
	ss >> n;
	return n;
}

float strTofloat(char *str) {
	float n;
	std::stringstream ss(str);
	ss >> n;
	return n;
}

char intTochar(int w) {
	char temp;
	temp = w;
	return temp;
}

void readfile(string path) {
	
	const string points_filename(path);
	cout << "--- ReadFile " << filename << endl;
	ifstream input(points_filename.c_str());
	if (!input)
		cout << "OPEN FAILTURE";
	else { cout << "OPEN SUCCESS" << endl; }

	//----------------Skip Comment '#' and Clear Line----------------
	string clearline, buffer;
	while (getline(input, buffer)) {//null

		if (buffer[0] != '#')
			clearline += buffer;
		clearline += ' ';
	}
	cout << "--- Clearline Funciton Succeed ---" << endl;

	//----------------Push data----------------------------------------
	char* char_line = (char*)clearline.c_str(); // transfer from string to char
	cout << "--- Transfer Function Succeed ---" << endl;

	char* strToken = NULL;
	char* next_token = NULL;
	char  delims[] = " ";

	// During the first read, establish the char string and get the first token.
	strToken = strtok_s(char_line, delims, &next_token);
	int  i = 0, j = 0;
	while (strToken != NULL) {
		int key = 2;		//int key For temp
		vector3f tvert;
		vector3i tface;

		if (strcmp(strToken, "Vertex") == 0) {
			key = 0;
			strToken = strtok_s(NULL, delims, &next_token);
			strToken = strtok_s(NULL, delims, &next_token);
			tvert.x = strTofloat(strToken);
			strToken = strtok_s(NULL, delims, &next_token);
			tvert.y = strTofloat(strToken);
			strToken = strtok_s(NULL, delims, &next_token);
			tvert.z = strTofloat(strToken);
			vertList.push_back(tvert);
			numofVec = numofVec + 1;

		}
		else {
			if (strcmp(strToken, "Face") == 0) {
				key = 1;
				strToken = strtok_s(NULL, delims, &next_token);
				strToken = strtok_s(NULL, delims, &next_token);
				tface.a = strToint(strToken);
				strToken = strtok_s(NULL, delims, &next_token);
				tface.b = strToint(strToken);
				strToken = strtok_s(NULL, delims, &next_token);
				tface.c = strToint(strToken);
				faceList.push_back(tface);
				numofFace = numofFace + 1;
			}
			else { key = 2; }
		}
		strToken = strtok_s(NULL, delims, &next_token);
	}
	cout << "--- Data Pushing Process Succeed ---" << endl;
	cout << "Number of Vertex in Current Model:  " << numofVec << endl;
	cout << "Number of Face in Current Model:  " << numofFace << endl;
	s = std::to_string(numofVec);
	g = std::to_string(numofFace);
	
	input.close();
}

void ScalingData() {
	
	float scale;
	float xr, yr, zr;
	for (int i = 0; i < numofVec; i++) {
		if (xmin > vertList[i].x) xmin = vertList[i].x;
		if (xmax < vertList[i].x) xmax = vertList[i].x;
		if (ymin > vertList[i].y) ymin = vertList[i].y;
		if (ymax < vertList[i].y) ymax = vertList[i].y;
		if (zmin > vertList[i].z) zmin = vertList[i].z;
		if (zmax < vertList[i].z) zmax = vertList[i].z;
	}
	xr = xmax - xmin;
	yr = ymax - ymin;
	zr = zmax - zmin;
	if (xr < 5 && yr < 5 && zr < 5) {
		float n = max(zr, max(xr, yr));
		scale = 10.0 / n;
		cout << "--- Scaling Models in  " << scale << " Times ---" << endl;
		for (int i = 0; i < numofVec; i++) {
			vertList[i].x = scale*vertList[i].x;
			vertList[i].y = scale*vertList[i].y;
			vertList[i].z = scale*vertList[i].z;
			

		}
			xmax = xmax * scale;
			ymax = ymax * scale;
			zmax = zmax * scale;
			xmin = xmin * scale;
			ymin = ymin * scale;
			zmin = zmin * scale;
	}
	else {
		if (xr > 10 && yr > 10 && zr > 10) {
			float n = max(zr, max(xr, yr));
			scale = n/10;
			cout << "--- Scaling Models in  " << scale << " Times ---" << endl;
			for (int i = 0; i < numofVec; i++) {
				vertList[i].x = vertList[i].x / scale;
				vertList[i].y = vertList[i].y / scale;
				vertList[i].z = vertList[i].z / scale;
				
			}
			xmax = xmax / scale;
				ymax = ymax / scale; 
				zmax = zmax / scale;
				xmin = xmin / scale;
				ymin = ymin / scale;
				zmin = zmin / scale;
		}

		
	}
	scale = 0.0;//reset
}
//--------------------------------FINISH M FILE PARSER---------------------------------------------

//--------------------------------HALF EDGE--------------------------------------------------------
//void CcwOrdering(HE_edge A, HE_edge B, HE_edge C) {}

vector3f crossproduct(HE_vert m, HE_vert n) {
	vector3f normal;
	normal.x = m.y*n.z - m.z*n.y;
	normal.y = m.z*n.x - m.x*n.z;
	normal.z = m.x*n.y - m.y*n.x;
	return normal;
}

float trianglearea(HE_vert m, HE_vert n) {
	float area;
	area =(m.y*n.z - m.z*n.y + m.z*n.x - m.x*n.z + m.x*n.y - m.y*n.x)/2;
	area = fabs(area);
	return area;
}

void OneRingTraversal() {

	HE_vert thvert;
	HE_edge thedge1, thedge2, thedge3;
	HE_face thface;
	vector3f nff;
	ScalingData();
	int i = 0, j = 0, n = 0;
	for (i = 0; i < numofVec; i++) { // push the verctor data into HEvertList
		if (i == 0) cout << "--- Start HE Vertex Transfer Process ---" << endl;
		thvert.x = vertList[i].x;
		thvert.y = vertList[i].y;
		thvert.z = vertList[i].z;
		
		HEvertList.push_back(thvert);
	}
	cout << " ---- Complete Process ----" << endl;
	cout << "---- Comlete " << i << " Vertex ----" << endl;
	
	while (numofFace - j) {
		int a = faceList[j].a - 1;
		int b = faceList[j].b - 1;
		int c = faceList[j].c - 1;
		HE_vert temp1, temp2;
		thedge1.vert = &(HEvertList[a]);
		thedge2.vert = &(HEvertList[b]);
		thedge3.vert = &(HEvertList[c]);

		temp1.x = thedge2.vert->x - thedge1.vert->x;
		temp1.y = thedge2.vert->y - thedge1.vert->y;
		temp1.z = thedge2.vert->z - thedge1.vert->z;

		temp2.x = thedge3.vert->x - thedge1.vert->x;
		temp2.y = thedge3.vert->y - thedge1.vert->y;
		temp2.z = thedge3.vert->z - thedge1.vert->z;


		thedge1.next = &(thedge2);
		thedge2.next = &(thedge3);
		thedge3.next = &(thedge1);

		thedge1.prev = &(thedge3);
		thedge2.prev = &(thedge1);
		thedge3.prev = &(thedge2);

		thface.edge = &(thedge1);

		thedge1.face = &thface;
		thedge2.face = &thface;
		thedge3.face = &thface;

		HEedgeList.push_back(thedge1);
		HEedgeList.push_back(thedge2);
		HEedgeList.push_back(thedge3);
		numofEdge += 3;
		HEfaceList.push_back(thface);

		nff = crossproduct(temp1, temp2);
		NfaceList.push_back(nff);

		float temp = trianglearea(temp1, temp2);

		(HEvertList[a].adjacentList).push_back(nff);
		(HEvertList[a].area).push_back(temp);
		HEvertList[a].add++;
		(HEvertList[b].adjacentList).push_back(nff);
		(HEvertList[b].area).push_back(temp);
		HEvertList[b].add++;
		(HEvertList[c].adjacentList).push_back(nff);
		(HEvertList[c].area).push_back(temp);
		HEvertList[c].add++;
		j++;
	}
	cout << "---- Complete Process ----" << endl;
}

void vertexnormal() {//calculate vertex 
	cout << "Start calculate vertexnormal" << endl;
	vector3f Tnvect;
	HE_vert temp;
	Tnvect.x = 0.0;
	Tnvect.y = 0.0;
	Tnvect.z = 0.0;
	temp.x = 0.0;
	temp.y = 0.0;
	temp.z = 0.0;
	
	float areasum = 0.0;

	for (int i = 0; i < numofVec;i++) {
		temp = HEvertList[i];
		
		int j = HEvertList[i].add;
		float length = sqrt(pow(temp.x,2)+ pow(temp.y, 2)+ pow(temp.z, 2));
		for (int c = 0; c < j; c++) {
			 areasum += HEvertList[i].area[c];
		}
		for (int q = 0; q <j ; q++) {
			Tnvect.x += (temp.adjacentList[q].x)*HEvertList[i].area[q] / areasum;
			Tnvect.y += (temp.adjacentList[q].y)*HEvertList[i].area[q] / areasum;
			Tnvect.z += (temp.adjacentList[q].z)*HEvertList[i].area[q] / areasum;
		}
		temp.nx = Tnvect.x / length;
		temp.ny = Tnvect.y / length;
		temp.nz = Tnvect.z / length;
		HENvertList.push_back(temp);
		areasum = 0.0;
		Tnvect.x = 0.0;
		Tnvect.y = 0.0;
		Tnvect.z = 0.0;
		temp.x = 0.0;
		temp.y = 0.0;
		temp.z = 0.0;//reset
	}
	cout << "Complete calculate vertexnormal" << endl;
}

//--------------------------------HALF EDGE--------------------------------------------------------

//--------------------------------OPENGL RENDERING-------------------------------------------------

int rotateX = 0, rotateY = 0, rotateZ = 0;
float l = 10;//length of axis
double ex = 1;
double ey = 1;
double ez = 1;
double upx = 0;
double upy = 1;
double upz = 0;
double min = 1; //  In GL_MODELVIEW mode, the stack depth is at least 32
int main_window;
float color[] = {0.0, 0.4, 0.5};

void InitGL(void)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//// set projection parameters
	glClearColor(0.1, 0.1, 0.1, 0);// set display-window color to white


	glMatrixMode(GL_MODELVIEW);
	glEnable(GL_NORMALIZE);

	GLfloat light_position[] = { 1.0, 1.0, 0.0, 0.0 }; // light position
	GLfloat white_light[] = { 1.0, 1.0, 1.0, 0.0 }; // light color
	GLfloat lmodel_ambient[] = { 1.0, 1.0, 1.0, 1.0 };
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white_light);
	glLightfv(GL_LIGHT0, GL_SPECULAR, white_light);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
	glEnable(GL_LIGHTING);	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHT0);	glEnable(GL_DEPTH_TEST);
	
	
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
	const GLfloat light_ambient[] = { 0.1f, 0.1f, 0.1f, 0.0f };
	const GLfloat light_diffuse[] = { 0.0f, 0.0, 0.0, 0.0f };
	const GLfloat light_specular[] = { 0.2f, 0.2f, 0.f, 0.5f };
	const GLfloat light_position1[] = { 1.0f, .0f, 0.0f, 0.0f };
	const GLfloat light_shininess[] = { 2.0f };
	const GLfloat light_emissive[] = { 0, 0, 0, 0 };
	
	glEnable(GL_COLOR_MATERIAL);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_POSITION, light_position1);
	glLightfv(GL_LIGHT1, GL_EMISSION, light_emissive);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glMaterialfv(GL_FRONT, GL_SPECULAR, light_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, light_shininess); 
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
	glEnable(GL_LIGHT1);glEnable(GL_DEPTH_TEST);

}
#define MAX_CHAR        128
void drawString(const char* str) {    //draw string through OpenGL

	static int isFirstCall = 1;
	static GLuint lists;
	if (isFirstCall) {
		isFirstCall = 0;



		lists = glGenLists(MAX_CHAR);
		wglUseFontBitmaps(wglGetCurrentDC(), 0, MAX_CHAR, lists);
	}
	for (; *str != '\0'; ++str)
		glCallList(lists + *str);
}
void drawaxis(float r, float g, float b) {
	glColor3f(r, g, b);
	glPushMatrix();
	glTranslatef(0, 0, l); //move cone 10 units
	glutSolidCone(0.2, 1.5, 10, min); //bottom = 0, top = 1
	glPopMatrix();
	glutSolidCylinder(0.1, l, 10, min);
}
void drawCoordinate(float size) {  // draws a cube with side length = size

	glPushMatrix();
	glScalef(size, size, size); // scale unit axis to desired size

	drawaxis(0.1, 0.2, 0.7); // blue z axis

	glPushMatrix();
	glRotatef(-90, 1, 0, 0);//minus ->clockwise
	drawaxis(0.1, 0.7, 0.2); // green y axis
	glPopMatrix();

	glPushMatrix();
	glRotatef(90, 0, 1, 0);
	drawaxis(0.7, 0.2, 0.1); // red x axis
	glPopMatrix();

	glPopMatrix(); // Restore matrix to its state before cube() was called.

}
void drawreferenceaixs() {

	glColor3f(0.5, 0.5, 0.5);
	for (int i = 0; i <= 20; i++) {
		if (i == 0 || i == 20)glLineWidth(2.f);
		glBegin(GL_LINES);
		glVertex3f(10 - i, -10, 0);
		glVertex3f(10 - i, 10, 0);
		glVertex3f(10, 10 - i, 0);
		glVertex3f(-10, 10 - i, 0);
		glEnd();
		glLineWidth(0.2f);
	}
}
void Drawboundingbox() {

	//-------------------------Draw Bounding box----------------------------

	glColor3f(0.7,0.2,0.2);
	glLineWidth(3.5);

	glBegin(GL_LINE_LOOP);
	glVertex3f(xmax, ymin, zmax);
	glVertex3f(xmin, ymin, zmax);
	glVertex3f(xmin, ymax, zmax);
	glVertex3f(xmax, ymax, zmax);
	
	glEnd();

	
	glBegin(GL_LINE_LOOP);
	glVertex3f(xmax, ymin, zmin);
	glVertex3f(xmin, ymin, zmin);
	glVertex3f(xmin, ymax, zmin);
	glVertex3f(xmax, ymax, zmin);
	
	glEnd();

	
	glBegin(GL_LINES);
	glVertex3f(xmax, ymin, zmin);
	glVertex3f(xmax, ymin, zmax);

	glVertex3f(xmin, ymin, zmax);
	glVertex3f(xmin, ymin, zmin);

	glVertex3f(xmin, ymax, zmin);
	glVertex3f(xmin, ymax, zmax);

	glVertex3f(xmax, ymax, zmax);
	glVertex3f(xmax, ymax, zmin);

	glEnd();
}
void PointCloud() {
	HE_vert temp;

	glColor3f(0.2, 0.62, 0.5);
	glPointSize(1.0);

	for (int i = 0; i < numofVec; i++) {
		temp = HEvertList[i];
			glBegin(GL_POINTS);
			glVertex3f(temp.x, temp.y, temp.z);
			glEnd();
	}
}
void Wireframe() {
	HE_vert temp1,temp2,temp3;
	int a, b, c;

	glLineWidth(0.1);
	glColor3f(0.7, 0.3, 0.5);
	for (int i = 0; i < numofFace; i++) {
		a = faceList[i].a - 1;
		b = faceList[i].b - 1;
		c = faceList[i].c - 1;
		temp1 = HEvertList[a];
		temp2 = HEvertList[b];
		temp3 = HEvertList[c];
		

		glBegin(GL_LINE_LOOP);
		glVertex3f(temp1.x, temp1.y, temp1.z);
		glVertex3f(temp2.x, temp2.y, temp2.z);
		glVertex3f(temp3.x, temp3.y, temp3.z);
		glEnd();
	}
}
void DrawPolygonwithVN() {
	HE_vert temp1, temp2, temp3;
	
	int a, b, c;

	glColor3f(color[0],color[1],color[2]);
	for (int i = 0; i < numofFace; i++) {
		a = faceList[i].a-1;
		b = faceList[i].b-1;
		c = faceList[i].c-1;
		temp1 = HENvertList[a];
		temp2 = HENvertList[b];
		temp3 = HENvertList[c];

		glBegin(GL_TRIANGLES);
		glNormal3f(temp1.nx,temp1.ny,temp1.nz);
		glVertex3f(temp1.x, temp1.y, temp1.z);
		glNormal3f(temp2.nx, temp2.ny, temp2.nz);
		glVertex3f(temp2.x, temp2.y, temp2.z);
		glNormal3f(temp3.nx, temp3.ny, temp3.nz);
		glVertex3f(temp3.x, temp3.y, temp3.z);
		glEnd();
	}
}
void DrawPolygonwithFN() {
	HE_vert temp1, temp2, temp3;
	vector3f Ntemp;

	int a, b, c;

	glColor3f(color[0], color[1], color[2]);
	for (int i = 0; i < numofFace; i++) {
		a = faceList[i].a - 1;
		b = faceList[i].b - 1;
		c = faceList[i].c - 1;
		temp1 = HENvertList[a];
		temp2 = HENvertList[b];
		temp3 = HENvertList[c];
		Ntemp = NfaceList[i];
		glNormal3f(Ntemp.x, Ntemp.y, Ntemp.z);
		glBegin(GL_TRIANGLES);
	
		glVertex3f(temp1.x, temp1.y, temp1.z);

		glVertex3f(temp2.x, temp2.y, temp2.z);
	
		glVertex3f(temp3.x, temp3.y, temp3.z);
		glEnd();
	}
}
void DrawColorPolygon() {
	HE_vert temp1, temp2, temp3;

	int a, b, c;

	glColor3f(color[0], color[1], color[2]);
	for (int i = 0; i < numofFace; i++) {
		a = faceList[i].a - 1;
		b = faceList[i].b - 1;
		c = faceList[i].c - 1;
		temp1 = HENvertList[a];
		temp2 = HENvertList[b];
		temp3 = HENvertList[c];

		glBegin(GL_TRIANGLES);
		glColor3f(temp1.x/xmax, temp1.y / ymax, temp1.z / zmax );
		glNormal3f(temp1.nx, temp1.ny, temp1.nz);
		glVertex3f(temp1.x, temp1.y, temp1.z);
		glColor3f(temp2.x / xmax, temp2.y / ymax, temp2.z / zmax);
		glNormal3f(temp2.nx, temp2.ny, temp2.nz);
		glVertex3f(temp2.x, temp2.y, temp2.z);
		glColor3f(temp3.x / xmax, temp3.y / ymax, temp3.z / zmax);
		glNormal3f(temp3.nx, temp3.ny, temp3.nz);
		glVertex3f(temp3.x, temp3.y, temp3.z);
		glEnd();
	}

}

//-------------------GLUI SETUP-----------------------------------------------

int wireframe = 0;			
int polygon = 0;
int drawreference = 1;
int colorpolygon = 0;
int pointcloud = 0;
int boundingbox = 0;		//  Related to boundingBox		
int listbox_item_id = 12;	//  Id of the selected item in the list box
int drawobject_id = 0; //  Id of the selcted radio button
int projection_id = 0;
int orthognaol = 1;
int perspective = 0;
int rendering_id = 0;

int window_width = 750;
int window_height = 770;
int window_x;
int window_y;
int vx, vy, vw, vh;
void glui_callback(int arg);

enum
{
	DRAW_OBJECT= 0,
	PROJECTION,
	DISPLAYMODE,
	RENDERING_TYPE,
	COLOR_LISTBOX ,
	ROTATION,
	QUIT_BUTTON
};

enum GLUT_SHAPES
{
	BUNNY,
	EIGHT ,
	CAP,
	GARGPYLE,
	KONT= 0,
};


void idle()
{
	glutSetWindow(main_window);
	glutPostRedisplay();
	Sleep(50);
}


char const *v = s.c_str(); 
char const *f = g.c_str();

void gluisetup() {

	GLUI_Master.set_glutIdleFunc(idle);
	GLUI *glui_window = GLUI_Master.create_glui_subwindow(main_window, GLUI_SUBWINDOW_LEFT);
	glui_window->set_main_gfx_window(main_window);
	glui_window->add_separator();
	glui_window->add_separator();
	GLUI_Panel *projection_panel = glui_window->add_panel("Projection");
	GLUI_RadioGroup *group0 = glui_window->add_radiogroup_to_panel(projection_panel, &projection_id, PROJECTION, glui_callback);
	glui_window->add_radiobutton_to_group(group0, "Orthognaol");
	glui_window->add_radiobutton_to_group(group0, "Perspective");
	glui_window->add_separator();

	GLUI_Panel *obj_panel = glui_window->add_panel("Draw Objects");
	GLUI_RadioGroup *group1 = glui_window->add_radiogroup_to_panel(obj_panel,&drawobject_id, DRAW_OBJECT, glui_callback);
	glui_window->add_radiobutton_to_group(group1, "Bunny");
	glui_window->add_radiobutton_to_group(group1, "Eight");
	glui_window->add_radiobutton_to_group(group1, "Cap");
	glui_window->add_radiobutton_to_group(group1, "Gargoyle");
	glui_window->add_radiobutton_to_group(group1, "Knot");
	group1->set_int_val(4);
	glui_window->add_separator();
	glui_window->add_statictext(" ");
	glui_window->add_separator();
	//glui_window->add_column(true);

	GLUI_Panel *display_panel = glui_window->add_panel("Display Modes");	
	glui_window->add_checkbox_to_panel(display_panel, "Coordinate", &drawreference);
	glui_window->add_checkbox_to_panel(display_panel, "Points Cloud",&pointcloud);

	glui_window->add_checkbox_to_panel(display_panel, "Wireframe", &wireframe);
	glui_window->add_checkbox_to_panel(display_panel, "Polygons ",&polygon);
	glui_window->add_checkbox_to_panel(display_panel, "Colorfull Polygons ", &colorpolygon);
	glui_window->add_checkbox_to_panel(display_panel, "BoundingBox ", &boundingbox);
	glui_window->add_separator_to_panel(display_panel);
	GLUI_Panel *render_panel = glui_window->add_panel_to_panel(display_panel,"Rendering Type");
	GLUI_RadioGroup *group2 =glui_window->add_radiogroup_to_panel(render_panel, &rendering_id, RENDERING_TYPE, glui_callback);
	glui_window->add_radiobutton_to_group(group2, "Smooth Shading");
	glui_window->add_radiobutton_to_group(group2, "Flat Shaidng");

	glui_window->add_separator();
	glui_window->add_statictext(" ");

	//glui_window->add_column(true);
	GLUI_Panel *op_panel = glui_window->add_panel("Object Properties");
	GLUI_Listbox *color_listbox = glui_window->add_listbox_to_panel(op_panel,
		"Color", &listbox_item_id, COLOR_LISTBOX, glui_callback);

	//  Add the items to the listbox
	color_listbox->add_item(1, "Black");
	color_listbox->add_item(2, "Blue");
	color_listbox->add_item(3, "Cyan");
	color_listbox->add_item(4, "Dark Grey");
	color_listbox->add_item(5, "Grey");
	color_listbox->add_item(6, "Green");
	color_listbox->add_item(7, "Light Grey");
	color_listbox->add_item(8, "Magenta");
	color_listbox->add_item(9, "Orange");
	color_listbox->add_item(10, "Pink");
	color_listbox->add_item(11, "Red");
	color_listbox->add_item(12, "White");
	color_listbox->add_item(13, "Yellow");
	//glui_window->add_column(true);
	//  Select the White Color by default
	color_listbox->set_int_val(3);
	glui_window->add_separator();//------------------------------------------------
	glui_window->add_separator();
	
	
	GLUI_Panel *data_panel = glui_window->add_panel("Data for Current Model");
	glui_window->add_statictext_to_panel(data_panel, "Number of Vertex");
	glui_window->add_statictext_to_panel(data_panel, v);
	glui_window->add_statictext_to_panel(data_panel, "Number of Vertex");
	glui_window->add_statictext_to_panel(data_panel, f);

	glui_window->add_separator();
	//------------------------------
	glui_window->add_separator();

	glui_window->add_rotation("Rotation");

	glui_window->add_statictext(" ");//----------------------------------------
	glui_window->add_separator();

	glui_window->add_button("Quit", 0, (GLUI_Update_CB)exit);

}
void reshape(int w, int h)
{
	window_width = w;
	window_height = h;
	GLUI_Master.get_viewport_area(&vx, &vy, &vw, &vh);
	glViewport(vx, vy, vw, vh);
	glutPostRedisplay();
	printf("Window Width: %d, Window Height: %d.\n", window_width, window_height);
	printf("Drawing Area: x: %d, y: %d, width: %d, height: %d.\n", vx, vy, vw, vh);
}
void glui_callback(int control_id) {
	switch (control_id)
		{
	case PROJECTION:
		printf("Projection Modes %d selected.\n", projection_id);
		
		break;
		
	case DRAW_OBJECT: 
		
			printf("Draw Object %d selected.\n", drawobject_id);
			/*switch (drawobject_id) {
			case 0:filename = "bunny.m"; break;
			case 1:filename = "eight.m"; break;
			case 2:filename = "cap.m"; break;
			case 3:filename = "gargoyle.m"; break;
			case 4:filename = "knot.m"; break;
			}*/
			break;
		
	case COLOR_LISTBOX:

		printf("Color List box item changed: ");
		/*
		switch (listbox_item_id)
		{
		case 0:
			color[0] = 0.0;
			color[1] = 0.0;
			color[2] = 0.0;
			break;
		case 1:
			color[0] = 0.0;
			color[1] = 0.0;
			color[2] = 1.0;
			break;
		case 2:
			color[0] = 0.0;
			color[1] = 1.0;
			color[2] = 1.0;
			break;
		case 3:
			color[0] = 64 / 255.0;
			color[1] = 64 / 255.0;
			color[2] = 64 / 255.0;
			break;
		case 4:
			color[0] = 128 / 255.0;
			color[1] = 128 / 255.0;
			color[2] = 128 / 255.0;
			break;
		case 5:
			color[0] = 0 / 255.0;
			color[1] = 255 / 255.0;
			color[2] = 0 / 255.0;
			break;
		case 6:
			color[0] = 192 / 255.0;
			color[1] = 192 / 255.0;
			color[2] = 192 / 255.0;
			break;
		case 7:
			color[0] = 192 / 255.0;
			color[1] = 64 / 255.0;
			color[2] = 192 / 255.0;
			break;
		case 8:
			color[0] = 255 / 255.0;
			color[1] = 192 / 255.0;
			color[2] = 64 / 255.0;
			break;
		case 9:
			color[0] = 255 / 255.0;
			color[1] = 0 / 255.0;
			color[2] = 255 / 255.0;
			break;
		case 10:
			color[0] = 255 / 255.0;
			color[1] = 0 / 255.0;
			color[2] = 0 / 255.0;
			break;
		case 11:
			color[0] = 255 / 255.0;
			color[1] = 255 / 255.0;
			color[2] = 255 / 255.0;
			break;
		case 12:
			color[0] = 255 / 255.0;
			color[1] = 255 / 255.0;
			color[2] = 0 / 255.0;
			break;
		}

		//printf("Item %d selected.\n", listbox_item_id);
		*/
		break;
	}
}

//-------------------GLUI SETUP END-----------------------------------------------

//---------------------------From Demo-----------------------------------------------

void mymouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
	{
		press_x = x; press_y = y;
		if (button == GLUT_LEFT_BUTTON)
			xform_mode = TRANSFORM_ROTATE;
		else {
			if (button == GLUT_RIGHT_BUTTON)
				xform_mode = TRANSFORM_SCALE;
			else if(button == GLUT_MIDDLE_BUTTON) 
				xform_mode = TRANSFORM_TRANSLATE;	
		}
	}
	else if (state == GLUT_UP)
	{
		xform_mode = TRANSFORM_NONE;
	}
}

void mymotion(int x, int y)
{
	if (xform_mode == TRANSFORM_ROTATE)
	{
		x_angle += (x - press_x) / 5.0;

		if (x_angle > 180)
			x_angle -= 360;
		else if (x_angle <-180)
			x_angle += 360;

		press_x = x;

		y_angle += (y - press_y) / 5.0;

		if (y_angle > 180)
			y_angle -= 360;
		else if (y_angle <-180)
			y_angle += 360;

		press_y = y;
	}
	else {
		if (xform_mode == TRANSFORM_SCALE)
		{
			float old_size = scale_size;

			scale_size *= (1 + (y - press_y) / 60.0);

			if (scale_size < 0)
				scale_size = old_size;
			press_y = y;
		}
		else if (xform_mode == TRANSFORM_TRANSLATE) {
			x_translate += (x - press_x)/15;	
			press_x = x;

			y_translate += (y - press_y)/15;
			press_y = y;
		}

	}
	// force the redraw function
	glutPostRedisplay();
}

void display() {
	// called when the display needs to be drawn
	glMatrixMode(GL_MODELVIEW);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();             // Set up modelview transform. 
	if (projection_id == 0) {
		glOrtho(-10, 10, -10, 10, -15, 15);
		gluLookAt(ex, ey, ez, 0, 0, 0, upx, upy, upz);
	}
	
	else {
		gluPerspective(60, 1, 0.5, 40);
		gluLookAt(12, 12, 12, 0, 0, 0, upz, upy, upz);
		//Strengthenlight
		GLfloat light_position[] = {10, 10, 10, 1.0 }; // light position
		GLfloat white_light[] = { 1.0, 1.0, 1.0, 1.0 }; // light color
		GLfloat lmodel_ambient[] = { 1.0, 1.0, 1.0, 0.5};

		glLightfv(GL_LIGHT2, GL_POSITION, light_position);
		glLightfv(GL_LIGHT2, GL_DIFFUSE, white_light);
		glLightfv(GL_LIGHT2, GL_SPECULAR, white_light);
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, lmodel_ambient);
		glEnable(GL_LIGHTING);	glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHT2);	glEnable(GL_DEPTH_TEST);
	}

	glRotatef(x_angle, 0, 1, 0);
	glRotatef(y_angle, 1, 0, 0);
	glScalef(scale_size, scale_size, scale_size);
	glTranslatef(x_translate,-y_translate,0);
	if (drawreference) {
	drawreferenceaixs();
	drawCoordinate(1);}
	
	

	if (rendering_id == 0 && polygon) {
		DrawPolygonwithVN();
		glShadeModel(GLU_SMOOTH);
}
	else if (rendering_id == 1 && polygon) {
		DrawPolygonwithFN();
		glShadeModel(GLU_FLAT);
	}
		
	if (wireframe) Wireframe();
	if (pointcloud) PointCloud();
	if (boundingbox) Drawboundingbox();
	if (colorpolygon) DrawColorPolygon();
	
	glutSwapBuffers();
	
}

//----------------------------------------------------------------------------------
void main(int argc, char** argv)
{
	filename = "knot.m";
	readfile(filename);
	OneRingTraversal();
	vertexnormal();

	glutInit(&argc, argv);            // initialize GLUT
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB| GLUT_DEPTH);  // set display mode
	glutInitWindowSize(window_width, window_height);                 // set display window width and height
	glutInitWindowPosition(window_x, window_y);             // set top-left display window position
	main_window = glutCreateWindow("3D Mesh Viewer     Hu Xinjie G1601277L");
	InitGL();      
	gluisetup();	

							
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mymouse);
	glutMotionFunc(mymotion);
	

	glutMainLoop();

}
