#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define  _USE_MATH_DEFINES
#include <math.h>

typedef struct { double x, y; } Point;

float randf(float min, float max)
{
	return min + ((float)rand() / RAND_MAX) * (max - min);
}
void generate_centers(int N, Point* centers)
{
	// Проверим корректность диапазона вариантов
	if (N < 0 || N > 999)
	{
		exit;
	}

	// Извлекаем цифры
	int h = (N / 100) % 10;
	int t = (N / 10) % 10;
	int u = N % 10;

	// Базовый вектор
	double vx = h + 1;
	double vy = t + 1;

	// Углы поворота (в радианах)
	double alpha = 2 * M_PI * (u + 1) / 11.0;
	double beta = 2 * M_PI * (h + 1) / 13.0;

	// Повороты вектора
	centers[0].x = vx;
	centers[0].y = vy;
	centers[1].x = vx * cos(alpha) - vy * sin(alpha);
	centers[1].y = vx * sin(alpha) + vy * cos(alpha);
	centers[2].x = vx * cos(beta) - vy * sin(beta);
	centers[2].y = vx * sin(beta) + vy * cos(beta);
	// Нормализация и масштабирование в диапазон [0, 100]
	double scale = 100.0 / 20.0;
	for (int i = 0; i < 3; i++)
	{
		centers[i].x = (centers[i].x + 10.0) * scale;
		centers[i].y = (centers[i].y + 10.0) * scale;
	}
}

void saveToFile(int var, int clusterNum, Point* centers, int pointNum, Point* points)
{
	// сохраняем в фалах сгенерированные точки и исходные центры кластеров
	// "N.txt" - файл с точками, N - номер варианта
	// "N res.txt" - файл с центрами кластеров, N - номер варианта
	char file1[50], file2[50];

	sprintf_s(file1, "%d.txt", var);
	sprintf_s(file2, "%d res.txt", var);

	FILE* f;
	fopen_s(&f, file1, "w");
	for (int i = 0; i < pointNum; i++)
	{
		fprintf(f, "%f %f\n", points[i].x, points[i].y);
	}
	fclose(f);

	fopen_s(&f, file2, "w");
	for (int i = 0; i < clusterNum; i++)
	{
		fprintf(f, "%f %f\n", centers[i].x, centers[i].y);
	}
	fclose(f);
}

int main(int argc, char* argv[]) 
{
	// argv[1] - количество точек для генерации
	// argv[2] - количество кластеров
	// argv[3] - разброс точек от центра кластера при генерации


	int pointNum; //Количество точек
	int clusterNum; //Количество кластеров
	int variantNo=1; //номер варианта
	float spread;
	if (argc == 4)
	{
		pointNum = atoi(argv[1]);
		clusterNum = atoi(argv[2]);
		spread = (argc >= 4) ? atof(argv[3]) : 5.0f;
	}
	else
	{
		pointNum = 100;
		clusterNum = 3;
		spread = 5.0;
		variantNo = 75;
	}

	srand((unsigned)time(NULL));

	// Генерация центров кластеров
	Point* centers = (Point*)malloc(sizeof(Point) * clusterNum);
	generate_centers(1000 - variantNo, centers);


	Point* points = (Point*)malloc(sizeof(Point) * pointNum);

	// Распределяем точки вокруг центров
	for (int i = 0, cluster; i < pointNum; i++)
	{
		
		if (i < 3)
			cluster = i; // первые 3 точки из разых кластеров
		else
			cluster = rand() % clusterNum; // выбираем случайный кластер
		double angle = randf(0, 2 * M_PI);
		double radius = randf(0, spread);

		points[i].x = centers[cluster].x + cos(angle) * radius;
		points[i].y = centers[cluster].y + sin(angle) * radius;
	}

	saveToFile(variantNo, clusterNum, centers, pointNum, points);

	free(centers);
	return 0;
}
