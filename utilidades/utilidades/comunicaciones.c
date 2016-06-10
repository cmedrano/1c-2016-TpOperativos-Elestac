#include "comunicaciones.h"


void aplicar_protocolo_enviar(int fdReceptor, int protocolo, void *mensaje){

	int desplazamiento = 0, tamanioMensaje, tamanioTotal;

	if (protocolo < 1 || protocolo > FIN_DEL_PROTOCOLO){
		fprintf(stderr, "Error al enviar paquete. No existe protocolo definido para %d\n", protocolo);
		abort(); // es abortivo
		}

	// Serealizar un mensaje dado un protocolo, me devuelve el mensaje empaquetado:
	void *mensajeSerealizado = serealizar(protocolo, mensaje);
	tamanioMensaje = sizeof(mensajeSerealizado);
	/* Lo que se envía es: el protocolo + tamaño del msj + el msj serializado
	 * Entonces, el tamaño total de lo enviado es:
	 * 	16 bytes de los dos int + el tamaño del msj empaquetado */

	tamanioTotal = 2*INT + tamanioMensaje;

	// Meto en el paquete para enviar, esas tres cosas:
	void *buffer = reservarMemoria(tamanioTotal);
	memcpy(buffer + desplazamiento, &protocolo, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &tamanioMensaje, INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensajeSerealizado, tamanioMensaje);

	// Se envía la totalidad del paquete (lo contenido en el buffer):
	enviarPorSocket(fdReceptor, buffer, tamanioTotal);

	free(buffer);
	free(mensajeSerealizado);
}


void * aplicar_protocolo_recibir(int fdEmisor, int * protocolo){

	// Recibo primero el head y lo verifico:
	int recibido = recibirPorSocket(fdEmisor, protocolo, INT);

	if (*protocolo < 1 || *protocolo > FIN_DEL_PROTOCOLO || recibido <= 0)
		return NULL; // Validar contra NULL al recibir en cada módulo (tirar un mensaje de error notificando)

	// Recibo ahora el tamaño del mensaje:
	int* tamanioMensaje = (int*)reservarMemoria(INT);
	recibirPorSocket(fdEmisor, tamanioMensaje, INT);

	// Recibo por último el mensaje serializado:
	void * mensaje = reservarMemoria(*tamanioMensaje);
	recibirPorSocket(fdEmisor, mensaje, *tamanioMensaje);

	// Deserealizo:
	void *buffer = deserealizar(*protocolo, mensaje);
	free(mensaje);
	free(tamanioMensaje);

	return buffer;

} // Se debe castear el mensaje al recibirse (indicar el tipo de dato que debe matchear con el void*)


void * serealizar(int protocolo, void * elemento){
	void * buffer;

	switch(protocolo){
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO:{
			buffer = serealizarTexto(elemento);
			break;
		} // en ambos casos se envía un texto (char*)
		case PCB: {
			buffer = serealizarPCB(elemento);
			break;
		}
		case INICIAR_PROGRAMA: {
			buffer = serealizarSolicitudInicioPrograma(elemento);
			break;
		}
		case PEDIDO_ESCRITURA: {
			buffer = serealizarSolicitudEscritura(elemento);
			break;
		}
		case RESPUESTA_PEDIDO:{
			buffer = serializarRespuestaPedido(elemento);
			break;
		}
		case FIN_QUANTUM: case FINALIZAR_PROGRAMA: case IMPRIMIR: case RECHAZAR_PROGRAMA: case PEDIDO_LECTURA:
			case RESPUESTA_INICIO_PROGRAMA: case QUANTUM_MODIFICADO:case LEER_PAGINA: case INDICAR_PID:{
			// En estos casos se reciben elementos estáticos o estructuras con campos estáticos:
			int tamanio = sizeof(elemento);
			buffer = reservarMemoria(tamanio);
			memcpy(buffer, elemento, tamanio);
			break;
		}
		case ESCRIBIR_PAGINA:{
			buffer = serealizarEscribirPagina(elemento);
			break;
		}
		case DEVOLVER_PAGINA:{
			buffer = serealizarDevolverPagina(elemento);
			break;
		}
		case 0:{
			printf("Se produjo una desconexión de sockets al serealizar\n"); // es el caso del recv = 0
			break;
		}
	}
	return buffer;
}

void * deserealizar(int protocolo, void * mensaje){
	void * buffer;

	switch(protocolo){
		case ENVIAR_SCRIPT: case IMPRIMIR_TEXTO: case DEVOLVER_CONTENIDO: {
			buffer = deserealizarTexto(mensaje);
				break;
		} // en ambos casos se recibe un texto (char*)
		case PCB:{
			buffer = deserealizarPCB(mensaje);
			break;
		}
		case INICIAR_PROGRAMA:{
			buffer = deserealizarSolicitudInicioPrograma(mensaje);
			break;
		}
		case PEDIDO_ESCRITURA: {
			buffer = deserealizarSolicitudEscritura(mensaje);
			break;
		}
		case RESPUESTA_PEDIDO:{
			buffer = deserializarRespuestaPedido(mensaje);
			break;
		}
		case FIN_QUANTUM: case FINALIZAR_PROGRAMA: case IMPRIMIR: case RECHAZAR_PROGRAMA: case PEDIDO_LECTURA:
			case RESPUESTA_INICIO_PROGRAMA: case QUANTUM_MODIFICADO: case LEER_PAGINA: case INDICAR_PID: {
			// En estos casos se reciben elementos estáticos o estructuras con campos estáticos:
			int tamanio = sizeof(mensaje);
			buffer = reservarMemoria(tamanio);
			memcpy(buffer, mensaje, tamanio);
			break;
		}
		case ESCRIBIR_PAGINA: {
			buffer = deserializarEscribirPagina(mensaje);
			break;
		}
		case DEVOLVER_PAGINA:{
			buffer = deserializarDevolverPagina(mensaje);
			break;
		}
		case 0:{
			printf("Se produjo una desconexión de sockets al desearealizar\n");
			// es el caso del recv = 0 // TODO: el retorno de recv da 0. Si es así no puede setearse protocolo y tampoco entra en la función
			break;
		}
	}
	return buffer;
} // Se debe castear lo retornado (indicar el tipo de dato que debe matchear con el void*)

/**** SERIALIZACIONES PARTICULARES ****/

// entra:	char *mensaje
// sale: 	int tamanio | char *mensaje
void *serealizarTexto(void *elemento) {

	char *mensaje = (char*)elemento;

	int desplazamiento = 0;
	int tamanioString = CHAR * strlen(mensaje);
	int tamanioTotal = INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);
	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;
	memcpy(buffer + desplazamiento, mensaje, tamanioString);

	return buffer;
}

// entra:	int tamanio | char *mensaje
// sale:	char *mensaje
void *deserealizarTexto(void *buffer) {

	int desplazamiento = 0, *tamanioString = NULL;

	memcpy(tamanioString, buffer + desplazamiento, INT);
	desplazamiento += INT;
	char *mensaje = (char*)reservarMemoria(*tamanioString);
	memcpy(mensaje, buffer + desplazamiento, *tamanioString);

	return mensaje;
}

void * serealizarPCB(void * estructura){
	pcb * unPCB = (pcb *) estructura;
	int tamanioTotal, desplazamiento = 0;

	int codigoSize = (sizeof(t_intructions) * unPCB->tamanioIndiceCodigo);

	int tamListaArgumentos = sizeof(direccion) * unPCB->indiceStack->tamanioListaArgumentos;
	int tamListaVariables = sizeof(variable) * unPCB->indiceStack->tamanioListaVariables;
	int tamRegistroStack = 6*INT + tamListaArgumentos + tamListaVariables;
	int stackSize = tamRegistroStack * unPCB->tamanioIndiceStack;

	int etiquetasSize = (CHAR * unPCB->tamanioIndiceEtiquetas);

	tamanioTotal = 10*INT + codigoSize + stackSize + etiquetasSize;
	// package->total_size = sizeof(package->username_long) + package->username_long + sizeof(package->message_long) + package->message_long;

	void * buffer = reservarMemoria(tamanioTotal);
	memcpy(buffer + desplazamiento,&(unPCB->pid), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->id_cpu), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->pc), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->paginas_codigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->estado), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->estado), INT);
		desplazamiento += INT;

	// Serealizo el índice de código:
	memcpy(buffer + desplazamiento, &(unPCB->tamanioIndiceCodigo), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceCodigo, codigoSize);
		desplazamiento += codigoSize;

	// Serealizo el índice de etiquetas:
	memcpy(buffer + desplazamiento, &(unPCB->tamanioIndiceEtiquetas), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceEtiquetas, etiquetasSize);
		desplazamiento += etiquetasSize;

	// Serealizo el índice de stack:
	memcpy(buffer + desplazamiento, &(unPCB->tamanioIndiceStack), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->proximaInstruccion), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->posicionDelResultado), sizeof(direccion));
		desplazamiento += sizeof(direccion);
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->tamanioListaArgumentos), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceStack->listaPosicionesArgumentos, tamListaArgumentos);
		desplazamiento += tamListaArgumentos;
	memcpy(buffer + desplazamiento, &(unPCB->indiceStack->tamanioListaVariables), INT);
		desplazamiento += INT;
	memcpy(buffer + desplazamiento, unPCB->indiceStack->listaVariablesLocales, tamListaVariables);

	return buffer;
}

pcb * deserealizarPCB(void * buffer){
	int desplazamiento = 0;
	pcb * unPcb = reservarMemoria(sizeof(pcb));

	memcpy(&unPcb->pid, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->id_cpu, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->pc, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->paginas_codigo, buffer + desplazamiento, INT );
		desplazamiento += INT;
	memcpy(&unPcb->estado, buffer + desplazamiento, INT );
		desplazamiento += INT;

	// Deserealizo el índice de código:
	memcpy(&unPcb->tamanioIndiceCodigo, buffer + desplazamiento, INT);
		desplazamiento += INT;
	unPcb->indiceCodigo = reservarMemoria(unPcb->tamanioIndiceCodigo);
	memcpy(unPcb->indiceCodigo, buffer + desplazamiento, unPcb->tamanioIndiceCodigo);
		desplazamiento += unPcb->tamanioIndiceCodigo;

		// Deserealizo el índice de etiquetas:
	memcpy(&unPcb->tamanioIndiceEtiquetas, buffer + desplazamiento, INT);
		desplazamiento += INT;
	unPcb->indiceEtiquetas = reservarMemoria(unPcb->tamanioIndiceEtiquetas);
	memcpy(unPcb->indiceEtiquetas, buffer + desplazamiento, unPcb->tamanioIndiceEtiquetas);
		desplazamiento += unPcb->tamanioIndiceEtiquetas;

	// Deserealizo el índice de stack:
	memcpy(&unPcb->tamanioIndiceStack, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->indiceStack->proximaInstruccion, buffer + desplazamiento, INT);
		desplazamiento += INT;
	memcpy(&unPcb->indiceStack->posicionDelResultado, buffer + desplazamiento, sizeof(direccion));
		desplazamiento += sizeof(direccion);
	memcpy(&unPcb->indiceStack->tamanioListaArgumentos, buffer + desplazamiento, INT);
			desplazamiento += INT;
	unPcb->indiceStack->listaPosicionesArgumentos = reservarMemoria(unPcb->indiceStack->tamanioListaArgumentos);
	memcpy(unPcb->indiceStack->listaPosicionesArgumentos, buffer + desplazamiento, unPcb->indiceStack->tamanioListaArgumentos);
		desplazamiento += unPcb->indiceStack->tamanioListaArgumentos;
	memcpy(&unPcb->indiceStack->tamanioListaVariables, buffer + desplazamiento, INT);
		desplazamiento += INT;
	unPcb->indiceStack->listaVariablesLocales = reservarMemoria(unPcb->indiceStack->tamanioListaVariables);
	memcpy(unPcb->indiceStack->listaVariablesLocales, buffer + desplazamiento, unPcb->indiceStack->tamanioListaVariables);

	return unPcb;
}


// ingresa:	solicitudEscritura *mensaje
// sale:	int pagina | int offset | int tamanio_escritura | int tamanio_contenido | char *contenido
void *serealizarSolicitudEscritura(void *elemento) {

	solicitudEscritura *mensaje = (solicitudEscritura*)elemento;
	int tamanioTotal, desplazamiento = 0;

	int tamanioString = CHAR * strlen(mensaje->contenido);
	tamanioTotal = 4 * INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);
	memcpy(buffer + desplazamiento, &(mensaje->pagina), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &(mensaje->offset), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &(mensaje->tamanio), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, mensaje->contenido, tamanioString);

	return buffer;
}

// entra:	int pagina | int offset | int tamanio_escritura | int tamanio_contenido | char *contenido
// sale:	solicitudEscritura *mensaje
void *deserealizarSolicitudEscritura(void *buffer) {

	int desplazamiento = 0, *tamanioString = NULL;
	solicitudEscritura *mensaje = reservarMemoria(sizeof(solicitudEscritura));

	memcpy( &(mensaje->pagina), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( &(mensaje->offset), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( &(mensaje->tamanio), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( tamanioString, buffer + desplazamiento, INT );
	desplazamiento += INT;

	mensaje->contenido = (char*)reservarMemoria(*tamanioString);
	memcpy( mensaje->contenido, buffer + desplazamiento, *tamanioString );

	return mensaje;
}

// entra:	inicioPrograma *mensaje
// sale:	int pid | int paginas | int tamanio | char *contenido
void *serealizarSolicitudInicioPrograma(void *elemento) {

	inicioPrograma *mensaje = (inicioPrograma*)elemento;
	int tamanioTotal, desplazamiento = 0;

	int tamanioString = CHAR * strlen(mensaje->contenido);
	tamanioTotal = 3 * INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);

	memcpy(buffer + desplazamiento, &(mensaje->pid), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &(mensaje->paginas), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, mensaje->contenido, tamanioString);

	return buffer;
}

// entra:	int pid | int paginas | int tamanio | char *contenido
// sale:	inicioPrograma *mensaje
void *deserealizarSolicitudInicioPrograma(void *buffer) {

	int desplazamiento = 0, *tamanioString = NULL;
	inicioPrograma *mensaje = (inicioPrograma*)reservarMemoria(sizeof(inicioPrograma));

	memcpy( &(mensaje->pid), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( &(mensaje->paginas), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( tamanioString, buffer + desplazamiento, INT );
	desplazamiento += INT;

	mensaje->contenido = (char*)reservarMemoria(*tamanioString);
	memcpy( mensaje->contenido, buffer + desplazamiento, *tamanioString);

	return mensaje;
}

void * serializarRespuestaPedido(void * elemento){
	respuestaPedido* respuesta = (respuestaPedido*) elemento;
	int tamanioTotal, desplazamiento = 0;

	int tamanioExcepcion = CHAR * respuesta->mensaje.tamanio;
	int tamanioDataPedida = CHAR * respuesta->dataPedida.tamanio;
	tamanioTotal = 3*INT + tamanioExcepcion + tamanioDataPedida;

	void * buffer = reservarMemoria(tamanioTotal);
		memcpy(buffer + desplazamiento, &(respuesta->estadoPedido), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, &(respuesta->mensaje.tamanio), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, respuesta->mensaje.cadena, tamanioExcepcion);
			desplazamiento += tamanioExcepcion;
		memcpy(buffer + desplazamiento, &(respuesta->dataPedida.tamanio), INT);
			desplazamiento += INT;
		memcpy(buffer + desplazamiento, respuesta->dataPedida.cadena, tamanioDataPedida);

	return buffer;
}

respuestaPedido * deserializarRespuestaPedido(void * buffer){
	int desplazamiento = 0;
		respuestaPedido * respuesta = reservarMemoria(sizeof(respuestaPedido));

		memcpy(&respuesta->estadoPedido, buffer + desplazamiento, INT);
			desplazamiento += INT;

		memcpy(&respuesta->mensaje.tamanio, buffer + desplazamiento, INT);
			desplazamiento += INT;
		respuesta->mensaje.cadena = reservarMemoria(respuesta->mensaje.tamanio);
		memcpy(respuesta->mensaje.cadena, buffer + desplazamiento, respuesta->mensaje.tamanio);
			desplazamiento += respuesta->mensaje.tamanio;

		memcpy(&respuesta->dataPedida.tamanio, buffer + desplazamiento, INT);
			desplazamiento += INT;
		respuesta->dataPedida.cadena = reservarMemoria(respuesta->dataPedida.tamanio);
		memcpy(respuesta->dataPedida.cadena, buffer + desplazamiento, respuesta->dataPedida.tamanio);

	return respuesta;
}

// entra:	solicitudEscribirPagina *elemento
// sale:	int pid | int pagina | int tamanio | char *contenido
void *serealizarEscribirPagina(void *elemento) {

	solicitudEscribirPagina *mensaje = (solicitudEscribirPagina*)elemento;
	int tamanioTotal, desplazamiento = 0;

	int tamanioString = CHAR * strlen(mensaje->contenido);
	tamanioTotal = 3 * INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);

	memcpy(buffer + desplazamiento, &(mensaje->pid), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &(mensaje->pagina), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, mensaje->contenido, tamanioString);

	return buffer;
}

// entra:	int pid | int pagina | int tamanio | char *contenido
// sale:	solicitudEscribirPagina *mensaje
void *deserializarEscribirPagina(void * buffer) {

	int desplazamiento = 0, *tamanioString = NULL;
	solicitudEscribirPagina *mensaje = (solicitudEscribirPagina*)reservarMemoria(sizeof(solicitudEscribirPagina));

	memcpy( &(mensaje->pid), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( &(mensaje->pagina), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( tamanioString, buffer + desplazamiento, INT );
	desplazamiento += INT;

	mensaje->contenido = (char*)reservarMemoria(*tamanioString);
	memcpy( mensaje->contenido, buffer + desplazamiento, *tamanioString);

	return mensaje;
}

// entra:	devolverPagina *elemento
// sale:	int pagina | int tamanio | char *contenido
void *serealizarDevolverPagina(void *elemento) {

	devolverPagina *mensaje = (devolverPagina*)elemento;
	int tamanioTotal, desplazamiento = 0;

	int tamanioString = CHAR * strlen(mensaje->contenido);
	tamanioTotal = 2 * INT + tamanioString;

	void *buffer = reservarMemoria(tamanioTotal);

	memcpy(buffer + desplazamiento, &(mensaje->pagina), INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, &tamanioString, INT);
	desplazamiento += INT;

	memcpy(buffer + desplazamiento, mensaje->contenido, tamanioString);

	return buffer;
}

// entra:	int pagina | int tamanio | char *contenido
// sale:	devolverPagina *elemento
void *deserializarDevolverPagina(void *buffer) {

	int desplazamiento = 0, *tamanioString = NULL;
	devolverPagina *mensaje = (devolverPagina*)reservarMemoria(sizeof(devolverPagina));

	memcpy( &(mensaje->pagina), buffer + desplazamiento, INT );
	desplazamiento += INT;

	memcpy( tamanioString, buffer + desplazamiento, INT );
	desplazamiento += INT;

	mensaje->contenido = (char*)reservarMemoria(*tamanioString);
	memcpy( mensaje->contenido, buffer + desplazamiento, *tamanioString);

	return mensaje;
}
