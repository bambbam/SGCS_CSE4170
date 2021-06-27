#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <ctime>
#include <set>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "Shaders/LoadShaders.h"
#include <vector>
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

// include glm/*.hpp only if necessary
//#include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f
#define BUFFER_OFFSET(offset) ((GLvoid *) (offset))

#define LOC_VERTEX 0

int win_width = 0, win_height = 0;
float centerx = 0.0f, centery = 0.0f;


// 2D 물체 정의 부분은 objects.h 파일로 분리
// 새로운 물체 추가 시 prepare_scene() 함수에서 해당 물체에 대한 prepare_***() 함수를 수행함.
// (필수는 아니나 올바른 코딩을 위하여) cleanup() 함수에서 해당 resource를 free 시킴.
#include "objects.h"
#include "my_header.h"
player_object player;
bool start_move = false;;


unsigned int timestamp = 0;
void timer(int value) {
	timestamp = (timestamp + 1) % UINT_MAX;
	glutPostRedisplay();
	glutTimerFunc(5, timer, 0);
}


void draw_player(void) {
	glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(player.pos, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(player.scale, player.scale, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, player.degree * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));

	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_airplane();
}

void draw_car2_object(OBJECT* cur) {
	glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cur->pos, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, (cur->object_degree) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_car2();
}

void draw_sword_object(OBJECT* cur) {
	glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cur->pos, 0.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(cur->scale, cur->scale, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, (cur->object_degree) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_sword();
}
house houses[4];
void draw_house_object() {
	for (int i = 0; i < 4; i++) {
		houses[i].draw();
	}
}

float velocity[] = { 2 , 3 };
void (*fcnPtr[2])(OBJECT*) = { draw_car2_object , draw_sword_object };
std::set<OBJECT*> objects;
std::set< OUT_OF_TIME_OBJECT*> dead_object;
void make_object() {
	OBJECT* a = new OBJECT;
	a->object_number = 0;//rand() % 4;
	a->move_degree = (rand() % 360) - 360;

	if (a->move_degree < 45 && a->move_degree >= -45) {
		a->pos.x = -square_size, a->pos.y = (rand() % 300) - square_size;
	}
	else if (a->move_degree >= 45 && a->move_degree < 135) {
		a->pos.x = (rand() % (int)square_size) - square_size; a->pos.y = -square_size;
	}
	else if (a->move_degree >= -135 && a->move_degree <= -45) {
		a->pos.x = (rand() % (int)square_size) - square_size; a->pos.y = square_size;
	}
	else {
		a->pos.x = square_size, a->pos.y = (rand() % (int)square_size) - square_size;
	}
	a->velocity = velocity[a->object_number];

	objects.insert(a);
}

bool time_to_draw_sword = false;

void delete_out_of_range() {
	std::vector<OBJECT*> v;
	for (auto ob : objects) {
		if (ob->pos.x > square_size || ob->pos.y > square_size || ob->pos.x < -square_size || ob->pos.y < -square_size) {
			v.push_back(ob);
		}
	}
	for (auto ob : v) {
		if (objects.erase(ob)) {
			delete ob;
		}
	}
}
float distance2(glm::vec2 pos1, glm::vec2 pos2) {
	return (pos1.x - pos2.x) * (pos1.x - pos2.x) + (pos1.y - pos2.y) * (pos1.y - pos2.y);
}



void delete_intersect() {
	std::vector<OBJECT*> v;
	for (OBJECT* ob : objects) {
		if (ob->object_number == 1) {
			int ch = 0;
			for (OBJECT* j : objects) {
				if (j->object_number == 0) {
					float f = distance2(ob->pos, j->pos);
					if (f < 500 * ob->scale * sqrt(ob->scale)) { v.push_back(j); ch++; }
				}
			}
			if (ch && ob->scale < 4) v.push_back(ob);
		}

	}
	for (OBJECT* ob : v) {
		if (objects.erase(ob)) {
			delete ob;
		}
	}
}
void make_out_of_time_object() {
	std::vector<OBJECT*> v;
	for (OBJECT* ob : objects) {
		if (ob->object_number != 1) {
			if (ob->is_dead()) {
				v.push_back(ob);
			}
		}
	}
	for (OBJECT* ob : v) {

		if (objects.erase(ob)) {
			dead_object.insert(new OUT_OF_TIME_OBJECT(ob));
			delete(ob);
		}
	}

	std::vector<OUT_OF_TIME_OBJECT*> vv;
	for (OUT_OF_TIME_OBJECT* ob : dead_object) {
		if (ob->is_dead()) {
			vv.push_back(ob);
		}
	}
	for (OUT_OF_TIME_OBJECT* ob : vv) {
		if (dead_object.erase(ob)) {
			int i;
			for (i = 0; i < 4; i++) {
				if (abs(ob->dest.x - houses[i].pos.x) <= 1 &&
					abs(ob->dest.y - houses[i].pos.y) <= 1) {
					break;
				}
			}
			houses[i].num_car++;
			delete(ob);
		}
	}

}
void draw_out_of_time() {
	for (OUT_OF_TIME_OBJECT* ob : dead_object) {
		ob->draw();
	}
}
hat hat1, hat2;
int right_hand_timer = 0;
void draw_human_object(int timer) {
	glm::mat4  ModelMatrix = glm::mat4(1.0f);

	//prepare_human();
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-500, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30, 30, 1));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	if ((timer / 100) % 2 == 0) {
		draw_human();
	}
	else {
		draw_human2();

	}
	if (!right_hand_up)
		draw_right_hand();
	else {
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-500, 80, 0));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30, -30, 1));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_right_hand();
	}
	draw_right_hand();
	hat1.draw();
	hat2.draw();
	if ((timer / 100) % 2 == 0) {
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-410, 40, 0));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2, 2, 1));
		ModelMatrix = glm::rotate(ModelMatrix, -45 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	}
	else {
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(-420, 60, 0));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2, 2, 1));
		ModelMatrix = glm::rotate(ModelMatrix, -30 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	}
	draw_sword();
	ModelMatrix = glm::mat4(1.0f);
	ModelMatrix = glm::translate(ModelMatrix, glm::vec3(500, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(30, 30, 1));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(-1, 1, 1));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	if ((timer / 100) % 2 == 0) {
		draw_human();
	}
	else {
		draw_human2();
	}
	if (!right_hand_up)
		draw_right_hand();
	else {
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(500, 80, 0));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(-30, -30, 1));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_right_hand();
		right_hand_timer++;
		if (right_hand_timer == 150) {
			right_hand_timer = 0;
			right_hand_up = false;
		}
	}
	hat1.draw();
	hat2.draw();
	if ((timer / 100) % 2 == 0) {
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(410, 40, 0));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2, 2, 1));
		ModelMatrix = glm::rotate(ModelMatrix, 45 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_sword();
	}
	else {
		ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(420, 60, 0));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(2, 2, 1));
		ModelMatrix = glm::rotate(ModelMatrix, 30 * TO_RADIAN, glm::vec3(0, 0, 1));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_sword();
	}

}

void draw_switching_hat() {
	hat1.move();
	hat2.move();
}

int mouseX, mouseY;
float sword_scale;

void display(void) {

	if (timestamp % 120 == 0) {
		make_object();
	}
	player.speed = sqrt((float(mouseY) - player.pos.y) * (float(mouseY) - player.pos.y) + (float(mouseX) - player.pos.x) * (float(mouseX) - player.pos.x)) / 50;
	glm::mat4 ModelMatrix;
	glClear(GL_COLOR_BUFFER_BIT);
	ModelMatrix = glm::mat4(1.0f);
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_axes();

	draw_house_object();

	draw_human_object(timestamp);

	if (switch_hat) {
		draw_switching_hat();
	}

	int move_speed = player.speed;
	if (start_move) {
		if (player.pos.x + move_speed * sinf(player.degree * TO_RADIAN) >= square_size ||
			player.pos.x + move_speed * sinf(player.degree * TO_RADIAN) <= -square_size) {
			player.pos.x = -player.pos.x;
		}
		if (player.pos.y - move_speed * cosf(player.degree * TO_RADIAN) >= square_size ||
			player.pos.y - move_speed * cosf(player.degree * TO_RADIAN) <= -square_size) {
			player.pos.y = -player.pos.y;
		}
		player.pos.x += move_speed * sinf(player.degree * TO_RADIAN);
		player.pos.y -= move_speed * cosf(player.degree * TO_RADIAN);
	}

	for (auto ob : objects) {
		fcnPtr[ob->object_number](ob);
		ob->move();
		ob->add_timer();
	}
	delete_out_of_range();
	delete_intersect();
	make_out_of_time_object();
	draw_player();
	if (time_to_draw_sword) {
		OBJECT* sword = new OBJECT;
		sword->move_degree = player.degree - 90;
		sword->object_number = 1;
		sword->pos = player.pos;
		sword->velocity = velocity[sword->object_number];
		sword->object_degree = player.degree - 180;
		sword->scale = 1 + sword_scale / 50.0f;
		objects.insert(sword);
		time_to_draw_sword = false;
	}
	draw_out_of_time();
	glFlush();
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	case 32:
		switch_hat = true;
		break;
	}


}
void special_keyboard(int key, int x, int y) {
	return;
}
int leftbuttonpressed = 0;

void mouse_movement(int x, int y) {

	x = x - win_width / 2;
	y = -y + win_height / 2;
	mouseX = x, mouseY = y;
	float degree = atan2f(float(y) - player.pos.y, float(x) - player.pos.x);

	player.degree = degree * TO_DEGREE + 90;
}

void mouse(int button, int state, int x, int y) {
	mouse_movement(x, y);
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		start_move = true;
		if (leftbuttonpressed == 0) leftbuttonpressed = timestamp;
	}
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {
		if (leftbuttonpressed != 0) {
			time_to_draw_sword = true;
			sword_scale = timestamp - leftbuttonpressed;
		}
		leftbuttonpressed = 0;
	}
}

void motion(int x, int y) {
	centerx = x - win_width / 2.0f, centery = (win_height - y) - win_height / 2.0f;
	mouse_movement(x, y);
}

void reshape(int width, int height) {
	win_width = width, win_height = height;

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0,
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	//update_axes();
	update_square(square_size);
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	glDeleteVertexArrays(1, &VAO_house);
	glDeleteBuffers(1, &VBO_house);
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(reshape);
	glutTimerFunc(10, timer, 0);
	glutCloseFunc(cleanup);
	glutSpecialFunc(special_keyboard);
	glutPassiveMotionFunc(mouse_movement);
}

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(255 / 255.0f, 255 / 255.0f, 255 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);

	player.pos.x = 0.0f, player.pos.y = 0.0f;
	player.degree = 0.0f; player.scale = 1.0f;
	square_size = 300.0f;
	hat1.pos = glm::vec2(-500, 80);
	hat1.hat_version = 1;
	hat1.num = 0;
	hat2.pos = glm::vec2(500, 80);
	hat2.hat_version = 2;
	hat2.num = 1000;
	for (int i = 0; i < 4; i++) {
		houses[i].pos = to_go[i];
	}
}

void prepare_scene(void) {
	prepare_human();
	prepare_square(square_size);
	prepare_airplane();
	prepare_house();
	prepare_car2();
	prepare_sword();
	prepare_hat();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 2
int main(int argc, char* argv[]) {
	srand(time(NULL));

	char program_name[64] = "Sogang CSE4170 Simple2DTransformationMotion_GLSL_3.0.3";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC' 'SPACE BAR'\n"
		"    - Mouse used: L-click  for shoot, mouse move for direction change"
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


