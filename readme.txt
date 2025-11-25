Install OpenCL lib from
https://github.com/GPUOpen-LibrariesAndSDKs/OCL-SDK/releases/download/1.0/OCL_SDK_Light_AMD.exe

VS project settings ( OpenCL default path "C:\Program Files (x86)\OCL_SDK_Light"):
 Visual Studio Project
 "Project Properties -> C/C++ -> General -> Additional Include Directories" = C:\Program Files (x86)\OCL_SDK_Light\include; 
 "Project Properties -> Linker -> All Options -> Additional Dependencies" = OpenCL.lib; 
 "Project Properties -> Linker -> All Options -> Additional Library Directories" =  C:\Program Files (x86)\OCL_SDK_Light\lib\x86_64;

Code Blocks project settings ( OpenCL default path "C:\Program Files (x86)\OCL_SDK_Light"):
 Code Blocks Project
 "Project -> Build options -> Linker settings -> Link libraries" -> Add = C:\Program Files (x86)\OCL_SDK_Light\lib\x86_64\OpenCL.lib
 "Project -> Build options -> Search directories -> Compiler" -> Add = C:\Program Files (x86)\OCL_SDK_Light\include
 "Project -> Build options -> Search directories -> Linker" -> Add = C:\Program Files (x86)\OCL_SDK_Light\lib\x86_64


Project "kmean_gen"
 Генерирует набор точек в кластерах.

 По умолчанию количество кластеров = 3
 Центры кластеров определяются на основании номера варианта N от 1 до 999.
 По умолчанию генерируется 100 точек.
 Точки генерируются вокруг центра кластеров с заданным разбросом (По умолчанию 5.0)
 Сгенерированные точки сохранятся в файле "N.txt"
 Центры кластеров сохраняются в файле "N res.txt"

Project "kmean_cl"
 Считвает набор точек из файла и опеределяет их по 3-м кластерам
 Выводит на экран 3 центра кластеров [{x,y}]
 Файл с данными должен лежать в основной папке проекта
 Количество кластеров = 3 
 Количество точек = 100