#include "fcpu.h"


// Funciones
void setearValores_config(t_config * archivoConfig){
	config = (t_configuracion*)reservarMemoria(sizeof(t_configuracion));
	config->ipNucleo = strdup(config_get_string_value(archivoConfig, "IP_NUCLEO"));
	config->ipUMC = strdup(config_get_string_value(archivoConfig, "IP_UMC"));
	config->puertoNucleo = config_get_int_value(archivoConfig, "PUERTO_NUCLEO");
	config->puertoUMC = config_get_int_value(archivoConfig, "PUERTO_UMC");
}

void conectarConNucleo() {
	fd_clienteNucleo = nuevoSocket();
	int ret = conectarSocket(fd_clienteNucleo, config->ipNucleo, config->puertoNucleo);
	validar_conexion(ret, 1); // Es terminante por ser cliente
	handshake_cliente(fd_clienteNucleo, "P");

	int * head = (int*)malloc(INT);
	pcb * pcbEnEjecucion;
	void * mensaje = aplicar_protocolo_recibir(fd_clienteNucleo, *head);

	while(mensaje!=NULL){
		switch(*head){

		case ENVIAR_PCB:
			pcbEnEjecucion = recibirPCB(mensaje); // el cpu obtiene una pcb para ejecutar
			// hacer algo con la pcb... (alguna función del cpu)
			break;
		}
		free(mensaje);

	mensaje = aplicar_protocolo_recibir(fd_clienteNucleo, *head); // recibe otro mensaje del Núcleo
	}
	free(head);
	cerrarSocket(fd_clienteNucleo);
}

pcb * recibirPCB(void * mensaje){
pcb * nuevoPCB = (pcb*)malloc(sizeof(pcb));
// deserealizaicón
memcpy(nuevoPCB, mensaje, sizeof(pcb));

return nuevoPCB;
}

void conectarConUMC(){
	fd_clienteUMC = nuevoSocket();
	int ret = conectarSocket(fd_clienteUMC, config->ipUMC, config->puertoUMC);
	validar_conexion(ret, 1);
	handshake_cliente(fd_clienteUMC, "P");

	int * tamPagina = (int*)malloc(INT);
	recibirPorSocket(fd_clienteUMC, tamPagina, INT);
	tamanioPagina = *tamPagina; // setea el tamaño de pág. que recibe de UMC

	cerrarSocket(fd_clienteUMC);
}

void liberarEstructura() {
	free(config->ipNucleo);
	free(config->ipUMC);
	free(config);
}

int validar_servidor(char *id) {
	if( !strcmp(id, "U") || !strcmp(id, "N") ) {
		printf("Servidor aceptado\n");
		return TRUE;
	} else {
		printf("Servidor rechazado\n");
		return FALSE;
	}
}

int validar_cliente(char *id) {return 0;}


/*void esperar_ejecucion() {

	uint8_t *head = (uint8_t*)reservarMemoria(1); // 0 .. 255

	while(TRUE) {
		int ret;
		ret = recibirPorSocket(fd_clienteNucleo, head, INT);
		validar_recive(ret, 1); // es terminante ya que si hay un error en el recive o desconexión debe terminar
		aplicar_protocolo_recibir(fd_clienteNucleo, *head);
	}
	free(head);
}*/

// Primitivas AnSISOP
t_puntero definirVariable(t_nombre_variable identificador_variable){
	t_puntero posicion;

	return posicion;
}

t_puntero obtenerPosicionVariable(t_nombre_variable identificador_variable){
	t_puntero posicion;

	return posicion;
}

t_valor_variable dereferenciar(t_puntero direccion_variable){
	t_valor_variable valorVariable;

	return valorVariable;
}

void asignar(t_puntero direccion_variable, t_valor_variable valor){

}

t_valor_variable obtenerValorCompartida(t_nombre_compartida variable){
	t_valor_variable valorVariable;

	return valorVariable;
}

t_valor_variable asignarValorCompartida(t_nombre_compartida variable, t_valor_variable valor){
	t_valor_variable valorVariable;

	return valorVariable;
}

void irAlLabel(t_nombre_etiqueta t_nombre_etiqueta){

}

void llamarConRetorno(t_nombre_etiqueta etiqueta, t_puntero donde_retornar){

}

void retornar(t_valor_variable retorno){

}

void imprimirAnSISOP(t_valor_variable valor_mostrar){

}

void imprimirTexto(char* texto){

}

void entradaSalida(t_nombre_dispositivo dispositivo, int tiempo){

}

void wait(t_nombre_semaforo identificador_semaforo){

}

void signal(t_nombre_semaforo identificador_semaforo){

}
