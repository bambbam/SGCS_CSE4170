#pragma once
#include <glm/glm.hpp>
#include "objects.h"
#include <glm/gtc/matrix_transform.hpp> 


class player_object {
public:
	glm::vec2 pos;
	GLfloat degree;
	GLfloat scale;
	GLfloat speed;
	player_object() {
		pos.x = 0.0f, pos.y = 0.0f;
		degree = 0.0f;
		scale = 0.0f;
	}
	glm::vec2 get_pos() {
		return pos;
	}
};


float square_size;
int alive_time = 1000;

class OBJECT {
public:
	GLint object_number;
	glm::vec2 pos;
	GLfloat object_degree; //오브젝트가 빙그르르 돌았으면 좋겠다
	GLfloat move_degree; //그런데 어디론가 움직이는.
	GLfloat velocity; //이정도의 속도로 움직이는.
	GLfloat scale;
	int timer = 0;
	void move() {
		if (pos.x + velocity * cosf(move_degree * TO_RADIAN) > square_size ||
			pos.x + velocity * cosf(move_degree * TO_RADIAN) < -square_size) {
			if (object_number != 1) {
				move_degree = (180 - move_degree);
				if (move_degree >= 180) move_degree -= 360;
			}
		}
		if (pos.y + velocity * sinf(move_degree * TO_RADIAN) > square_size ||
			pos.y + velocity * sinf(move_degree * TO_RADIAN) < -square_size) {
			if (object_number != 1) {
				move_degree = -move_degree;
			}
		}
		pos.x += velocity * cosf(move_degree * TO_RADIAN);
		pos.y += velocity * sinf(move_degree * TO_RADIAN);
	}
	void add_timer() {
		timer++;
		timer %= alive_time * 2;
	}
	bool is_dead() {
		return timer >= alive_time;
	}
};
glm::vec2 to_go[4]{
	{250,250},{-250,250},{250,-250},{-250,-250}
};
class OUT_OF_TIME_OBJECT {
public:
	glm::vec2 start_pos;
	glm::vec2 pos;
	glm::vec2 dest;
	GLfloat velocity;
	int timer;
	GLfloat degree;
	bool ch = false;
	OUT_OF_TIME_OBJECT(OBJECT* x) {
		pos = x->pos;
		start_pos = pos;
		velocity = 2.0f;
		srand(time(NULL));
		dest = to_go[rand() % 4];
		degree = atan2f((dest.y - pos.y), (dest.x - pos.x)) * TO_DEGREE + 90;

		timer = 0;
	}
	void move() {
		if (ch || (pos.x - dest.x) * (pos.x - dest.x) + (pos.y - dest.y) * (pos.y - dest.y) < 200) {
			pos.x += 1 / 10.0f * ((10 * sinf(2 * degree * TO_RADIAN) + 6 * sinf(3 * degree * TO_RADIAN)));
			pos.y -= 1 / 10.0f * (6 * cosf(3 * degree * TO_RADIAN) - 10 * cos(2 * degree * TO_RADIAN));
			degree += 1;
		}
		else {
			pos.x += velocity * sinf(degree * TO_RADIAN);
			pos.y -= velocity * cosf(degree * TO_RADIAN);
		}
	}
	void draw() {
		if (ch || (pos.x - dest.x) * (pos.x - dest.x) + (pos.y - dest.y) * (pos.y - dest.y) < 200) {
			timer++;
			timer %= 2 * alive_time;
			move();
			ch = true;
			glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, (degree)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_car2();
		}
		else {
			move();
			glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, (degree)*TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_car2();
		}
	}
	bool is_dead() {
		return timer >= 1000;
	}
};
bool switch_hat = false;
bool right_hand_up = false;

class hat {
public:
	glm::vec2 pos;
	float degree = 0.0;
	int hat_version;
	int num;

	void draw() {
		glm::mat4 ModelMatrix = glm::mat4(1.0f);
		ModelMatrix = glm::translate(ModelMatrix, glm::vec3(pos.x, pos.y, 0));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		if ((num/1000)%2) draw_hat();
		else draw_hat2();
		num++;
		num %= 10000;
	}
	void move() {
		if (hat_version == 1) {
			pos.x = -500 * cosf(degree * TO_RADIAN);
			pos.y = 80 + 300 * sinf(degree * TO_RADIAN);
		}
		else if (hat_version == 2) {
			float tempdegree = 180 + degree;
			pos.x = -500 * cosf(tempdegree * TO_RADIAN);
			pos.y = 80 + 500 * sinf(tempdegree * TO_RADIAN);
		}
		draw();
		degree += 0.1;
		if (degree >= 180.0f) { degree = 0; hat_version = (hat_version) % 2 + 1; switch_hat = false; right_hand_up = true; }
	}
};
void (*draw_house_objects[5])() = {
	draw_house_body,
	draw_house_chimney,
	draw_house_door,
	draw_house_roof,
	draw_house_window
};
class house {
public:
	glm::vec2 pos;
	int timer = 0;
	bool is_break_down = false;
	int num_car = 0;
	void draw() {

		float scale = 1.0 + (float)num_car / 3;
		if (num_car < 12) {
			glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos, 0.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scale, scale, 1));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			for (int i = 0; i < 5; i++) {
				draw_house_objects[i]();
			}
		}
		else {
			is_break_down = true;
			for (int i = 0; i < 5; i++) {
				glm::mat4 ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(pos.x, pos.y, 0.0f));
				ModelMatrix = glm::rotate(ModelMatrix, 72.0f * i * TO_RADIAN, glm::vec3(0, 0, 1));
				ModelMatrix = glm::translate(ModelMatrix, glm::vec3(timer * 0.05, exp2f(timer * 0.05), 0.0f));
				ModelMatrix = glm::scale(ModelMatrix, glm::vec3(scale * (1 + timer / 200.0f), scale * (1 + timer / 200.0f), 1));
				ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
				glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
				draw_house_objects[i]();
			}
			timer++;
			if (timer >= 200) {
				timer = 0;
				num_car = 0;
				is_break_down = false;
			}
		}
	}
};