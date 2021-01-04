#include<stdio.h>
#include<string.h>
#include<ctype.h>
#include <stdlib.h>
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
		if(strcmp(orden,"info")!=0 && strcmp(orden,"bytemaps")!=0 && strcmp(orden,"dir")!=0 && strcmp(orden,"rename")!=0 && strcmp(orden,"imprimir")!=0 && strcmp(orden,"remove")!=0 && strcmp(orden,"copy")!=0 && strcmp(orden,"salir")!=0){
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
	for(int i = 1; i < MAX_FICHEROS;i++){
		if(directorio[i].dir_inodo != NULL_INODO){
			if (strcmp(directorio[i].dir_nfich,nombre)==0){
				r=directorio[i].dir_inodo;
				break;
			}
		}
	}
	return r;
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
//  1 - no se encuentra el fichero original.
// -1 - ya hay un fichero con el nuevo nombre.
int Renombrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, char *nombreantiguo, char *nombrenuevo){
	int r = BuscaFich(directorio, inodos, nombrenuevo);
	if (r > 0){
		r=-1;
	}
	if (r == 0){
		r = BuscaFich(directorio, inodos, nombreantiguo);
	}

	if (r > 0){
		//memcpy(directorio[i].dir_nfich, nombrenuevo, LEN_NFICH);
		for(int i = 1; i < MAX_FICHEROS;i++){
			if(directorio[i].dir_inodo != NULL_INODO){
				if (strcmp(directorio[i].dir_nfich,nombreantiguo)==0){
					memcpy(directorio[i].dir_nfich, nombrenuevo, LEN_NFICH);
					break;
				}
			}
		}
	}

	return r;
}

// Devuelve 0 si se puede imprimir.
int Imprimir(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_DATOS *memdatos, char *nombre){
	int r=BuscaFich(directorio, inodos, nombre);
	if (r!=0){
		for(int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++){
			if(inodos->blq_inodos[r].i_nbloque[j] != NULL_BLOQUE){
				printf("%s ", memdatos[inodos->blq_inodos[r].i_nbloque[j]-4].dato);
			}
		}
		printf("\n");
	}

	return r;
}

int Borrar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, char *nombre,  FILE *fich){
	int r=BuscaFich(directorio, inodos, nombre);
	
	if (r!=0){
		printf("\tSe borra el fichero %s.\n", nombre);
		for(int i = 1; i < MAX_FICHEROS;i++){
			if(directorio[i].dir_inodo != NULL_INODO){
				if (strcmp(directorio[i].dir_nfich,nombre)==0){
					memcpy(directorio[i].dir_nfich,"       ", LEN_NFICH);
					inodos->blq_inodos[directorio[i].dir_inodo].size_fichero=0;
					for(int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++){
						if(inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j] != NULL_BLOQUE){
							ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]]=0;
							inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]=NULL_BLOQUE;
						}
					}
					ext_bytemaps->bmap_inodos[directorio[i].dir_inodo]=0;
					directorio[i].dir_inodo=NULL_INODO;
					break;
				}
			}
		}
	}

	return r;
}

// 0	Se copia sin problemas.
// 1	No existe el origen.
// -1	Ya existe el archivo
int Copiar(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, EXT_BYTE_MAPS *ext_bytemaps, EXT_SIMPLE_SUPERBLOCK *ext_superblock, EXT_DATOS *memdatos, char *nombreorigen, char *nombredestino,  FILE *fich){
	int r = BuscaFich(directorio, inodos, nombredestino);
	if (r > 0){
		r=-1;
	}
	if (r == 0){
		r = BuscaFich(directorio, inodos, nombreorigen);
	}

	if (r > 0){
		printf("\tSe copia %s en %s.\n", nombreorigen, nombredestino);
		
		for(int i = 1; i < MAX_FICHEROS;i++){
			if(directorio[i].dir_inodo == NULL_INODO){
				for(int j = 3; j < MAX_NUMS_BLOQUE_INODO; j++){
					if(inodos->blq_inodos[j].size_fichero== 0){
						directorio[i].dir_inodo=j;
						ext_bytemaps->bmap_inodos[directorio[i].dir_inodo]=1;
						break;	
					}
				}
				memcpy(directorio[i].dir_nfich,nombredestino, LEN_NFICH);
				inodos->blq_inodos[directorio[i].dir_inodo].size_fichero=inodos->blq_inodos[r].size_fichero;
				int cont=2;
				for(int j = 0; j < MAX_NUMS_BLOQUE_INODO; j++){
					if(inodos->blq_inodos[r].i_nbloque[j] != NULL_BLOQUE){
						do{
							cont++;
						}while(ext_bytemaps->bmap_bloques[cont]==1);
						memcpy(memdatos[cont].dato,memdatos[inodos->blq_inodos[r].i_nbloque[j]-4].dato,MAX_BLOQUES_DATOS);
						inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]=cont;
						ext_bytemaps->bmap_bloques[inodos->blq_inodos[directorio[i].dir_inodo].i_nbloque[j]]=1;
					}
				}
				break;
			}
		}
	}

	return r;
}

char* LeerLineaDinamica ( int tamanoMaximo )
{
	int tamanoLinea = 1;
	char* linea = NULL;
	linea = (char*) malloc (sizeof(char) * tamanoLinea);
	char c = '\0';

	c = getc(stdin);
	while ( c != '\n')
	{
		if (tamanoLinea < tamanoMaximo)
		{	
			linea[tamanoLinea-1] = c;
			++tamanoLinea;
			linea = (char*) realloc (linea, tamanoLinea);
		}
		c = getc(stdin);
	}
	linea[tamanoLinea -1] = '\0';

	return linea;
}

void Grabarinodosydirectorio(EXT_ENTRADA_DIR *directorio, EXT_BLQ_INODOS *inodos, FILE *fich);
void GrabarByteMaps(EXT_BYTE_MAPS *ext_bytemaps, FILE *fich);
void GrabarSuperBloque(EXT_SIMPLE_SUPERBLOCK *ext_superblock, FILE *fich);
void GrabarDatos(EXT_DATOS *memdatos, FILE *fich);

int main()
{
	char *comando;//[LONGITUD_COMANDO];
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
	/*for(int i = 0 ; i<10; i++){
		printf("%s", memdatos[i].dato);
	}*/
	// Buce de tratamiento de comandos
	for (;;){
		do {
			printf (">> ");
			//fflush(stdin);
			//fgets(comando, LONGITUD_COMANDO, stdin);
			
			comando = LeerLineaDinamica(LONGITUD_COMANDO);
			
			i = ComprobarComando(comando,orden,argumento1,argumento2);
			free(comando);
			if (i==1)
				printf ("ERROR: Comando ilegal [bytemaps,copy,dir,info,imprimir,rename,remove,salir]\n");
		} while (i!=0);

		if (strcmp(orden,"dir")==0) {
			printf("Comando dir.\n");
			Directorio(&directorio,&ext_blq_inodos);
			continue;
		}
		else if (strcmp(orden,"bytemaps")==0) {
			printf("Comando Bytemaps.\n");
			Printbytemaps(&ext_bytemaps);
			continue;
		}
		else if (strcmp(orden,"copy")==0) {
			i=Copiar(&directorio,&ext_blq_inodos,&ext_bytemaps,&ext_superblock,&memdatos,argumento1,argumento2,fent);
			if (i < 0)
				printf("ERROR: El fichero %s ya existe.\n", &argumento2);
			else if (i == 0)
				printf("ERROR: Fichero %s no encontrado.\n", &argumento1);
			
			continue;
		}
		else if (strcmp(orden,"info")==0) {
			printf("Comando Informacion.\n");
			LeeSuperBloque(&ext_superblock);
			continue;
		}
		else if (strcmp(orden,"imprimir")==0) {
			i = Imprimir(&directorio, &ext_blq_inodos, &memdatos, argumento1);

			if (i==0)
				printf("ERROR: Fichero %s no encontrado.\n", &argumento1);
			
			continue;
		}
		else if (strcmp(orden,"rename")==0) {
			i = Renombrar(&directorio, &ext_blq_inodos, argumento1, argumento2);

			if (i < 0)
				printf("ERROR: El fichero %s ya existe.\n", &argumento2);
			else if (i == 0)
				printf("ERROR: Fichero %s no encontrado.\n", &argumento1);
			
			continue;
		}
		else if (strcmp(orden,"remove")==0) {
			i = Borrar(&directorio, &ext_blq_inodos, &ext_bytemaps, &ext_superblock, argumento1, fent);

			if (i==0)
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
		if (strcmp(orden,"salir")==0){
			//GrabarDatos(&memdatos,fent);
			fclose(fent);
			return 0;
		}
	}
}
