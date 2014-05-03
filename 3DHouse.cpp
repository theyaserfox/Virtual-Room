//Frank Fodera
//3DHouse.cpp
//This program creates a museum with random pictures including two rooms.

#include "3DHouse.h"

bool load_TGA(TextureImage *texture, char *filename)							// Loads A TGA File Into Memory
{    
	GLubyte		TGAheader[12]={0,0,2,0,0,0,0,0,0,0,0,0};					// Uncompressed TGA Header
	GLubyte		TGAcompare[12];												// Used To Compare TGA Header
	GLubyte		header[6];													// First 6 Useful Bytes From The Header
	GLuint		bytesPerPixel;												// Holds Number Of Bytes Per Pixel Used In The TGA File
	GLuint		imageSize;													// Used To Store The Image Size When Setting Aside Ram
	GLuint		temp;														// Temporary Variable
	GLuint		type=GL_RGBA;												// Set The Default GL Mode To RBGA (32 BPP)

	FILE *file = fopen(filename, "rb");										// Open The TGA File

	if(	file==NULL ||														// Does File Even Exist?
		fread(TGAcompare,1,sizeof(TGAcompare),file)!=sizeof(TGAcompare) ||	// Are There 12 Bytes To Read?
		memcmp(TGAheader,TGAcompare,sizeof(TGAheader))!=0				||	// Does The Header Match What We Want?
		fread(header,1,sizeof(header),file)!=sizeof(header))				// If So Read Next 6 Header Bytes
	{
		if (file == NULL)													// Did The File Even Exist? *Added Jim Strong*
			return FALSE;													// Return False
		else																// Otherwise
		{
			fclose(file);													// If Anything Failed, Close The File
			return FALSE;													// Return False
		}
	}

	texture->width  = header[1] * 256 + header[0];							// Determine The TGA Width	(highbyte*256+lowbyte)
	texture->height = header[3] * 256 + header[2];							// Determine The TGA Height	(highbyte*256+lowbyte)

	if(	texture->width	<=0	||												// Is The Width Less Than Or Equal To Zero
		texture->height	<=0	||												// Is The Height Less Than Or Equal To Zero
		(header[4]!=24 && header[4]!=32))									// Is The TGA 24 or 32 Bit?
	{
		fclose(file);														// If Anything Failed, Close The File
		return FALSE;														// Return False
	}

	texture->bpp	= header[4];											// Grab The TGA's Bits Per Pixel (24 or 32)
	bytesPerPixel	= texture->bpp/8;										// Divide By 8 To Get The Bytes Per Pixel
	imageSize		= texture->width*texture->height*bytesPerPixel;			// Calculate The Memory Required For The TGA Data

	texture->imageData=(GLubyte *)malloc(imageSize);						// Reserve Memory To Hold The TGA Data

	if(	texture->imageData==NULL ||											// Does The Storage Memory Exist?
		fread(texture->imageData, 1, imageSize, file)!=imageSize)			// Does The Image Size Match The Memory Reserved?
	{
		if(texture->imageData!=NULL)										// Was Image Data Loaded
			free(texture->imageData);										// If So, Release The Image Data

		fclose(file);														// Close The File
		return FALSE;														// Return False
	}

	for(GLuint i=0; i<int(imageSize); i+=bytesPerPixel)						// Loop Through The Image Data
	{																		// Swaps The 1st And 3rd Bytes ('R'ed and 'B'lue)
		temp=texture->imageData[i];											// Temporarily Store The Value At Image Data 'i'
		texture->imageData[i] = texture->imageData[i + 2];					// Set The 1st Byte To The Value Of The 3rd Byte
		texture->imageData[i + 2] = temp;									// Set The 3rd Byte To The Value In 'temp' (1st Byte Value)
	}

	fclose (file);															// Close The File

	// Build A Texture From The Data
	glGenTextures(1, &texture[0].texID);									// Generate OpenGL texture IDs
	glBindTexture(GL_TEXTURE_2D, texture[0].texID);							// Bind Our Texture
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);		// Linear Filtered
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);		// Linear Filtered

	if (texture[0].bpp==24)													// Was The TGA 24 Bits
	{
		type=GL_RGB;														// If So Set The 'type' To GL_RGB
	}

	glTexImage2D(GL_TEXTURE_2D, 0, type, texture[0].width, texture[0].height, 0, type, GL_UNSIGNED_BYTE, texture[0].imageData);

	return true;															// Texture Building Went Ok, Return True
}

void readstr(FILE* f, char* string)											// Function Used to read strings from a file
{
	do 
	{
		fgets(string, 255, f);
	} while ((string[0] == '/') || (string[0] == '\n'));
}

void setup_world()															// Sets up the world using the world.txt file
{
	float x, y, z, u, v;
	int numpolygons;
	int texid;
	FILE *filein;
	char oneline[255];
	filein = fopen("Data/world.txt", "rt");									// File To Load World Data From

	readstr(filein, oneline);
	sscanf(oneline, "NUMPOLLIES %d\n", &numpolygons);						// The number of polygons that we have
	g_sector1.polygon = new POLYGON[numpolygons];							// Create the array of polygons using the number of polygons input from the text file
	g_sector1.texture = new int[numpolygons];								// Create the number of textures, one per polygon
	g_sector1.numpolygons = numpolygons;									

	for (int loop = 0; loop < numpolygons; loop++) 
	{
		readstr(filein, oneline);											// Reads from the open file
		sscanf(oneline,"TEXTURE %d\n", &texid);								// Scans the file until it finds TEXTURE with an ID
		for (int vert = 0; vert < 4; vert++) 
		{
			g_sector1.texture[loop] = texid;								// Stores the texture id in an array to be used later
			readstr(filein, oneline);										// Reads from the open file
			sscanf(oneline, "%f %f %f %f %f %f %f", &x, &y, &z, &u, &v);	// Scans until it finds a series of floats and inputs them into x y z u v
			g_sector1.polygon[loop].vertex[vert].x = x;
			g_sector1.polygon[loop].vertex[vert].y = y;
			g_sector1.polygon[loop].vertex[vert].z = z;
			g_sector1.polygon[loop].vertex[vert].u = u;
			g_sector1.polygon[loop].vertex[vert].v = v;
		}
	}
	fclose(filein);
	return;
}

bool setup_textures()												// Setup Our Textures. Returns true On Success, false On Fail
{
	if (!load_TGA(&textures[0],"Data/tiled.tga")					// Create The Textures' Id List
		|| !load_TGA(&textures[1],"Data/ceiling.tga")
		|| !load_TGA(&textures[2],"Data/painting1.tga")
		|| !load_TGA(&textures[3],"Data/painting2.tga") 
		|| !load_TGA(&textures[4],"Data/painting3.tga") 
		|| !load_TGA(&textures[5],"Data/painting4.tga") 
		|| !load_TGA(&textures[7],"Data/painting6.tga")
		|| !load_TGA(&textures[6],"Data/painting5.tga") 
		|| !load_TGA(&textures[8],"Data/painting7.tga") 
		|| !load_TGA(&textures[9],"Data/painting8.tga") 
		|| !load_TGA(&textures[10],"Data/painting9.tga") 
		|| !load_TGA(&textures[11],"Data/graywall.tga") 
		|| !load_TGA(&textures[12],"Data/door.tga")
		|| !load_TGA(&textures[13],"Data/crosshairs.tga")
		|| !load_TGA(&textures[14],"Data/hardwood.tga")
		|| !load_TGA(&textures[15],"Data/stairsbottom.tga")
		|| !load_TGA(&textures[16],"Data/atticceiling.tga")
		|| !load_TGA(&textures[17],"Data/bluewall.tga")
		|| !load_TGA(&textures[18],"Data/railing.tga")
		|| !load_TGA(&textures[19],"Data/font.tga")
		|| !load_TGA(&textures[20],"Data/opendoor.tga")
		|| !load_TGA(&textures[21],"Data/simpledoor.tga")
		|| !load_TGA(&textures[22],"Data/column.tga"))		// End of texture lists 
		return false;												// We return false if one of the textures failed to load
	return true;													// If all textures loaded fine then we return true
}

GLvoid build_front(GLvoid)											// Build Our Font Display List
{
	base=glGenLists(95);											// Creating 95 Display Lists
	glBindTexture(GL_TEXTURE_2D, textures[19].texID);				// Bind Our Font Texture
	for (int loop=0; loop<95; loop++)								// Loop Through All 95 Lists
	{
		float cx=float(loop%16)/16.0f;								// X Position Of Current Character
		float cy=float(loop/16)/8.0f;								// Y Position Of Current Character
	
		glNewList(base+loop,GL_COMPILE);							// Start Building A List
		glBegin(GL_QUADS);											// Use A Quad For Each Character
		glTexCoord2f(cx,         1.0f-cy-0.120f); glVertex2i(0,0);	// Texture / Vertex Coord (Bottom Left)
		glTexCoord2f(cx+0.0625f, 1.0f-cy-0.120f); glVertex2i(16,0);	// Texutre / Vertex Coord (Bottom Right)
		glTexCoord2f(cx+0.0625f, 1.0f-cy);		  glVertex2i(16,16);// Texture / Vertex Coord (Top Right)
		glTexCoord2f(cx,         1.0f-cy);		  glVertex2i(0,16);	// Texture / Vertex Coord (Top Left)
		glEnd();													// Done Building Our Quad (Character)
		glTranslated(15,0,0);										// Move To The Right Of The Character
		glEndList();												// Done Building The Display List
	}																// Loop Until All 256 Are Built
}


bool init()															// Our GL Specific Initializations. Returns true On Success, false On Fail.
{
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	//glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	for(int i = 0; i < 3; i++)
		my_shape[i] = gluNewQuadric();
	setup_textures();
	build_front();
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);							// Pixel Storage Mode To Byte Alignment
	glEnable(GL_TEXTURE_2D);										// Enable Texture Mapping 
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);							// Black Background 
	glClearDepth(1.0f);												// Depth Buffer Setup
	glDepthFunc(GL_LESS);											// The Type Of Depth Testing To Do
	glEnable(GL_DEPTH_TEST);										// Enables Depth Testing
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);				// Enable Alpha Blending (disable alpha testing)
	glEnable(GL_BLEND);												// Enable Blending       (disable alpha testing)
	glAlphaFunc(GL_GREATER, 0.1f);
	glEnable(GL_ALPHA_TEST);
	glShadeModel(GL_SMOOTH);										// Enable Smooth Shading
	setup_world();
	memset(g_key, 0, sizeof(g_key));
	return true;
}

void reshape(int w, int h)											// Our Reshaping Handler (Required Even In Fullscreen-Only Modes)
{
	window_width = w;
	window_height = h;
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
	glLoadIdentity();												// Reset The Projection Matrix
	if (h == 0) h = 1;
	gluPerspective(45.0f, (float)w/(float)h, 0.1, 100.0);
	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	glLoadIdentity();												// Reset The Modelview Matrix
}

GLvoid gl_print(GLint x, GLint y, const char *string, ...)			// Where The Printing Happens
{
	char		text[256];											// Holds Our String
	va_list		ap;													// Pointer To List Of Arguments

	if (string == NULL)												// If There's No Text
		return;														// Do Nothing

	va_start(ap, string);											// Parses The String For Variables
	vsprintf(text, string, ap);										// And Converts Symbols To Actual Numbers
	va_end(ap);														// Results Are Stored In Text

	glBindTexture(GL_TEXTURE_2D, textures[19].texID);				// Select Our Font Texture
	glPushMatrix();													// Store The Modelview Matrix
	glLoadIdentity();												// Reset The Modelview Matrix
	glTranslated(x,y,0);											// Position The Text (0,0 - Bottom Left)
	glListBase(base-32);											// Choose The Font Set
	glCallLists(strlen(text), GL_UNSIGNED_BYTE, text);				// Draws The Display List Text
	glPopMatrix();													// Restore The Old Projection Matrix
}

void object(float width,float height,GLuint texid)					// Draw Object Using Requested Width, Height And Texture
{
	glBindTexture(GL_TEXTURE_2D, textures[texid].texID);			// Select The Correct Texture
	glBegin(GL_QUADS);												// Start Drawing A Quad
	glTexCoord2f(0.0f,0.0f); glVertex3f(-width,-height,0.0f);		// Bottom Left
	glTexCoord2f(1.0f,0.0f); glVertex3f( width,-height,0.0f);		// Bottom Right
	glTexCoord2f(1.0f,1.0f); glVertex3f( width, height,0.0f);		// Top Right
	glTexCoord2f(0.0f,1.0f); glVertex3f(-width, height,0.0f);		// Top Left
	glEnd();														// Done Drawing Quad
}


/*** Gemy function ***/
void texture_object(int polygonNum)									// Takes all of the textures in the world.txt file and creates them
{
	GLfloat x_m, y_m, z_m, u_m, v_m;

	glBindTexture(GL_TEXTURE_2D, textures[g_sector1.texture[polygonNum]].texID);
	glBegin(GL_POLYGON);
	glNormal3f(0.0f, 0.0f, 1.0f);
	x_m = g_sector1.polygon[polygonNum].vertex[0].x;
	y_m = g_sector1.polygon[polygonNum].vertex[0].y;
	z_m = g_sector1.polygon[polygonNum].vertex[0].z;
	u_m = g_sector1.polygon[polygonNum].vertex[0].u;
	v_m = g_sector1.polygon[polygonNum].vertex[0].v;
	glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[1].x;
	y_m = g_sector1.polygon[polygonNum].vertex[1].y;
	z_m = g_sector1.polygon[polygonNum].vertex[1].z;
	u_m = g_sector1.polygon[polygonNum].vertex[1].u;
	v_m = g_sector1.polygon[polygonNum].vertex[1].v;
	glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[2].x;
	y_m = g_sector1.polygon[polygonNum].vertex[2].y;
	z_m = g_sector1.polygon[polygonNum].vertex[2].z;
	u_m = g_sector1.polygon[polygonNum].vertex[2].u;
	v_m = g_sector1.polygon[polygonNum].vertex[2].v;
	glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);

	x_m = g_sector1.polygon[polygonNum].vertex[3].x;
	y_m = g_sector1.polygon[polygonNum].vertex[3].y;
	z_m = g_sector1.polygon[polygonNum].vertex[3].z;
	u_m = g_sector1.polygon[polygonNum].vertex[3].u;
	v_m = g_sector1.polygon[polygonNum].vertex[3].v;
	glTexCoord2f(u_m,v_m); glVertex3f(x_m,y_m,z_m);
	glEnd();
}


/*** Gemy function ***/
void render(void)													// Our Rendering Is Done Here
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);				// Clear The Screen And The Depth Buffer
	glLoadIdentity();												// Reset The View

	GLfloat xtrans = -g_xpos;
	GLfloat ztrans = -g_zpos;
	GLfloat ytrans = -g_ypos;
	if(g_yrot > 360)
		g_yrot -= 360;
	else if(g_yrot < 0)
		g_yrot += 360;
	GLfloat sceneroty = (360.0f - g_yrot);


	int numpolygons;

	glRotatef(g_lookupdown,1.0f,0,0);
	glRotatef(sceneroty,0,1.0f,0);

	glTranslatef(xtrans, ytrans, ztrans);

	numpolygons = g_sector1.numpolygons;

	for (int loop_m = 0; loop_m < numpolygons; loop_m++)
		texture_object(loop_m);	

	gluQuadricDrawStyle(my_shape[0],GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, textures[1].texID);
	glScalef(0.1,0.1,0.1);
	glTranslatef(0.78,14.3,-4.2);
	gluSphere(my_shape[0], 1.0,50,50);

	gluQuadricDrawStyle(my_shape[1],GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, textures[8].texID);
	glTranslatef(-20,0,0);
	gluSphere(my_shape[1], 1.0,50,50);

	gluQuadricDrawStyle(my_shape[2],GLU_FILL);
	glBindTexture(GL_TEXTURE_2D, textures[22].texID);
	glTranslatef(40,0,0);
	gluSphere(my_shape[2], 1.0,50,50);

	glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
	glPushMatrix();													// Store The Projection Matrix
	glLoadIdentity();												// Reset The Projection Matrix

	glOrtho(-10,window_width,0,window_height,-10,10);					// Set Up An Ortho Screen
	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix
	//glTranslated(window_width/2,window_height/2,0.0f);				// Move To The Current Mouse Position
	//object(24,24,13);												// Draw The Crosshair

	gl_print(50,10,"X: %f", g_xpos);
	gl_print(250,10,"Y: %f", g_ypos);
	gl_print(450,10,"Z: %f", g_zpos);
	if(ENTER_WAS_PRESSED)
	{
		glDisable(GL_BLEND);												
		glDisable(GL_ALPHA_TEST);
		gl_print((window_width/2)-200,(window_height/2)+80,"Up Arrow: Move Forward");
		gl_print((window_width/2)-200,(window_height/2)+60,"Down Arrow: Move Backwards");
		gl_print((window_width/2)-200,(window_height/2)+40,"Left Arrow: Turn Left");
		gl_print((window_width/2)-200,(window_height/2)+20,"Right Arrow: Turn Right");
		gl_print((window_width/2)-200,(window_height/2),"Page Up: Look up");
		gl_print((window_width/2)-200,(window_height/2)-20,"Page Down: Look Down");
		gl_print((window_width/2)-200,(window_height/2)-40,"Spacebar: Jump");
		gl_print((window_width/2)-200,(window_height/2)-60,"Plus Sign: Speed Up Movement");
		gl_print((window_width/2)-200,(window_height/2)-80,"Minus Sign: Slow Down Movement");
		glEnable(GL_BLEND);												
		glEnable(GL_ALPHA_TEST);
	}
	if(g_zpos < -0.65 && g_xpos < 0.5 && g_xpos > -0.5 && g_ypos < 0.5)
		gl_print((window_width/2)-150,(window_height/2),"Press X to open door");

	glMatrixMode(GL_PROJECTION);									// Select The Projection Matrix
	glPopMatrix();													// Restore The Old Projection Matrix
	glMatrixMode(GL_MODELVIEW);										// Select The Modelview Matrix

	glutSwapBuffers ( );
}


/*** Gemy function ***/
void openDoor()														//This function is used to open the door
{
	if(!DOOR_IS_OPEN)
	{
		DOOR_IS_OPEN = true;
		for(int i = 0; i < 110; i++)
		{
			g_sector1.polygon[95].vertex[0].x = -0.18 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[95].vertex[1].x = -0.18 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[95].vertex[0].z = -1.999 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[95].vertex[1].z = -1.999 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7);
			render();
		}
	}
	else
	{
		DOOR_IS_OPEN = false;
		for(int i = 0; i < 110; i++)
		{
			g_sector1.polygon[95].vertex[0].x = 0.314 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[95].vertex[1].x = 0.314 - (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[95].vertex[0].z = -2.49 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			g_sector1.polygon[95].vertex[1].z = -2.49 + (0.373 - (0.373 * cos(i*PI_OVER_180)));
			Sleep(7);
			render();
		}
	}
}

void keyboard(unsigned char key, int x, int y)						// Our Keyboard Handler (Normal Keys)
{
	GLfloat jumpTop = g_ypos + 0.25;
	GLfloat jumpBottom = g_ypos + (speed_XYZ/2) ;
	switch (key) 
	{
		case ' ':
			if(g_zpos < -4.5 && g_xpos < 0.2 && g_xpos > -0.2)
				jumpBottom = 0.45 + (speed_XYZ/2);
			g_ypos += (speed_XYZ/4);
			while(g_ypos < jumpTop)
			{
				g_ypos += (speed_XYZ/2);
				render();
			}
			while(g_ypos > jumpBottom)
			{
				g_ypos -= (speed_XYZ/2);
				render();
			}
			g_ypos -= (speed_XYZ/4);
			break;
		case 'x':
		case 'X':
			if(g_zpos < -0.65 && g_xpos < 0.5 && g_xpos > -0.5 && g_ypos < 1.0)
				openDoor();
			break;
		case '-':
			if(speed_UDLR > 0.02 && speed_XYZ > 0.002)
			{
				speed_UDLR -= 0.01;
				speed_XYZ -= 0.001;
			}
			break;
		case '+':
			speed_UDLR += 0.01;
			speed_XYZ += 0.001;
			break;
		case 13:													//When enter is pressed display values
			if(!ENTER_WAS_PRESSED)
				ENTER_WAS_PRESSED = true;
			else
				ENTER_WAS_PRESSED = false;
			break;
		case 27:													// When Escape Is Pressed...
			exit(0);												// Exit The Program
			break;        
		default:
			break;
	}
	render();
}


void special_keys(int a_keys, int x, int y)							// Our Keyboard Handler For Special Keys (Like Arrow Keys And Function Keys)
{
	switch (a_keys) 
	{
		case GLUT_KEY_F1:
			if (!g_gamemode) 
			{
				g_fullscreen = !g_fullscreen;						// Toggle g_fullscreen Flag
				if (g_fullscreen) glutFullScreen();					// We Went In Fullscreen Mode
				else glutReshapeWindow(window_width, window_height);  // We Went In Windowed Mode
			}
			break;
		default:
			g_key[a_keys] = true;
			break;
	}
}

void special_keys_up(int key, int x, int y)							// Our Keyboard Handler For Special Key Releases.
{
	g_key[key] = false;
}

void game_function()												// Our Game Function. Check The User Input And Performs The Rendering
{
	//Process User Input
	if (g_key[GLUT_KEY_PAGE_UP]) 
	{
		g_z -= 0.002f;
		g_lookupdown -= speed_UDLR;
	}
	if (g_key[GLUT_KEY_PAGE_DOWN]) 
	{
		g_z += 0.002f;
		g_lookupdown += speed_UDLR;
	}
	if (g_key[GLUT_KEY_UP]) 
	{
		if(g_zpos < -2.5 && g_xpos < 0.2 && g_xpos > -0.2 && g_ypos < 1.46 && g_ypos >= 0.44 && g_zpos > -3.7)
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ/2);
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ/2);
			if(g_ypos < 1.45 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos += (3*speed_XYZ/5);
			else if(g_ypos >= 0.46) 
				g_ypos -= (3*speed_XYZ/5);
		}
		else
		{
			g_xpos -= (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
			g_zpos -= (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;

			if(!no_collision())
			{
				g_xpos += (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
				g_zpos += (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;
			}
		}
	}
	if (g_key[GLUT_KEY_DOWN]) 
	{
		if(g_zpos < -2.5 && g_xpos < 0.2 && g_xpos > -0.2 && g_ypos < 1.46 && g_ypos >= 0.44 && g_zpos > -3.7)
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * (speed_XYZ/2);
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * (speed_XYZ/2);
			if(g_ypos >= 0.46 && (g_yrot >= 270 || g_yrot <= 90))
				g_ypos -= (3*speed_XYZ/5);
			else if(g_ypos < 1.44)
				g_ypos += (3*speed_XYZ/5);
		}
		else
		{
			g_xpos += (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
			g_zpos += (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;

			if(!no_collision())
			{
				g_xpos -= (float)sin(g_yrot*PI_OVER_180) * speed_XYZ;
				g_zpos -= (float)cos(g_yrot*PI_OVER_180) * speed_XYZ;
			}
		}
	}
	if (g_key[GLUT_KEY_RIGHT]) 
		g_yrot -= speed_UDLR;
	if (g_key[GLUT_KEY_LEFT]) 
		g_yrot += speed_UDLR;
	//Sleep(1);
	render();														// Do The Rendering
}


bool no_collision()															//Detects the collision of walls, false if collision true if no collision
{
	if(g_zpos < 2.8 && g_zpos > -2.8 && g_xpos < 2.8 && g_xpos > -2.8 && g_ypos < 1.44)	//Check for a box around the downstairs
	{
		if(g_zpos < -1.8 && g_zpos > -2.2)										//Check for the inner box downstairs Front Wall
		{
			if(g_xpos < 0.1 && g_xpos > -0.1 && DOOR_IS_OPEN)					//Checks to see if we can make it through door and if it's open
				return true;
			else
				return false;
		}
		if(g_zpos <= -2.2)														//Check for the stairway
		{
			if(g_xpos < 0.2 && g_xpos > -0.2 )
				return true;
			else
				return false;
		}
		if(g_xpos < -1.8)														//Check for the inner box downstairs Left Wall
		{
			if(g_zpos < 0.35 && g_zpos > -0.35)
				return true;
			else
				return false;
		}
		if(g_xpos > 1.8)														//Check for the inner box downstairs Right Wall
		{
			if(g_zpos < 0.35 && g_zpos > -0.35)
				return true;
			else 
				return false;
		}
		if(g_zpos > 1.8)														//Check for the inner box downstairs Back Wall
		{
			if(g_xpos < 0.35 && g_xpos > -0.35)
				return true;
			else
				return false;
		}

		return true;
	}
	else if(g_ypos >= 1.44)															//Check to see if we are upstairs
	{
		if(g_zpos < 0.8 && g_xpos < 2.8 && g_xpos > -2.8 && g_zpos > -5.8)			//Check for the 4 walls upstairs
		{
			if(g_zpos > -0.70 && g_zpos < -0.15)										//Check for the 3 pillars
			{
				if(g_xpos > -2.20 && g_xpos < -1.65)									//Check for Left Pillar
					return false;
				else if(g_xpos < 2.35 && g_xpos > 1.80)								//Check for Right Pillar
					return false;
				else if(g_xpos > -0.2 && g_xpos < 0.35)								//Check for Middle Pillar
					return false;
				else 
					return true;
			}
			if(g_xpos < 0.35 && g_xpos > -0.35)										//Check for Inside railing
			{
				
				if(g_xpos < 0.2 && g_xpos > -0.2 && g_zpos > -4.2 && g_zpos < -2.0)
					return true;
				else if(g_zpos <= -4.2)
					return true;
				else if(g_zpos >= -2.0)
					if(g_zpos < -1.8)												//Check for back railing
						return false;
					else
						return true;
				else
					return false;
				return false;
			}
			if(g_zpos > -4.2 && g_zpos < -1.8)										//Check for outside of railing
			{
				if(g_xpos < -0.7)													//Check for left railing
					return true;
				if(g_xpos > 0.7)													//Check for right rialing
					return true;	
				else 
					return false;
			}
				return true;
		}
		else 
			return false;
		return true;
	}
	else if(g_zpos <= -2.8 && g_zpos > -5.8)															//Check for stairs now
	{
		if(g_xpos < 0.2 && g_xpos > -0.2)
			return true;
		else
			return false;
	}
	else 
		return false;
}


int main(int argc, char** argv)									// Main Function For Bringing It All Together.
{
	glutInit(&argc, argv);										// GLUT Initializtion
	glutInitDisplayMode(GLUT_DEPTH | GLUT_RGBA | GLUT_DOUBLE);	// (CHANGED)
	if (g_gamemode) 
	{
		glutGameModeString("640x480:16");						// Select The 640x480 In 16bpp Mode
		if (glutGameModeGet(GLUT_GAME_MODE_POSSIBLE))
			glutEnterGameMode();								// Enter Full Screen
		else g_gamemode = false;								// Cannot Enter Game Mode, Switch To Windowed
	}
	screen_width = glutGet(GLUT_SCREEN_WIDTH);
	screen_height = glutGet(GLUT_SCREEN_HEIGHT);
	window_width = screen_width/1.4;
	window_height = screen_height/1.4;

	if (!g_gamemode) 
	{
		glutInitWindowSize(window_width,window_height);           // Window Size If We Start In Windowed Mode
		glutInitWindowPosition((screen_width-window_width)/2,(screen_height-window_height)/2);
		glutCreateWindow("Frank's 3-D House");					// Window Title 
	}

	init();
	glutIgnoreKeyRepeat(true);									// Disable Auto Repeat (NEW)
	glutDisplayFunc(render);									// Register The Display Function
	glutReshapeFunc(reshape);									// Register The Reshape Handler
	glutKeyboardFunc(keyboard);									// Register The Keyboard Handler
	glutSpecialFunc(special_keys);								// Register Special Keys Handler
	glutSpecialUpFunc(special_keys_up);							// Called When A Special Key Released (NEW)
	glutIdleFunc(game_function);								// Process User Input And Does Rendering (CHANGED)
	glutMainLoop();												// Go To GLUT Main Loop
	return 0;
}
