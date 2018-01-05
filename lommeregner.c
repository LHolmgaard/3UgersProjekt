#include "stdio.h"

double result=0;
double a=0;
double b=0;

void addition(double a, double b);
void subtraction(double a, double b);
void mult(double a, double b);
void division(double a, double b);


int main(){
    int counter = 0;
    printf("Start lommeregner ved at taste 1, afslut ved at taste 0");
    scanf("%d",&counter);

    if (counter==1){
        while(counter==1){
            char symbol;
            printf("Indtast tal");
            scanf("%lf", &a);
            getchar();

            printf("\nIndtast symbol");
            scanf(" %c", &symbol);
            getchar();

            printf("\nindtast andet tal");
            scanf("%lf", &b);
            getchar();

            switch(symbol){
                case '+':
                    addition(a,b);
                    printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a, symbol,b,result);
                    break;
                case '-':
                    subtraction(a,b);
                    printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a,symbol,b,result);
                    break;
                case '*':
                    mult(a,b);
                    printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a,symbol,b,result);
                    break;
                case '/':
                    division(a,b);
                    printf("Resultat er: %.2lf %c %.2lf =  %.2lf\n", a,symbol,b,result);
                    break;
                default:
                    printf("Forkert indtastet operator\n");
                    break;
            }

            printf("Fortsæt eller afslut?\n 1 for fortsæt, 0 for afslut");

            scanf("%d",&counter);
        }
    }
    else if (counter ==0 ) {printf("lommeregner afsluttes");}

    return 0;
}

void addition(double a, double b){
    result= a + b;

}
void subtraction(double a, double b){
    result=(a- b);

}
void mult(double a, double b){
    result=(a * b);

}
void division(double a, double b){
    result=(a / b);

}