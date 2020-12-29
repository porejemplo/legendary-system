#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include "cabeceras.h"

#define LONGITUD_COMANDO 100

void Printbytemaps(EXT_BYTE_MAPS *ext_bytemaps){
	printf("Inodos: ");
	for (int i = 0; i < MAX_INODOS; i++){
		printf(" %d ", ext_bytemaps->bmap_inodos[i]);
	}
	printf("\nBloques[0-25]: ");
	for (int i = 0; i < 25; i++){
		printf(" %d ", ext_bytemaps->bmap_bloques[i]);
	}
	printf("\n");
};
// Devuelve 0 si el comando es valido.
int ComprobarComando(char *strcomando, char *orden, char *argumento1, char *argumento2){
	int r = 1;
	int contComandos = 0;
	// Sacar la orden y los argumentos.
	const char s[2] = " ";
	char *token;
	
	// Coger el primer comando.
	token = strtok(strcomando, s);

	for (contComandos=0; contComandos<3 && token!=NULL; contComandos++){
		if (contComandos == 0)
			strcpy (orden, token);
		else if (contComandos == 1)
			strcpy (argumento1, token);
		else if (contComandos == 2)
			strcpy (argumento2, token);
			
		r=0;

		token = strtok(NULL, s);
	}
	//Comprobar si la orden es correcta
	if (r == 0){
		if(strcmp(orden,"info\n")!=0 && strcmp(orden,"bytemaps\n")!=0 && strcmp(orden,"dir\n")!=0 && strcmp(orden,"rename\n")!=0 && strcmp(orden,"imprimir\n")!=0 && strcmp(orden,"remove\n")!=0 && strcmp(orden,"copy\n")!=0 && strcmp(orden,"salir\n")!=0){
			r=1;
		}
	}
	
	return r;
}

void LeeSuperBloque(EXT_SIMPLE_SUPERBLOCK *psup){
	printf("Bloque %d\n", psup->s_block_size);
	printf("inodos Particion = %d\n", psup->s_inodes_count);
	printf("inodos libres = %d:\n", psup->s_free_inodes_count);
	printf("Bloques particion = %d\n", psup->s_blocks_count);
	printf("Bloques libres = %d:\n", psup->s_free_blocks_count);
	printf("Primer bloque de datos = %d:\n", psup->s_first_data_block);
};

// Devuelve 0 si no encuentra el fichero.
int BuscaFich(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombre){
	int r = 0;

	//Buscar el ficehro.
	printf("\tSe bucsa el ficehro %s.\n", &nombre);

	return 0;
}

void Directorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos){
	for(int i = 1; i < MAX_FICHEROS;i++){
		if(directorio[i].dir_inodo != NULL_INODO){
		printf("Fichero: %s	tamaño: %d	inodo: %d	bloques: ", directorio[i].dir_nfich, inodos->blq_inodos[directorio[i].dir_inodo].size_fichero, directorio[i].dir_inodo);
		for(int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++){
			if(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE){
			printf(" %d ", inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]);
			}
		}
		printf("\n");
		}
	}
	printf("\n");
}

// Cambia el nomrbe de un fichero por otro.
//  0 - si se ha podido cambair el nombre del fichero.
//  1 - no se encuentra el fichero.
// -1 - ya hay un fichero con el nuevo nombre.
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
	int r = 0;
	if (BuscaFich(&directorio, &inodos, nombreantiguo) != 0){
		r=1;
	}
	if (r==0 && BuscaFich(&directorio, &inodos, nombrenuevo) == 0){
		r=-1;
	}

	if (r==0){
		printf("\tSe cambian los nombres de los ficheros.\n");
	}

	return r;
}

// Devuelve 0 si se puede imprimir.
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
	int r=0;
	if (BuscaFich(&directorio, &inodos, nombre) != 0){
		r=1;
	}

	if (r==0){
		printf("\tSe imprime el contenido del fichero %s.\n", &nombre);
	}

	return r;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
	int r=0;
	if (BuscaFich(&directorio, &inodos, nombre) != 0){
		r=1;
	}

	if (r==0){
		printf("\tSe borra el fichero %s.\n", &nombre);
	}

	return r;
}

// 0	Se copia sin problemas.
// 1	No existe el origen.
// -1	Ya existe el archivo
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){
	int r=0;
	if (BuscaFich(&directorio, &inodos, nombreorigen) != 0){
		r=1;
	}
	if (r==0 && BuscaFich(&directorio, &inodos, nombredestino) == 0){
		r=-1;
	}

	if (r==0){
		printf("\tSe copia %s en %s.\n", &nombreorigen, &nombredestino);
	}

	return r;
}
void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	char *comando[LONGITUD_COMANDO];
	char *orden[LONGITUD_COMANDO];
	char *argumento1[LONGITUD_COMANDO];
	char *argumento2[LONGITUD_COMANDO];

	int i,j,f;
	unsigned long int m;
	EXT_SIMPLE_SUPERBLOCK ext_superblock;
	EXT_BYTE_MAPS ext_bytemaps;
	EXT_BLQ_INODOS ext_blq_inodos;
	EXT_ENTRADA_DIR directorio[MAX_FICHEROS];
	EXT_DATOS memdatos[MAX_BLOQUES_DATOS];
	EXT_DATOS datosfich[MAX_BLOQUES_PARTICION];
	int entradadir;
	int grabardatos;
	FILE *fent;

	// Lectura del fichero completo de una sola vez

	fent = fopen("particion.bin","r+b");
	fread(&datosfich, SIZE_BLOQUE, MAX_BLOQUES_PARTICION, fent);    


	memcpy(&ext_superblock,(EXT_SIMPLE_SUPERBLOCK *)&datosfich[0], SIZE_BLOQUE);
	memcpy(&directorio,(EXT_ENTRADA_DIR *)&datosfich[3], SIZE_BLOQUE);
	memcpy(&ext_bytemaps,(EXT_BLQ_INODOS *)&datosfich[1], SIZE_BLOQUE);
	memcpy(&ext_blq_inodos,(EXT_BLQ_INODOS *)&datosfich[2], SIZE_BLOQUE);
	memcpy(&memdatos,(EXT_DATOS *)&datosfich[4],MAX_BLOQUES_DATOS*SIZE_BLOQUE);

	// Buce de tratamiento de comandos
	for (;;){
		do {
			printf (">> ");
			fflush(stdin);
			fgets(comando, LONGITUD_COMANDO, stdin);
			i = ComprobarComando(comando,orden,argumento1,argumento2);
			if (i==1)
				printf ("ERROR: Comando ilegal [bytemaps,copy,dir,info,imprimir,rename,remove,salir]\n");
		} while (i!=0);

		if (strcmp(orden,"dir\n")==0) {
			printf("Comando dir.\n");
			Directorio(&directorio,&ext_blq_inodos);
			continue;
		}
		else if (strcmp(orden,"bytemaps\n")==0) {
			printf("Comando Bytemaps.\n");
			Printbytemaps(&ext_bytemaps);
			continue;
		}
		else if (strcmp(orden,"copy\n")==0) {
			printf("Comando Copy.\n");
			//Directorio(&directorio,&ext_blq_inodos);
			continue;
		}
		else if (strcmp(orden,"info\n")==0) {
			printf("Comando Informacion.\n");
			LeeSuperBloque(&ext_superblock);
			continue;
		}
		else if (strcmp(orden,"imprimir\n")==0) {
			i = Imprimir(&directorio, &ext_blq_inodos, &memdatos, argumento1);

			if (i>0)
				printf("ERROR: Fichero %s no encontrado.\n", &argumento1);
			
			continue;
		}
		else if (strcmp(orden,"rename\n")==0) {
			i = Renombrar(&directorio, &ext_blq_inodos, argumento1, argumento2);

			if (i<0)
				printf("ERROR: El fichero %s ya existe.\n", &argumento2);
			else if (i>0)
				printf("ERROR: Fichero %s no encontrado.\n", &argumento1);
			
			continue;
		}
		else if (strcmp(orden,"remove\n")==0) {
			i = Borrar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);

			if (i>0)
				printf("ERROR: Fichero %s no encontrado.\n", &argumento1);
			
			continue;
		}

		// Escritura de metadatos en comandos rename, remove, copy     
		/*Grabarinodosydirectorio(&directorio,&ext_blq_inodos,fent);
		GrabarByteMaps(&ext_bytemaps,fent);
		GrabarSuperBloque(&ext_superblock,fent);
		if (grabardatos)
			GrabarDatos(&memdatos,fent);
		grabardatos = 0;*/
		//Si el comando es salir se habrán escrito todos los metadatos
		//faltan los datos y cerrar
		if (strcmp(orden,"salir\n")==0){
			//GrabarDatos(&memdatos,fent);
			fclose(fent);
			return 0;
		}
	}
}