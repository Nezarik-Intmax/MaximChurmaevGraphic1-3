# MaximChurmaevGraphic1-3
```c++
#include <iostream>
#include "GL/glew.h";
#include "GL/glut.h";
#include "glm/glm.hpp";

GLuint VBO;
void RenderSceneOB() {
	glEnableVertexAttribArray(0);		//Включает индексирование атрибутов
	glBindBuffer(GL_ARRAY_BUFFER, VBO);	//Привязывает Буфер VBO к цели GL_ARRAY_BUFFER (массив вершин)
	//Передает данные у буфере веришин(начальный индекс, кол-во параметров(X,Y,Z), тип, нужно ли нормализовать, расстояние между двумя эл-ми, смещение в структуре)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glDrawArrays(GL_TRIANGLES, 0, 3);	//Отрисовывыет 3 вершины из буфера типом GL_TRIANGLES с индекса 0
	glDisableVertexAttribArray(0);		//Выключает индексирование атрибутов
	glutSwapBuffers();			//Меняет местами Выводимый на экран буфер с резервным
}
int main(int argc, char** argv)
{
	glutInit(&argc, argv);				//Инициализация glutInit
	//Установка режима отображения glutInit. GLU_DOUBLE - двойной буфер, GLUT_RGBA - красный-зеленый-синий-альфа-канал
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA); 	
	glutInitWindowSize(1024, 768);			//Размер окна
	glutInitWindowPosition(100, 100);		//Позиция окна
	glutCreateWindow("Tutorial 03");		//Заголовок
	glutDisplayFunc(RenderSceneOB);			//Привязываем функцию, которая будет вызываться в glutMainLoop

	GLenum res = glewInit();			//Инициализация GLenum
	if (res != GLEW_OK) {				//Проверка на ошибки
		fprintf(stderr, "Error: '%s' \n", glewGetErrorString(res));
		return 1;
	}
	glm::vec3 Vertices[3];					//Создание массива векторов
	Vertices[0] = glm::vec3(1.0f, 1.0f, 0.0f);		//Инициализация векторов
	Vertices[2] = glm::vec3(-1.0f, 1.0f, 0.0f);
	Vertices[1] = glm::vec3(0.0f, -1.0f, 0.0f);
	glGenBuffers(1, &VBO);					//Генерация Буфера
	glBindBuffer(GL_ARRAY_BUFFER, VBO);			//Привязывает Буфер VBO к цели GL_ARRAY_BUFFER (массив вершин)
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);	//Заполняет цель GL_ARRAY_BUFFER данными из Verices
	glutMainLoop();		//Главный цикл программы
}
```
