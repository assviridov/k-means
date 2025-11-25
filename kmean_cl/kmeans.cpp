#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <CL/cl.h>
#include <time.h>
#include <string.h>

#define MAX_SOURCE_SIZE 4096
#define MAX_ITERS 1000


typedef struct { float x, y; } Point;

// ---------- OpenCL kernel ----------
const char* kernelSource =
"__kernel void assign_clusters(                         \n"
"   __global const float2 *points,                      \n"
"   __global const float2 *centers,                     \n"
"   __global int *labels,                               \n"
"   const int k) {                                      \n"
"   int i = get_global_id(0);                           \n"
"   float minDist = FLT_MAX;                            \n"
"   int best = 0;                                       \n"
"   for (int j = 0; j < k; j++) {                       \n"
"       float dx = points[i].x - centers[j].x;          \n"
"       float dy = points[i].y - centers[j].y;          \n"
"       float dist = dx*dx + dy*dy;                     \n"
"       if (dist < minDist) {                           \n"
"           minDist = dist;                             \n"
"           best = j;                                   \n"
"       }                                               \n"
"   }                                                   \n"
"   labels[i] = best;                                   \n"
"}                                                      \n";

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



    // --- OpenCL setup ---
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_int err;

    //получение первой платформы из списка
    clGetPlatformIDs(1, &platform, NULL);
    //получение первого устройства на платформе. тип устройства: CL_DEVICE_TYPE_CPU CL_DEVICE_TYPE_GPU
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    //создание контекста и очереди
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    queue = clCreateCommandQueueWithProperties(context, device, 0, &err);

    //создание программы из исходного кода ядра
    program = clCreateProgramWithSource(context, 1, &kernelSource, NULL, &err);
    //компиляция программы
    clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    //создание ядра
    kernel = clCreateKernel(program, "assign_clusters", &err);

    // выделение памяти в контексте устройства
    cl_mem bufPoints = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Point) * pointsNum, NULL, &err);
    cl_mem bufCenters = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Point) * clustersNum, NULL, &err);
    cl_mem bufLabels = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * pointsNum, NULL, &err);

    size_t global = pointsNum;

    for (int iter = 0; iter < MAX_ITERS; iter++)
    {
        //очистка памяти на каждой итерации
        memset(counts, 0, sizeof(int) * clustersNum);
        memset(newCenters, 0, sizeof(Point) * clustersNum);

        // Обновляем буферы на устройстве
        clEnqueueWriteBuffer(queue, bufPoints, CL_TRUE, 0, sizeof(Point) * pointsNum, points, 0, NULL, NULL);
        clEnqueueWriteBuffer(queue, bufCenters, CL_TRUE, 0, sizeof(Point) * clustersNum, centers, 0, NULL, NULL);

        // Настраиваем аргументы кернела
        clSetKernelArg(kernel, 0, sizeof(cl_mem), &bufPoints);
        clSetKernelArg(kernel, 1, sizeof(cl_mem), &bufCenters);
        clSetKernelArg(kernel, 2, sizeof(cl_mem), &bufLabels);
        clSetKernelArg(kernel, 3, sizeof(int), &clustersNum);

        // Запускаем кернел
        clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);
        clFinish(queue);

        // Читаем результаты (метки кластеров)
        clEnqueueReadBuffer(queue, bufLabels, CL_TRUE, 0, sizeof(int) * pointsNum, labels, 0, NULL, NULL);

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

    // Очистка
    clReleaseMemObject(bufPoints);
    clReleaseMemObject(bufCenters);
    clReleaseMemObject(bufLabels);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    free(counts);
    free(newCenters);
    free(points);
    free(centers);
    free(labels);
    return 0;
}
