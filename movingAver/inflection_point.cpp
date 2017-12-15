
#include <unistd.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <time.h>

#include <complex.h>
#include <fcntl.h>         // O_WRONLY
#include <fftw3.h>

#include <chrono>


#include "gnuPlotPipe.h"

bool withORG=false;

int main(int argc, char* argv[]){


    char fn[]="result1.txt";//"iqresult.txt";//

    char* filename=fn;

    if(argc==2){
        filename=argv[1];
    }
    if(argc==3){

    }

    FILE* wf= fopen(filename,"r");    
    if(!wf)
    {
        perror("fopen");
        return -1;
    }

    GNUPlotPipe gp;
    //gp.write(" set xrange [0.326:0.33]\n");
    gp.write("plot ");
    if(withORG)
        gp.writef(" \"%s\" u 1:2 w line,",filename);
    gp.write(" '-' with line title 'up-line' \n");//,\"%s\" u 1:2 w line \n",filename);//, '-' with line title 'downline'\n");






    //if there is a set of points like below...
    //
    // 1 1 3 1 1 1 3 2 2 2
    //
    //     +       +
    //               + + +
    // + +   + + +
    //

    //third one might be noise..
    //this is how to configure..
    //

    //rule1. if meet flat values, use as pivot points. (first pivot point's value is 1 at point 1,2)

    // get variations until some high value (more than twice) and skip the high value. (probably until 3th)
    //the variation of 3th is '2=(3-1)', and store previous average variation; ( point 1,2 average variation is 0, stored variation is 0)
    //then reset all 0, and do again.. 4'th, 5th, 6th...
    //4, 5, 6th variation also flat. then make pivot (current pivot point's 1 at point 4,5,6)

    //doing again....meet point7, (stored varation is 0, previous pivot is 1), meet point 8,9,10 (make current pivot as 2)
    //when current pivot(2) is higer than both previous pivot( '1' ) and stored variation( '0' ), there was some jump. at longist slope (point 7)

    //draw all pivots and jump slope.


    double curP=0;
    double preP=0;

    double preT,preV,curT,curV;
    preT=0;preV=0;curT=0;curV=0;

    double curA,preA, preA2,curA2;
    curA=0;preA=0;curA2=0;preA2=0;

    double dt=0;
    long count1=0;
    long count=0;
    long count2=0;
    double moveM=0;
    double preM=0;

    double sum_dif_sq=0;
    double pre_dif_sq=0;
    double max_dif_sq=0;

    double slopeX1=0;
    double slopeY1=0;
    double slopeT1=0;
    double slopeX2=0;
    double slopeY2=0;

    double slopeleng=0;
    double maxslopeleng=0;

    double longslopeX1=0;
    double longslopeX2=0;

    char line[200], *ps;

    while(fgets(line, sizeof(line),wf))
    {
        //ps=strchr(line,'\n');
        //if(ps!=NULL) *ps='\0';

        int cnt=0;
        ps=strtok(line," ");
        while(ps!=NULL){

            if(cnt==0)
                curT=atof(ps);
            if(cnt==1)
                curV=atof(ps);
            ps=strtok(NULL," ");
            cnt++;
        }


        if(dt==0)
            dt=curT-preT;

        curA=(curV-preV)/dt;

        count2++;

        moveM=(preM*count+curV)/(++count);


        double MA=(moveM-preM)/dt;

        double dif_sq=pow(curV-moveM,2);



        if(curA*preA<=0){
            slopeX1=preT;
            slopeY1=preV;
            slopeT1=curP;
        }
        slopeX2=curT;
        slopeY2=curV;


        slopeleng= slopeY2-slopeY1;
        if(slopeleng<0)
            slopeleng*=-1;

        if(maxslopeleng<slopeleng){
            maxslopeleng=slopeleng;




                longslopeX1=slopeX1;
                longslopeX2=slopeX2;


        }else{
            max_dif_sq= (max_dif_sq<dif_sq)? dif_sq: max_dif_sq;
        }




        if(dif_sq>2*pre_dif_sq)
        {

            pre_dif_sq= sum_dif_sq/count;
            count=1;
            sum_dif_sq=dif_sq;



        }
        else{

        sum_dif_sq+=dif_sq;

            if(round(MA*10)==0 ){

                if(pow(curP-preP,2)>max_dif_sq){

                    gp.writef("%lf %lf \n",longslopeX1,preP);
                    gp.writef("%lf %lf \n",longslopeX2,curP);

                    maxslopeleng=0;
                }
                preP=curP;
                curP=curV;

                pre_dif_sq= sum_dif_sq/count;
                count=0;
                sum_dif_sq=0;


                count1++;

                max_dif_sq=0;

                slopeX1=curT;
                slopeY1=curV;

               // gp.writef("%lf %lf \n",curT,curV);//(moveM+preM)/2);
            }

        }






        preV=curV;
        preM=moveM;
        preT=curT;

        preA=curA;


    }



    /*
     *
     *
  //v3
    double curP=0;
    double temp=0;

    double preT,preV,curT,curV;
    preT=0;preV=0;curT=0;curV=0;

    double curA,preA, preA2,curA2;
    curA=0;preA=0;curA2=0;preA2=0;

    double dt=0;
    long count1=0;
    long count=0;
    long count2=0;
    double moveM=0;
    double preM=0;

    double sum_dif_sq=0;
    double pre_dif_sq=0;

    double slopeX1=0;
    double slopeY1=0;
    double slopeX2=0;
    double slopeY2=0;

    double slopeleng=0;
    double maxslopeleng=0;

    double longslopeX1=0;
    double longslopeY1=0;
    double longslopeX2=0;
    double longslopeY2=0;

    char line[200], *ps;

    while(fgets(line, sizeof(line),wf))
    {
        //ps=strchr(line,'\n');
        //if(ps!=NULL) *ps='\0';

        int cnt=0;
        ps=strtok(line," ");
        while(ps!=NULL){

            if(cnt==0)
                curT=atof(ps);
            if(cnt==1)
                curV=atof(ps);
            ps=strtok(NULL," ");
            cnt++;
        }


        if(dt==0)
            dt=curT-preT;

        curA=(curV-preV)/dt;

        if(curA*preA<=0){
            slopeX1=preT;
            slopeY1=preV;
        }
        slopeX2=curT;
        slopeY2=curV;


        slopeleng= slopeY2-slopeY1;
        if(slopeleng<0)
            slopeleng*=-1;

        if(maxslopeleng<slopeleng){
            maxslopeleng=slopeleng;


                longslopeX1=slopeX1;
                longslopeX2=slopeX2;

                longslopeY1=curP;//slopeY1;
                longslopeY2=curV;//slopeY2;


        }

        count2++;

        moveM=(preM*count+curV)/(++count);


        double MA=(moveM-preM)/dt;

        double dif_sq=pow(curV-moveM,2);


        if(dif_sq>2*pre_dif_sq)
        {


            //gp.writef("%lf %lf \n",longslopeX1,longslopeY1);
            //gp.writef("%lf %lf \n",longslopeX2,curV);
            //most significant add...
            //count1++;
            pre_dif_sq= sum_dif_sq/count;
            count=1;
            sum_dif_sq=dif_sq;
            curP=curV;

        }
        else{

        sum_dif_sq+=dif_sq;

        if(round(MA*10)==0 ){
            pre_dif_sq= sum_dif_sq/count;
            count=0;
            sum_dif_sq=0;
            curP=curV;


            count=0;
            curP=curV;

            count1++;
            gp.writef("%lf %lf \n",curT,(moveM+preM)/2);
        }

        }






        preV=curV;
        preM=moveM;
        preT=curT;

        preA=curA;


    }


    /*

//first set;
    long MINPICKCOUNT=0;

    double preT,preV,curT,curV;
    preT=0;preV=0;curT=0;curV=0;

    double curA,preA, preA2,curA2;
    curA=0;preA=0;curA2=0;preA2=0;

    double dt=0;
    long count1=0;
    long count=0;
    double moveM=0;
    double preM=0;

    double maxA=0;
    double maxT=0;
    double maxV=0;
    double tmpA=0;

    double maxDif=0;
    double maxDifV=0;
    double maxDifT=0;
    double maxDifPV=0;
    double maxDifPT=0;

    while(fscanf(wf,"%lf %lf\n",&curT,&curV)){
        if(feof(wf)) break;

        if(dt==0)
            dt=curT-preT;


        moveM=(count*preM+curV)/(++count);
        //count1++;
        //gp.writef("%.8lf %lf\n",curT,moveM);

        //mean의 부호가 바뀌기 전까지. 원래 그래프에서 기울기가 가장 큰 값이 변곡점.
        curA= (moveM-preM)/dt;


        tmpA=curA;
        if(curA<0)
            tmpA=curA*-1;

        if(tmpA>maxA)
        {
            maxA=tmpA;
            maxT=curT;
            maxV=moveM;//curV;
        }

        if( (round(tmpA*1000)==0)   &&  ( (preA<0&&curA>0) || (preA>0&&curA<0) ) ){
            count1++;
            maxA=0;
            gp.writef("%.8lf %lf\n",maxT,maxV);

        }


        preM=moveM;
        preT=curT;
        preV=curV;
        preA=curA;


    }



*/

/*
 //second set

    GNUPlotPipe gp;
    gp.writef("set xrange[1.857:1.862] \n plot \"%s\" u 1:2 w line, '-' with line title 'up-line'  \n",filename);//,\"%s\" u 1:2 w line \n",filename);//, '-' with line title 'downline'\n");

    double curM=0;

    double preT,preV,curT,curV;
    preT=0;preV=0;curT=0;curV=0;

    double curA,preA, preA2,curA2;
    curA=0;preA=0;curA2=0;preA2=0;

    double dt=0;
    long count1=0;
    long count=0;
    double moveM=0;
    double preM=0;

    double sum_dif_sq=0;
    double pre_dif_sq=0;
    double pre_max_dif_sq=0;

    double max_dif_sq=0;

    double a,b,c,d;

    bool keepgo=false;

    while(fscanf(wf,"%lf %lf\n",&curT,&curV)){
        if(feof(wf)) break;

        if(dt==0)
            dt=curT-preT;

        moveM=(preM*count+curV)/(++count);


        double dif_sq=pow(curV-curM,2);
        max_dif_sq= (dif_sq>max_dif_sq)? dif_sq:max_dif_sq;

        //gp.writef("%lf %lf \n",curT,moveM);

        double MA=(moveM-preM)/dt;

        //MA=( (MA<0)? MA*-1 : MA);

        if(round(MA*10)==0 ){
            curM=curV;

            count=1;
            pre_max_dif_sq=max_dif_sq;
            max_dif_sq=0;
            sum_dif_sq=dif_sq;

            //count1++;
             gp.writef("%lf %lf \n",curT,curV);
            a=curT;
            b=curV;
            keepgo=true;
        }



        preM=moveM;
        preT=curT;
        preV=curV;
        preA=curA;


    }
 */

    gp.write("e\n");
    gp.flush();




    fclose(wf);

    printf("%ld / %ld\n",count1,count2);
    while(1){}

    return 0;

}
