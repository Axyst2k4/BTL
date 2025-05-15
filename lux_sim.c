#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#define MIN_sensor 0.10
#define MAX_sensor 100000.00
void Error(void){
    printf("Error!");
    exit(1);
}

void input_data(int count ,char*arr[],int* n, int* s, int* i){
    *n = 1;
    *s = 60;
    *i = 24;
    if(count % 2 == 0 || count > 7) {
        Error();
    } else
{
    for( int k=1; k<=count-1; k+=2){
        for(int j=0; j < strlen(arr[k+1]);j++ ){
            if (!isdigit(arr[k+1][j])){ 
                Error();      
            } else
                continue;
        }

        if (strcmp(arr[k],"-n")==0){
             *n = atoi(arr[k+1]);
             if(*n<1) Error();
                     
        } else if (strcmp(arr[k],"-s")==0){
             *s = atoi(arr[k+1]);
             if(*s<1) Error();
                          
        } else if (strcmp(arr[k],"-i")==0){
             *i = atoi(arr[k+1]);
             if(*i<1) Error();
                        
        } else {
            Error();
        } 
            
    }
}
}


void read_time(int n, int s,int i){
    FILE *file = fopen("lux_sim.csv", "w");
    fprintf(file, "id,time,value\n");
    time_t now;
    char time_str[30];
    time(&now);
    time_t start =now - i*3600;
    while(start<=now){
        //ime_t time_str=start;
        strftime(time_str, sizeof(time_str), "%Y:%m:%d %H:%M:%S", localtime(&start));
        
        for (int id =1; id<=n;id++){
           float value_sensor;           
            value_sensor = MIN_sensor + (rand()/(float)RAND_MAX)*(MAX_sensor-MIN_sensor);
            fprintf(file,"%d,",id);
            fprintf(file,"%s,",time_str);
            fprintf(file,"%.2f\n",value_sensor);
        }
        start = start + s ;
    }
    fclose(file);
}
int main(int count, char*arr[]){
    int n, s, i;
    n = 1;
    s = 60;
    i = 24;
   

    srand(time(NULL));
    input_data(count,arr, &n, &s, &i);
    
    read_time(n,s,i);
    
    return 0;

}