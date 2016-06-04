#ifndef LIB_FUNCIONES_H_
#define LIB_FUNCIONES_H_

#include "globales.h"

/**** FUNCIONES SECUNDARIAS ****/
void setearValores_config(t_config * archivoConfig);
void crearLogger();
void actualizarDatosDePcbEjecutada(cpu * unCPU, pcb * pcbNuevo);
void inicializarIndices(pcb* pcb, t_metadata_program* metaData);
int asignarPid(t_list procesos);
pcb* buscarProcesoPorPid(int pid, int* index);
pcb* crearPCB(string programa);
void liberarPcb(pcb * pcb);
void planificarProceso();
void finalizarPrograma(int pid);
void limpiarListasYColas();
void liberarConsola(consola * consola);
void liberarCPU(cpu * cpu);

#endif /* LIB_FUNCIONES_H_ */