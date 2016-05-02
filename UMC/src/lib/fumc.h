#ifndef LIB_FUMC_H_
#define LIB_FUMC_H_

	#include <utilidades/sockets.h>
	#include <utilidades/general.h>
	#include <commons/config.h>
	#include <commons/log.h>
	#include <pthread.h>

	#define PACKAGESIZE 1024 // Size máximo de paquete para sockets
	#define RUTA_CONFIG_UMC "configUMC.txt"

	// Estructuras
	typedef struct {
		int backlog;
		int puerto; // para conexiones de CPU y Núcleo
		char *ip_swap;
		int puerto_swap;
		int marcos;
		int marco_size;
		int marco_x_proceso;
		int entradas_tlb;
		int retardo;
	} t_configuracion;

	// Cabeceras
	void abrirArchivoConfig(char *ruta); // Setea todos los valores de configuración
	void conectarConSwap(); // Se crea al principio. Luego se llama a necesidad
	void crearHilos(); // Crea hilos servidor y consola
	void servidor(); // Las conexiones de CPU y Núcleo se van a realizar por acá
	void consola(); // Entrada por stdin
	void crearHiloCliente(int *sockCliente); // Crea un hilo cliente al aceptar conexión
	void cliente(void* sockCliente); // Maneja pedidos del cliente
	void liberarEstructura();
	void liberarRecusos();
	int validar_cliente(char *id); // Valida que el cliente sea CPU o Nucleo
	int validar_servidor(char *id); // Valida que el servidor sea Swap

#endif /* LIB_FUMC_H_ */
