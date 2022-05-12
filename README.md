В каждой папке находятся задачи "нулёвки" перед выполнением лабы и сама лаба в отдельной папке. 

1)  Первая лабораторная работа - расчёт уравнения переноса, 
выполняемый по схеме "прмоугольник", прилагаются гарфики эффективности, ускорения и времени выполнения программы в зависимости от числа исполнителей.

Запуск кода первой лабораторной работы:

  mpicc LW_1.c -o lw1 -lm  
  mpirun -np m ./lw1 dt
	
Здесь m - количество исполнителей, dt - шаг по сетке в направлении времени

Пример вывода mpirun -np 4 ./lw1 0.001:  
  [ Info ] Test for 4 executors  
  [ Info ] dt: 0.001000; dx: 0.002000; dots quantity: 500000  
  [Exec 0] Time 0.002495  
  [Exec 1] Time 0.003296  
  [Exec 2] Time 0.003342  
  [Exec 3] Time 0.003330  
    
    
    
2)  Вторая лабораторная работа - численное вычисление интеграла функции sin(1/x).Программа должна реализовать численное интегрирование заданной
функции на заданном интервале с использованием распараллеливания с общей памятью. Аргументы программы – число потоков, необходимая точность
интегрирования. Требуется обеспечить оптимальный шаг для каждого из участков интегрирования и динамическую балансировку между исполнителями, а также универсальность программы (последнее подразумевает, что функция записана только в одном месте в программе, оценка её поведения проводится численными методами, а не при помощи заранее вычисленных производных разных степеней). Требуется измерить ускорение и эффективность программы.  

Запуск кода второй лабораторной работы:  
  gcc LW_2.c -o lw2 -pthread -l  
  ./lw2 m e  
  Здесь m - количество исполнителей, e - нужная величина ошибки  
 Пример вывода ./lw2 8 0.000000001:  
  [  Info  ] Numerical integration of a function(x) on a segment (0.000010, 1.000000)  
  [  Info  ] Precision: 0.000000; Number of threads: 8  
  [Thread 3] Adding 0.508532 to the total amount (total amount = 0.508532)  
             Calculated 354268616 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [Thread 6] Adding 0.000494 to the total amount (total amount = 0.509026)  
             Calculated 355363923 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [Thread 4] Adding 0.000106 to the total amount (total amount = 0.509131)  
             Calculated 363771416 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [Thread 1] Adding -0.025084 to the total amount (total amount = 0.484047)  
             Calculated 360590933 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [Thread 2] Adding 0.000169 to the total amount (total amount = 0.484217)  
             Calculated 344885043 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [Thread 7] Adding -0.000007 to the total amount (total amount = 0.484209)  
             Calculated 362305656 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [Thread 0] Adding -0.000853 to the total amount (total amount = 0.483356)  
             Calculated 363353847 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [Thread 5] Adding 0.020711 to the total amount (total amount = 0.504067)  
             Calculated 357203061 segments  
             CPU time: 2m 06s, Wall time: 0m 16s  
  [ Result ] J = 0.504067  
   
 Во второй работе также имеются графики эффективности, времени выполнения программы и ускорения в зависимости от числа исполнителей.  Также в выводе написаны время работы программы на каждом процессоре и число посчитанных итераций для каждого исполнителя, чт одемонстрирует динамическую балансировку работы исполнителей. 
 
