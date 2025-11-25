#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <cfloat>

#define MAX_ITERS 1000


typedef struct { float x, y; } Point;


// ---------- Вспомогательные функции
void load_points(const char* filename, Point* p, int n)
{
    FILE* f;
    f = fopen(filename, "r");
    if (f == NULL)
    {
        printf("File not found. %s\n", filename);
        exit(0);
    }
    for (int i = 0; i < n; i++)
    {
        fscanf(f, "%f %f", &p[i].x, &p[i].y);
    }
    fclose(f);
}

void print_centers(Point* c, int K)
{
    for (int i = 0; i < K; i++)
        printf("Center %d: (%.2f, %.2f)\n", i, c[i].x, c[i].y);
}



void assign_clusters(const Point* points, int* labels, const int points_num,  const Point* centers,  const int centers_num)
{
	for (int i = 0; i < points_num; i++)
	{
		float minDist = FLT_MAX;
		int best = 0;
		for (int j = 0; j < centers_num; j++) {
			float dx = points[i].x - centers[j].x;
			float dy = points[i].y - centers[j].y;
			float dist = dx * dx + dy * dy;
			if (dist < minDist) {
				minDist = dist;
				best = j;
			}
		}
		labels[i] = best;
    }
}

// ---------- Главная программа ----------
int main()
{
    int clustersNum = 3;              // количество кластеров
    int pointsNum = 100;           // количество точек

    srand((unsigned)time(NULL));

    Point* points = (Point*)malloc(sizeof(Point) * pointsNum);
    Point* centers = (Point*)malloc(sizeof(Point) * clustersNum);
    Point* newCenters = (Point*)malloc(sizeof(Point) * clustersNum);
    int* labels = (int*)malloc(sizeof(int) * pointsNum);
    int* counts = (int*)malloc(sizeof(int) * clustersNum);

    load_points("75.txt", points, 100);
    for (int i = 0; i < clustersNum; i++)
        centers[i] = points[i]; // начальные центры берутся по первым 3 загруженным точкам


    for (int iter = 0; iter < MAX_ITERS; iter++)
    {
        //очистка памяти на каждой итерации
        memset(counts, 0, sizeof(int) * clustersNum);
        memset(newCenters, 0, sizeof(Point) * clustersNum);

        // для каждой точки находим ближайший центр кластера
        assign_clusters(points,  labels, pointsNum, centers, clustersNum);

        // Пересчитываем центры на CPU
        for (int i = 0; i < pointsNum; i++) {
            int c = labels[i];
            newCenters[c].x += points[i].x;
            newCenters[c].y += points[i].y;
            counts[c]++;
        }

        for (int j = 0; j < clustersNum; j++) {
            if (counts[j] > 0) {
                newCenters[j].x /= counts[j];
                newCenters[j].y /= counts[j];
            }
            else {
                // если кластер пустой — выбрать случайную точку
                newCenters[j] = points[rand() % pointsNum];
            }
        }

        // Проверяем сходимость (разница между старым и новым центром)
        float shift = 0.0f;
        for (int j = 0; j < clustersNum; j++) {
            float dx = newCenters[j].x - centers[j].x;
            float dy = newCenters[j].y - centers[j].y;
            shift += dx * dx + dy * dy;
            centers[j] = newCenters[j];
        }

        //printf("Iteration %d, shift=%.4f\n", iter + 1, shift);
        if (shift < 1e-4f) break;
    }

    // --- Вывод результатов ---
    printf("\nFinal cluster centers:\n");
    print_centers(centers, clustersNum);

    free(counts);
    free(newCenters);
    free(points);
    free(centers);
    free(labels);
    return 0;
}
