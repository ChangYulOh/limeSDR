#include <stdio.h>         // puts()
#include <string.h>        // strlen()
#include <fcntl.h>         // O_WRONLY
#include <unistd.h>        // write(), close()
#include <math.h>
#include <complex>
#include <iostream>
#include <fftw3.h>

#define REAL 0
#define IMAG 1

#define max_fft_samp_rate 11.77e6

static float samp_rate=20e6;

//gnuradio's(or lms7) max fft rate might 11.4mhz...

/*

                    //주기를 알기위한 갯수카운팅..
                   if(tmp>0.2){//0.08){
                       if(count==0 && count2>50000){
                           printf("%d\n",count2+count+count1);
                           count=0;
                           count2=0;
                           count1=0;
                       }

                     count++;
                     if(count>10){
                         count1+=count;
                         count=0;
                     }

                   }
                   else{
                     count2++;
                     if(count2>10){
                         count2+=count;
                         count=0;
                     }

                   }
 */



static int readiqval(){

    int BUFF_SIZE=4;
    char *buff[BUFF_SIZE];
    int fd1,fd2;
    int fft_size=64;

    double timeval= (double)fft_size/samp_rate*1e3;


    float* ptr;

    fftw_complex signal[fft_size];
    fftw_complex result[fft_size];
    fftw_plan plan;


    //when you using fft i,q values.

   if ( 0 > ( fd1 = open( "./realval.hex", O_RDONLY)))
   {
       return -1;

   }

   if ( 0 > ( fd2 = open( "./imagval.hex", O_RDONLY)))
   {
       return -1;

   }

   int i=0;
int n=0;


int count=0;
int count2=0;
int count1=0;

   while(read( fd1, &buff, BUFF_SIZE)>0){
       ptr=(float*)buff;
        signal[i][REAL]=*ptr;

       if(read( fd2, &buff, BUFF_SIZE)){
       ptr=(float*)buff;
       signal[i][IMAG]=*ptr;
       }
    i++;


   if(i>=fft_size)
   {
       n++;
       //printf("[%d]\n",n);
       i=0;

       plan = fftw_plan_dft_1d(fft_size,
                                        signal,
                                        result,
                                        FFTW_FORWARD,
                                        FFTW_MEASURE);
       //acquire_from_somewhere(signal);
       fftw_execute(plan);

        float tmp=sqrt(pow(result[1][REAL],2)+pow(result[1][IMAG],2) );
      // printf("%f\t%f\t%f\t%f \n",n*timeval,result[1][REAL],result[1][IMAG], tmp);




        //주기를 알기위한 갯수카운팅..
       if(tmp>0.2){//0.08){
           if(count==0 && count2>50000){
               printf("%d\n",count2+count+count1);
               count=0;
               count2=0;
               count1=0;
           }

         count++;
         if(count>10){
             count1+=count;
             count=0;
         }

       }
       else{
         count2++;
         if(count2>10){
             count2+=count;
             count=0;
         }

       }

       //printf("%f\t%f\n",result[fft_size/2][REAL],result[fft_size/2][REAL]);

       /*
       for(int j=0;j<fft_size;j++)
              printf("%f\t%f\t%f\t%f (%d)\n",n*timeval,result[j][REAL],result[j][IMAG], 20*log10(sqrt(pow(result[j][REAL],2)+pow(result[j][IMAG],2)) ) , j);
       if(n==2)
       break;
  */     //do_something_with(result);

   }

   }
   close( fd1);
  fftw_destroy_plan(plan);
   close( fd2);
}

static int fftiqval(){

    int BUFF_SIZE=4;
    char   *buff[BUFF_SIZE];
    float *ptr;
    float real,imag;
    int fd1,fd2;
    int fft_size=64;

    int i=0,n=0;
        float fftval[fft_size];


        double timeval= (double)fft_size/samp_rate*1e3;

        //when you using fft i,q values.

       if ( 0 > ( fd1 = open( "./fftrealval.hex", O_RDONLY)))
       {
           return -1;

       }

       if ( 0 > ( fd2 = open( "./fftimagval.hex", O_RDONLY)))
       {
           return -1;

       }

       int count=0;
       int count1=0;
       int count2=0;

       while(read( fd1, &buff, BUFF_SIZE)>0){
       ptr=(float*)buff;
        real=*ptr;

        imag=0;
       if(read( fd2, &buff, BUFF_SIZE)){
       ptr=(float*)buff;
       imag=*ptr;
       }

       fftval[i++]=(sqrt(pow(real,2)+pow(imag,2)));

       if(i>=fft_size)
       {
           n++;
           //printf("[%d]\n",n);
           i=0;
           //printf("%lf\t%f\t%f\t%f\n",n*timeval,real,imag,20*log10(fftval[fft_size/2]));

        int tmp=fftval[fft_size/2];



            //주기를 알기위한 갯수카운팅..
           if(tmp>0.2){//0.08){
               if(count==0 && count2>50000){
                   printf("%d\n",count2+count+count1);
                   count=0;
                   count2=0;
                   count1=0;
               }

             count++;
             if(count>10){
                 count1+=count;
                 count=0;
             }

           }
           else{
             count2++;
             if(count2>10){
                 count2+=count;
                 count=0;
             }

           }
           /*
           for(int j=0; j<fft_size; j++){
              printf("%lf\t%f\t%f\t%f(%d)\n",n*timeval,real,imag,20*log10(fftval[j]),j);
           }
           printf("\n\n");
           break;
            */
       }

      // if(real>=10 || real<=-10 || imag>=10 || imag<=-10)
      //     printf("%lf\t%f\n",n*timeval,sqrt(pow(real,2)+pow(imag,2)));
       //printf("%f\t%f\n",real,imag);

       }
       close( fd1);

       close( fd2);
}

static int logfftval(){

    int BUFF_SIZE =4;
    char   *buff[BUFF_SIZE];
    float *ptr;

    long i=0;
    double n=0;
    int count=0;
    int count1=0;
    int count2=0;

    int fft_size=64;

    float fftval[fft_size];

    double timeval= (double)fft_size/samp_rate*1e3;

    int fd1;

       //log fft value
       if((fd1=open("./logfft.hex",O_RDONLY))<0){
           return -1;
       }



       while(read( fd1, &buff, BUFF_SIZE)>0){
       ptr=(float*)buff;

        fftval[i++]=*ptr;

        if(i>=fft_size)
        {
            n++;
            //printf("[%d]\n",n);
            printf("%lf\t%f\n",n*timeval,fftval[12]);//fft_size-1]);
            i=0;

            /*

                    //주기를 알기위한 갯수카운팅..
                   if(tmp>0.2){//0.08){
                       if(count==0 && count2>50000){
                           printf("%d\n",count2+count+count1);
                           count=0;
                           count2=0;
                           count1=0;
                       }

                     count++;
                     if(count>10){
                         count1+=count;
                         count=0;
                     }

                   }
                   else{
                     count2++;
                     if(count2>10){
                         count2+=count;
                         count=0;
                     }

                   }
            */

            //for(int j=0; j<fft_size; j++){
            //    printf("%d\t%f\n",j,fftval[j]);
            //}
            //printf("\n\n");
            //if(n==10)
            //break;

        }
       //printf("%ld\t%f\n",i,real);


       }
       close( fd1);


}

static int complexfftval(){

    int fft_size=64;

    //complex data type
    std::complex<float> comfft[fft_size];
    std::complex<float>* tmp;
    int BUFF_SIZE=8;
    char * buff2[BUFF_SIZE];

    double timeval= (double)fft_size/samp_rate*1e3;

    int fd1;
    //com fft value
    if((fd1=open("./singiq_complex.bin",O_RDONLY))<0){
        printf("err\n");
        return -1;
    }

    int fd2;
    if((fd2=open("./iqresult.bin",O_RDWR|O_CREAT|O_TRUNC))<0){
        printf("err\n");
        return -1;
    }

    FILE* wf = fopen("iqresult.txt","w");
    if(!wf){
        perror("err file open");
    }


    long count=0;
    while(read( fd1, &buff2, BUFF_SIZE)>0){
    tmp=(std::complex<float>*)buff2;

    count++;

    fprintf(wf,"%ld %lf\n",count,std::real(*tmp));
    write(fd2,&buff2, BUFF_SIZE);
    if(count==1000000)
        break;



    }
    close( fd1);
    close( fd2);
    fclose(wf);
}

int main()
{


    complexfftval();
    //readiqval();

   return 0;
}
