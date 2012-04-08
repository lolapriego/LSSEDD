#include "MT-10.h"


//función que se ejecuta después de init
//se ejecuta constantemente, sirve para seleccionar el estado del sistema
void bucleMain(void){
char opcion;


printf("Seleccione una de las siguientes opciones\n");
output("1) Caracterización de filtros\n");
output("2) Ecualización Gráfica\n");
output("3) Incorporación de Reverberación Simple\n");
output("---------------------------------\n");
opcion=teclado();

switch (opcion){
case '1': printf("Caracterización de filtros\n");
	GestionCaracterizacion();
	break;
case '2': printf("Ecualización Gráfica\n");
	GestionEcualizacion();
	break;
case'3': printf("Incorporación de Reverberación Simple\n");
	GestionReverberacion();
	break;
default: output("Tecla no válida\n");

 

}
}//buclemain


//función para seleccionar el filtro con el que se desea filtrar
void GestionCaracterizacion(){
char opcion;
printf("Seleccione el filtro que desea caracterizar u 8 para volver al menu inicio\n");
opcion=teclado();
if(opcion=='8')
return;
filtro= opcion - '0' -1;
printf("Ha seleccionado el filtro: %d\n",filtro +1); 
estadoFiltrado=1;
filtrado(leerADC(), filtro);


}



void GestionEcualizacion(){
}


void GestionReverberacion(){
}

void __init(){
	swInit();
	hwInit();
}



void swInit(){
 int i;

	filtros[0].banda=32; // no habría que inicializarlo aquí, no variables globale son constantes!!
	filtros[0].ganancia=8;
	filtros[0].nv=0;
	filtros[1].banda=64;
	filtros[1].ganancia=17;
	filtros[1].nv=0;
	filtros[2].banda=125;
	filtros[2].ganancia=34;
	filtros[2].nv=0;
	filtros[3].banda=250;
	filtros[3].ganancia=66;
	filtros[3].nv=0;
	filtros[4].banda=500;
	filtros[4].ganancia=125;
	filtros[4].nv=0;
	filtros[5].banda=1000;
	filtros[5].ganancia=227;
	filtros[5].nv=0;
	filtros[6].banda=2000;
	filtros[6].ganancia=392;
	filtros[6].nv=0;

	estadoFiltrado=0;
	filtro=0; //filtro por defecto
	for(i=0;i <2;i++){
	historia[i]=0;
	}

	
	
}

void hwInit(){
  initInt();
  DAC_ADC_init();


}


//------------------------------------------------------
// void rutina_tout0(void)
//
// Descripción:
//   Función de atención a la interrupción para TIMER0
// Si se ha empezado a filtrar llama a la función filtrado con cada interrupción
//------------------------------------------------------
void rutina_tout0(void)
{
  mbar_writeShort(MCFSIM_TER0,BORRA_REF); 	// Reset del bit de fin de cuenta
if(estadoFiltrado)
	  filtrado(leerADC(), filtro);

}

//------------------------------------------------------
// void initInt(void)
//
// Descripción:
//   Función por defecto de inicialización de la interrupción del timer 0
//------------------------------------------------------
void initInt()
{
  mbar_writeByte(MCFSIM_PIVR,V_BASE);			// Fija comienzo de vectores de interrupción en V_BASE.
  ACCESO_A_MEMORIA_LONG(DIR_VTMR0)= (ULONG)_prep_TOUT0; // Escribimos la dirección de la función para TMR0
  output("COMIENZA EL PROGRAMA\r\n");
  mbar_writeShort(MCFSIM_TMR0, (PRESCALADO-1)<<8|0x3D);		// TMR0: PS=1-1=0 CE=00 OM=1 ORI=1 FRR=1 CLK=10 RST=1
  mbar_writeShort(MCFSIM_TCN0, 0x0000);		// Ponemos a 0 el contador del TIMER0
  mbar_writeShort(MCFSIM_TRR0, CNT_INT1);	// Fijamos la cuenta final del contador
  mbar_writeLong(MCFSIM_ICR1, 0x8888C888);	// Marca la interrupción del TIMER0 como no pendiente
  sti();					// Habilitamos interrupciones
}




void iO () //para comprobar rápido que todo va bien, lee del ADC y debería sacar lo mismo por el DAC escalado x2
{
WORD tension1;
int lectura;
  double tension;
  lectura = ADC_dato();
if(lectura & 0x00000800)
lectura = lectura | 0xFFFFF000;

// Calcula la tensión correspondiente al valor leído
tension = ((double)lectura/FONDO_ESCALA);

tension1 = (tension * 0xFFF) ;
DAC_dato(tension1 + 0x800);
     
}




//lee un dato del ADC y devuelve la tensión 
int leerADC(){
int lectura;
 lectura = ADC_dato();
if(lectura & 0x00000800)
lectura = lectura | 0xFFFFF000;
return lectura;
}






//dada una tensión de entrada saca por el DAC la tensión de salida filtrada

void filtrado(int tension_ent, int filtro){

int salida;
int tension1;
int aux;

if(estadoFiltrado==1){
	int i;
	for(i=0;i <2;i++){ // cada vez que se reinicie el filtrado, borramos el historial
	historia[i]=0;
	}
	estadoFiltrado=2;
	salida= B0 * tension_ent;
	historia[0] = (B0*tension_ent) >> 10;
	salida = salida >> 10;
	salida = salida * filtros[filtro].ganancia;
	salida = salida >> 10;
	DAC_dato(salida + 0x800);
}

else if (estadoFiltrado==2) { 
estadoFiltrado = 3;
salida = B0 * tension_ent + B1 * historia[0] -a[0][filtro] * historia[0];
historia[1] = historia[0];
historia[0] = ( B0 * tension_ent -a[0][filtro] * historia[0] ) >>10;
salida = salida >> 10;
salida = salida * filtros[filtro].ganancia;
salida = salida >> 10;
DAC_dato(salida + 0x800);
}

else if(estadoFiltrado==3){
salida= B0*tension_ent + B1* historia[0] -a[0][filtro]* historia[0] + historia[1]*(-a[1][filtro]+B2);
aux = historia[0];
historia[0]= ( B0*tension_ent -a[0][filtro]* historia[0] -a[1][filtro]*historia[1] ) >>10;
historia[1]=aux;
salida = salida >> 10;
salida = salida * filtros[filtro].ganancia;
salida = salida >> 10;
DAC_dato(salida + 0x800);
}

 

}

void rutina_int1(void){}

void rutina_int2(void){}

void rutina_int3(void){}

void rutina_int4(void){}
void rutina_tout1(void){}
void rutina_tout2(void){}
void rutina_tout3(void){}
