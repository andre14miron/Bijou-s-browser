#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define INCR 3

enum color { white, black, red, green, blue, yellow };

typedef struct site_data{
    char URL[50];
    int nr_access;
    int checksum;
    int length;
    char title[50];
    char *content;
    char *code;
    enum color color_text;
    enum color color_background;
}site_data;

/* citirea datelor dintr-un fisier f si memorarea lor intr-o structura site de
tip site_data */
void read_site_data(site_data * site, FILE *f)
{
    fscanf(f, "%s", site->URL);         // citire URL
    fscanf(f, "%d", &site->length);     // citire lungimea in octeti a codului
    fscanf(f, "%d", &site->nr_access);  // citire numarul de accesari
    fscanf(f, "%d", &site->checksum);   // citire checksum

    char t[100];      // variabila folosita pentru citirea unei linii
    fgets(t, 100, f); // citeste ce a ramas din prima linie

    /* citire codul HTML */
    site->code = (char *) calloc(site->length+1,sizeof(char));
    while(fgets(t, 100, f) != NULL) 
        strcat(site->code, t);
}

/* identificarea CSS pentru un element */
void CSS(site_data *site, char *style)
{
    char c[6][7]={"white","black","red","green","blue","yellow"}; 
    char *ptext,*pback;
    int i;
    // verificare daca se specifica culoarea fundalului si aflarea ei
    pback=strstr(style,"background-color");
    if(pback!=NULL){
        pback[23]='\0';        // delimitam secventa
        for(i=0; i<6; i++)     // cautare culoare
            if(strstr(pback,c[i])!=NULL)
                site->color_background = i; 
    }

    // verificare daca se specifica culoarea textului si aflarea ei
    ptext=strstr(style,"color");
    if(ptext!=NULL && style[ptext-style-1]!='-'){
        ptext[12]='\0';        // delimitam secventa
        for(i=0; i<6; i++)     // cautare culoare
            if(strstr(ptext,c[i])!=NULL)
                site->color_text = i; 
    }
}

/* obtinere titlu, CSS si continut dupa parsarea codului HTML */ 
void parsing_html(site_data *site)
{
    /// initializam culorile textului si al fundalului
    site->color_text=black;  
    site->color_background=white;

    /// in variabila text se copiaza codul HTML pentru a se efectua parsarea 
    char *text = (char *) calloc(strlen(site->code)+1,sizeof(char));  
    strcpy(text, site->code);

    int i=0; 
    char *p=strtok(text,"<>");
    while(p){
        /// obtinere titlu 
        if(i==3) 
            strcpy(site->title,p);

        /// obtinere CSS 
        if(i==6)
            if(strlen(p)!=1)
                CSS(site,p); 

        /// obtinere continut 
        if(i==7){
            site->content = (char *) calloc(strlen(p)+1,sizeof(char));
            strcpy(site->content,p);
        }
        p=strtok(NULL,"<>");
        i++;
    }
    free(text);
} 

int main()
{   
    FILE *fm, *f;
    fm = fopen ( "master.txt", "r" );
    
    /// cap reprez. capacitatea curenta a vectorului de structuri
    /// n reprez. numarul site-urilor citite din master.txt
    int cap=INCR, n=0; 
    site_data *site; 
    site = (site_data *) calloc(cap,sizeof(site_data));

    /* citire din master.txt si citirea datelor din fisiere */
    char s[30];
    while ( fscanf(fm,"%s",s) != EOF ){
        f = fopen (s, "r"); 
        
        /// realoca memorie daca nu exista destula memorie
        if(n == cap-1){
            cap = cap + INCR;
            site = (site_data *) realloc(site,cap*sizeof(site_data));
        }
        
        read_site_data(&site[n], f); /// citeste datele din fisier
        fclose(f);
        parsing_html(&site[n]);      /// parsarea elementelor HTML 
        n++;
    }
    fclose(fm);
    site = (site_data *) realloc(site,n*sizeof(site_data));

    /* afisare URL, numarul de accesari si titlul pentru fiecare site */
    int i;
    for(i=0; i<n; i++)
        printf("%s %d %s\n",site[i].URL,site[i].nr_access,site[i].title);

    /* eliberarea memoriei */
    for(i=0; i<n; i++){
        free(site[i].content);
        free(site[i].code);
    }
    free(site);
    
    return 0;
}
