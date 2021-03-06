#include "MT-10.h"

#define B0 1024
#define B1 0
#define B2 -1024

  // =============
  // recibe una tensión de entrada, y según el filtro en el que se encuentre el sistema
  // aplica el sistema de la documentación
  // =============
  int filtrado(int tension_ent){
    //Constantes de filtrado necesarias para los cálculos
    static int a [2][7] = {{-2029, -2011, -1970, -1878, -1660, -1115, 141} , {1006, 988, 955, 890, 772, 569, 239}};
    static int filtros_ganancia [7] = {8, 17, 34, 66, 125, 227, 392};

    static int historia[2][7] = { {0,0,0,0,0,0,0} , {0,0,0,0,0,0,0}};

    int salida;
    int aux;

    aux = historia[0][filtro];

    salida = B0 * tension_ent -a[0][filtro] * historia[0][filtro] -a[1][filtro] * historia[1][filtro];
    historia[0][filtro] = salida >> 10;

    salida += B1 * aux + historia[1][filtro] * B2;
    historia[1][filtro] = aux;

    salida = salida >> 10;
    salida = salida * filtros_ganancia[filtro];
    salida = salida >> 10;
    return salida;
  }


  // =============
  // llama a la función filtrado para cada filtro, y suma sus salidas según la ganancia seleccionada mediante
  // la interfaz. Además gestiona la salida del nv de energía por el vúmetro
  // =============
  int filtradoMultiple () {
    int output;
    int i;
    int salida_unica;
    int tension;
    static int ganancia_energia [16] = {1024, 790, 610, 471, 364, 281, 217, 167, 129, 99, 77, 59, 46, 35, 27, 21};

    output = 0;
    tension = leerADC();
    for(i=0; i<7 ;i++){
      filtro = i;

      salida_unica =  (filtrado(tension) * ganancia_energia[nv[i]]) >> 10;
      output += salida_unica;

      if(i==fila_ilum)
        nv_energia += salida_unica*salida_unica;
    }

    output = output >> 1;
    return output;
  }
