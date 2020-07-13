#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#define READ_END    0    /* index pipe extremo escritura */
#define WRITE_END   1    /* index pipe extremo lectura */
#define nTub numTuberias-1


int cuentaMayor(char *cadena);

int cuentaPipes(char * cadena);

int dividirCadena(char* delimitador, char* cadena, char ** cads);

void redireccionamiento(char ** cadenaRe, char ** cads, int banderaMenorque, int banderaRedir, int fi, int fo, int r);

void salida(int banderaRedir, int banderaMenorque, char** cadenaRe, char** cads, int r);

void quitaEspacio( char **  cadenaRe);

/*
//      archivo1.txt ------> archivo2.txt
//  archivo2.txt ------> pwd :v
//      cat < archivo1.txt | ls -l | wc >> archivo3.txt
sl | wc < hola.txt
ls | sort -n | wc > archivo.txt
cal -m 2 >> archivo.txt
*/

int main(int argc, char *argv[]){
	int pid, status;
	int banderaRedir = 0;
	int banderaMenorque=0;
	int numTuberias = 1;
	int z=0;

	FILE *fp;
	//char * Promp="Promp.txt";

	char * cadena={"NUEVA"};//cadena ingresada por el ususario 
	char *cads[200];//cadena resultado de dividir -> hola >> que
	char * cadenaRe[200];//cadena resultado de dividir hola < que.txt
	char * cadenaPipes[200];
	char  cadenaPromp[100];
	char lectura;
	cadena=(char *)(malloc(sizeof(char)));
	
	if((fp=fopen("Prompt.txt", "r"))==NULL){
		printf("No se puede abrir el archivo");
		exit(-1);
	}
	
	lectura=getc(fp);
	
	while(lectura!='\n'){
		cadenaPromp[z]=lectura;
		z++;
		lectura=getc(fp);
	}

	fclose(fp);

	while(1){

		printf("%s > ", cadenaPromp);
		scanf(" %[^\n]", cadena);
		if(!((strcmp(cadena, "exit"))!=0)){
			exit(-2);
		}

		char* incidencias[]={">", ">>", "<"};//busca las incidencias
		int r=0;

		if(dividirCadena(incidencias[0], cadena, cads)){//encontro una incidencia de >> o >
			banderaRedir++;//la bandera vale 1 para >>, >
		}
		r = cuentaMayor(cadena);
		cadena = cads[0];

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

		int i = cuentaPipes(cadena);//cuenta el numero de pipes del comando

		if(i==0){//no hay pipes

			if(dividirCadena(incidencias[2], cadena, cadenaRe)){//encontro una incidencia de <
				banderaMenorque++;//la bandera de <
			}
			salida(banderaRedir,banderaMenorque, cadenaRe,cads,r);

		}else{//hay pipes
			dividirCadena("|", cadena, cadenaPipes);
			if(dividirCadena(incidencias[2], cadenaPipes[0], cadenaRe)){//encontro una incidencia de <
				banderaMenorque++;//la bandera de <
			}
			int x = 0, j=0, v = 1;

			int tuberias[i][2];

			for(x = 0; x < i; x++){
				pipe(tuberias[x]);
			}
			
			do{
				char * argumentos[200];//comando y argumentos del comando
				int ultimo;
				if(v == 1){
					ultimo = dividirCadena(" ", cadenaRe[0], argumentos);//recibe la ultima posicion de argumentos
					argumentos[ultimo] = NULL;//ponemos un nulo al final del vector para exec
				} else{
					ultimo = dividirCadena(" ", cadenaPipes[v-1], argumentos);//recibe la ultima posicion de argumentos
					argumentos[ultimo] = NULL;//ponemos un nulo al final del vector para exec
				}
				
				pid=fork();
				
				if(pid==0){
					if(v==1){//solo escritura, primer tuberia
						int fi = 0, fo = 0;
						close(tuberias[j][READ_END]);
						dup2(tuberias[j][WRITE_END], STDOUT_FILENO); 
						close(tuberias[j][WRITE_END]);
						redireccionamiento(cadenaRe, cads, banderaMenorque, 0, fi, fo, r);
						execvp(argumentos[0],argumentos);
					}
					if(v==i+1){//tuberia extreectura
						j--;
						int fi = 0, fo = 0;
								//close(tuberias[j][WRITE_END]);
						dup2(tuberias[j][READ_END], STDIN_FILENO);
						close(tuberias[j][READ_END]);
						redireccionamiento(cadenaRe, cads, 0, banderaRedir, fi, fo, r);
						execvp(argumentos[0], argumentos);
					}
					else{//tuberia media, lectura y escritura
						
						dup2(tuberias[j-1][READ_END], STDIN_FILENO);
						close(tuberias[j-1][READ_END]);

						close(tuberias[j][READ_END]);	
						dup2(tuberias[j][WRITE_END], STDOUT_FILENO);			
						close(tuberias[j][WRITE_END]);
						execvp(argumentos[0], argumentos);
					}
							
				}		 
				else{//Proceso padre
					wait(&status);
					if(v==1){//primer tuberia
						close(tuberias[j][WRITE_END]);
					}    
							
					if(v==i+1) {//ULTIMA TUBERIA, TALVEZ OPCIONAL
						close(tuberias[j][READ_END]);
					}
					else{//segunda tuberia
						close(tuberias[j-1][READ_END]);
						close(tuberias[j][WRITE_END]);
					}
					

				}
				v++;		
				j++;
			}while(v<=i+1);
		}
		banderaRedir = 0;
		banderaMenorque=0;
		cads[0]=NULL;
		cads[1]=NULL;
	}
	
	return 0;
}


int cuentaMayor(char *cadena){
	int cont = 0;
	int x=0;
	for(x=0; cadena[x]!='\0'; x++){
			if(cadena[x]=='>'){
					cont ++;
			}
	}

	return cont;
}

int dividirCadena(char* delimitador, char* cadena, char ** cads){
	int x=0;
	char *token, * theRest;
	char *original;

	original=strdup(cadena);
	theRest=original;

	while(token = strtok_r(theRest,delimitador, &theRest)){

			cads[x]=token;
			x++;
	}

	if(strlen(cads[0])==strlen(cadena)){
			return (delimitador == " ") ? 1 : 0;
			//no hubo coincidencias
	}
	else{
			return (delimitador == " ") ? x : 1; //hubo coincidencias
	}

}

int cuentaPipes(char * cadena){
	int i, cont = 0;

	for(i = 0; cadena[i]; i++){
			if(cadena[i] == '|') cont++;
	}

	return cont;
}

void redireccionamiento(char ** cadenaRe, char** cads, int banderaMenorque, int banderaRedir, int fi, int fo, int r){
	char * sinE[10];
	if(banderaMenorque){//redireccionamos la entrada
		dividirCadena(" ", cadenaRe[1], sinE);//quitamos espacios al archivo
		fi = open(sinE[0], O_RDONLY);//(fichero de entrada, lo abre con permisos de lectura)
		dup2(fi, STDIN_FILENO);//redireccion del fichero a la entrada
		close(fi);
	}
	if(banderaRedir){ //redireccionamos la salida
		dividirCadena(" ", cads[1], sinE);//quitamos espacios al archivo
		int flags[] = {(O_RDWR | O_CREAT | O_TRUNC), (O_RDWR | O_CREAT | O_APPEND)};//(>, >>)
		int banderaCrear = (S_IRUSR | S_IWUSR | S_IXUSR);// si se crea, dar permisos al usario
		fo = open(sinE[0], flags[r-1], banderaCrear);
		dup2(fo, STDOUT_FILENO);//redireccionamiento de la salida
		close(fo);
	}
}

void salida(int banderaRedir, int banderaMenorque, char** cadenaRe, char** cads, int r){
	int pid = 0, i, status;
	char * argumentos[200];//comando y argumentos del comando
	int ultimo = dividirCadena(" ", cadenaRe[0], argumentos);//recibe la ultima posicion de argumentos

	argumentos[ultimo] = NULL;//ponemos un nulo al final del vector para exec

	pid = fork();

	if(pid == 0){
			int fi = 0, fo = 0;//ficheros de descriptor de archivos
			redireccionamiento(cadenaRe, cads, banderaMenorque, banderaRedir, fi, fo, r);
			execvp(argumentos[0], argumentos);
	} else{
			wait(&status);
	}
}
