#include <lime/LimeSuite.h>

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

#include <condition_variable>
#include <mutex>
#include <future>
#include <thread>
#include <queue>
#include <chrono>
#include <pthread.h>
#include <thread>

//#include "gnuPlotPipe.h"

bool timecheck = true;

const int queN=5;
static std::queue<float> Queue[2][queN];
static pthread_mutex_t pMutex[2][queN];
static bool sendEnd=false;

using namespace std;

/*
 *
 * to compile this cpp file
 g++ -o fft_processor.out fft_processor.cpp -lLimeSuite -lfftw3 -lpthread -std=c++11 -lm


*/

static const int rxNum=2;
//Device structure, should be initialize to NULL
static lms_device_t* device = NULL;
static lms_stream_t rx_stream[rxNum];
static lms_stream_t tx_stream;

static float sample_rate=10e6;//32E6;
static int sampleCnt=round(sample_rate/10);

static float record_sec=6.0;

static float center_freq= 2400e6;//2452:ch9
static int center_int=round(center_freq/1e6);

//read thread

static void reader (int num, float maxtime)
{
    fprintf(stderr,"read thread num:%d %lf\n",num,maxtime);
    char resultFile[100]="result1.txt";
    char resultFile_m[100]="result2.txt";
    if(num==1){
        strcpy(resultFile,"result3.txt");
        strcpy(resultFile_m,"result4.txt");
    }


        FILE* wf= fopen(resultFile,"w");
        if(!wf)
        {
            perror("fopen");
            return;
        }

        FILE* wf2= fopen(resultFile_m,"w");
        if(!wf2)
        {
            perror("fopen");
            return;
        }

    float preA=0;
    float curA=0;
    float preV=0;
        bool doneflag=false;
        long count=0;
        long count2=0;
double preT=0;
double curT=0;

float real=0;
float img=0;
std::chrono::system_clock::time_point startT;

    if(timecheck)
         startT = std::chrono::system_clock::now();

    while (true) {
        double val;
        for(int j=0; j<queN;j++)
            if(!pthread_mutex_trylock(&pMutex[num][j])){
            while(!Queue[num][j].empty() ){
                    //std::unique_lock<std::mutex> ul(queueMutex[j]);
//////////////////////////////
                    curT=Queue[num][j].front();
                    Queue[num][j].pop();

                    val=Queue[num][j].front();
                    Queue[num][j].pop();
                    //printf("%.8lf\n ",time);

                    real=Queue[num][j].front();
                    Queue[num][j].pop();

                    img=Queue[num][j].front();
                    Queue[num][j].pop();

                    count++;

                    if(curT>=maxtime){
                        doneflag=true;
                    }

                    curA= (val-preV)/(curT-preT);

                    if(curA<0) curA*=-1 ;


                    if(preA*10 <curA || curA*10<preA)
                    {    fprintf(wf2,"%.8lf  %lf %lf %lf\n",curT,val,real,img);
                        count2++;

                    }
                    preA=curA;
                    preV=val;
                    preT=curT;
                    fprintf(wf,"%.8lf  %lf %lf %lf\n",curT,val,real,img);
                    /*

                    //curA=(val-preV)/timeval;
                    //if(!(curA>=0 && preA>=0 && ( preA*0.5<=curA||curA*0.5>=preA )) && !(curA<0 && preA<0 && ( preA*0.5>=curA||curA*0.5<=preA ))){
                    curA=(val-preV)/(curT-preT);

                    if(curA<0)
                        curA*=-1;


                    //if(btw>1e13)//preA*0.000000005>curA || curA*0.000000005>preA)
                    {
                        count2++;

                        fprintf(wf,"%.8lf %lf \n",curT,curA);
                        preA=curA;
                    }

                    preV=val;
                    preT=curT;


  */                  //curA=(val-preV)/(time-pret);
                    //if(curA<0)
                    //    curA*=-1;
                    //printf("%lf\n",curA);
                    //if(!( ( preA*0.05<=curA||preA*1.0005>=curA )) && !(( preA*0.5>=curA||preA*1.5<=curA ))){
                     //   count2++;
                     //   fprintf(wf,"%.8lf %lf \n",time,val);
                    //}
                    //preV=val;
                    //preA=curA;


                    //printf("\n");
                    //fprintf(wf,"%8f %f\n ",time,val);

////////////////////////////////
            }
            pthread_mutex_unlock(&pMutex[num][j]);
            }

            /*
        int ret= read(fileDescriptors[0], (char*)buffer, 1024*sizeof(float));

        for(int i=0; i<ret/sizeof(float);i++)
        {

            val=buffer[i];
            //float real=buffer[i*2];
            //float img= buffer[i*2+1];
            //val=sqrt(pow(real,2)+pow(img,2));
            curA=(val-preV)/timev;

            count++;
            if(count*timev>=maxtime)
                doneflag=true;

            if(!(curA>=0 && preA>=0 && ( preA*0.5<=curA||preA*1.5>=curA )) && !(curA<0 && preA<0 && ( preA*0.5>=curA||preA*1.5<=curA ))){
                count2++;
                fprintf(wf,"%.8lf %lf \n",(count-1)*timev,val);
            }
            preV=val;
            preA=curA;

        }*/

        if(doneflag)
            break;

   }

    printf("count:%ld\ncount2:%ld\n",count,count2);

    fclose(wf);
    fclose(wf2);
    if(timecheck)
    {
        std::chrono::duration<double> sec = std::chrono::system_clock::now() - startT;
        fprintf(stderr,"spending time for making '%s' : %lf sec\n", resultFile,sec.count());
    }

}

void sigGen(float freq, int sendSigN, int waitSigN){//float* tx_buffer, int len, float freq

    freq=1;
    const int len=round(2*sample_rate/freq);

    float* sine_buffer= new float[len*2];
    float* wait_buffer= new float[len*2];

    for (int i = 0; i <len; i++)
    {
        sine_buffer[2*i] = cos(2*M_PI*i/(float)len);
        sine_buffer[2*i+1] = 0;//sin(2*M_PI*i/16.0);

        wait_buffer[2*i]=0;
        wait_buffer[2*i+1]=0;
    }


    LMS_StartStream(&tx_stream);

    while(!sendEnd){

        for(int i=0; i<sendSigN;i++){
            int ret = LMS_SendStream(&tx_stream,sine_buffer,len,NULL,1000);
            if (ret != len)
                printf( "error: samples sent: %d / %d\n",ret ,len);

        }
        for(int i=0; i<waitSigN; i++){
           int ret = LMS_SendStream(&tx_stream,wait_buffer,len,NULL,1000);
           if (ret != len)
               printf( "error: samples sent: %d / %d\n",ret ,len);
        }

    }

    delete sine_buffer;
    delete wait_buffer;

    LMS_StopStream(&tx_stream);
}


int error()
{
    //print last error message
    printf("ERROR: %s\n", LMS_GetLastErrorMessage());
    if (device != NULL)
        LMS_Close(device);
    exit(-1);
}



static int fftw3_from_lms(){

    fprintf(stderr,"writing fft result in 'result.txt'...\n");

    sampleCnt= round(sample_rate/10);
    float buffer[2][sampleCnt*2];
    int samplesRead[2];

    for(int i=0;i<rxNum; i++)
    memset(buffer[i], 0, sampleCnt*2*sizeof(float));

    int j=0;
    int k=0;

    std::thread th1(reader, 0,record_sec);
    std::thread th2(reader, 1,record_sec);
    std::thread th3(sigGen,1,1,1);//freq= 1Hz, send 1 sine, sleep 1 sine.


 //GNUPlotPipe gp;
 //gp.write("plot '-' with points title 'iq-sqrt'\n");
    for(int i=0;i<rxNum; i++)
        LMS_StartStream(&rx_stream[i]);

   // LMS_StartStream(&tx_stream);

long long count=0;
long long count2=0;

//int countval[nyquist+1]={0};
//int tenmax[11]={0};
double preA=0;
double curA=0;
double preV=0;
double curT=0;
double preT=0;
double val=0;

float preR=0;
float preI=0;


auto t0 = chrono::high_resolution_clock::now();
auto t1 = t0;
auto t2 = t0;
double t3=0;
double timeval=0;
std::chrono::duration<float> sec =t2-t0;

printf("timeval:%f\n",timeval);



while (timeval < record_sec) //run for 10 seconds
{

    //Receive samples
    for(int i=0;i<rxNum; i++)
        samplesRead[i] = LMS_RecvStream(&rx_stream[i], buffer[i], sampleCnt, NULL, 1000);

    t2 = chrono::high_resolution_clock::now();
    sec=t2-t1;

    for(int i=0;i<rxNum; i++)
        for(;k<queN;k=(k+1)%queN)
        if(!pthread_mutex_trylock(&pMutex[i][k])){
            //printf("q:%d\n",k);

            t3=sec.count()/samplesRead[i];
            //printf("timeval:%lf  / t:%.8lf\n",timeval,t3);

            for ( j = 0; j < samplesRead[i]; j++)
            {
                    //(float)buffer[2*j+1];    //Imagin part :Q
                    //(float)buffer[2*j];      //Real part :I

                    float img=buffer[i][2*j+1];
                    float real=buffer[i][2*j];

                    val=sqrt( pow(real,2) + pow(img,2) );

                        count++;
                        /*
                        //curA=(val-preV)/timeval;
                        //if(!(curA>=0 && preA>=0 && ( preA*0.5<=curA||curA*0.5>=preA )) && !(curA<0 && preA<0 && ( preA*0.5>=curA||curA*0.5<=preA ))){
                        curA=(val-preV)/(curT-preT);

                        if(curA<0)
                            curA*-1;

                        if(preA<curA*10000 || curA<preA*10000)
                        {

                        }
                        //if(( preA*0.50>=curA||curA*0.5>=preA )){
                         else{   //printf("%lf\n",curA);
                            count2++;
                            Queue[k].push(curT);
                            Queue[k].push (val);
                            //fprintf(wf,"%.8lf %lf \n",time,val);
                            preV=val;
                            preA=curA;
                            preT=curT;


                        }
                        */

                        Queue[i][k].push(timeval+t3*j);
                        Queue[i][k].push (val);
                        Queue[i][k].push (real);
                        Queue[i][k].push (img);

            }
            //double timev=count*timeval;
            //printf("%.8lf\n",curT);
            pthread_mutex_unlock(&pMutex[i][k]);
            break;
        }

        t1=t2;
        sec=t2-t0;
        timeval=sec.count();

    }

    printf("c:%ld\nc2:%ld\n",count,count2);

    //gp.write("e\n");
    //gp.flush();

    sendEnd=true;
    th3.join();

    LMS_StopStream(&tx_stream);
    for(int i=0;i<rxNum; i++)
        LMS_StopStream(&rx_stream[i]);

    th1.join();
    th2.join();
    return 0;
}


static int preprocessing(){
    //Find devices
    int n;
    lms_info_str_t list[8]; //should be large enough to hold all detected devices
    if ((n = LMS_GetDeviceList(list)) < 0) //NULL can be passed to only get number of devices
        return error();

    printf("Devices found: %d\n", n ); //print number of devices
    if (n < 1)
        return -1;

    //open the first device
    if (LMS_Open(&device, list[0], NULL))
        return error();

    //Initialize device with default configuration
    //Do not use if you want to keep existing configuration
    //Use LMS_LoadConfig(device, "/path/to/file.ini") to load config from INI

    if( LMS_Init(device) !=0 )
        return error();


    //Enable RX channel
    //Channels are numbered starting at 0
    if (LMS_EnableChannel(device, LMS_CH_RX, 0, true) != 0)	//### Enable->Disable (true->false)
        error();
    if (LMS_EnableChannel(device, LMS_CH_RX, 1, true) != 0)	//### Enable->Disable (true->false)
        error();
    if (LMS_EnableChannel(device, LMS_CH_TX, 0, true) != 0)	//### Enable->Disable (true->false)
        error();


    if (LMS_SetAntenna(device, LMS_CH_RX, 0, LMS_PATH_LNAL) != 0) // manually select antenna
        error();

    if (LMS_SetAntenna(device, LMS_CH_RX, 1, LMS_PATH_LNAL) != 0) // manually select antenna
        error();

    if (LMS_SetAntenna(device, LMS_CH_TX, 0, LMS_PATH_TX2) != 0) // manually select antenna
        error();


    //Set center frequency to 800 MHz
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 0, center_freq)!=0) //800e6) != 0)
       error();
    if (LMS_SetLOFrequency(device, LMS_CH_RX, 1, center_freq)!=0) //800e6) != 0)
       error();
    if (LMS_SetLOFrequency(device, LMS_CH_TX, 0, center_freq)!=0) //800e6) != 0)
       error();

    //Set sample rate to 8 MHz, ask to use 2x oversampling in RF
    if(LMS_SetSampleRate(device, sample_rate, 8)<0)
        error();
/*
    LMS_SetGaindB(device, LMS_CH_RX, 0, 40);

    LMS_SetGaindB(device, LMS_CH_RX, 1, 40);
    //if (LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 0.5) != 0)
    //    error();
    //if (LMS_SetNormalizedGain(device, LMS_CH_RX, 1, 0.5) != 0)
    //    error();
    //if (LMS_SetNormalizedGain(device, LMS_CH_TX, 0, 0.5) != 0)
    //    error();

/*
    float_type host_Hz[6]={0};


    if( LMS_GetClockFreq(device, 0, &host_Hz[0]) !=0)
        error();
if( LMS_GetClockFreq(device, 1, &host_Hz[1]) !=0)
        error();
if( LMS_GetClockFreq(device, 2, &host_Hz[2]) !=0)
        error();
if( LMS_GetClockFreq(device, 3, &host_Hz[3]) !=0)
        error();
if( LMS_GetClockFreq(device, 4, &host_Hz[4]) !=0)
        error();
if( LMS_GetClockFreq(device, 5, &host_Hz[5]) !=0)
        error();


        printf("clk hz:%f, %f, %f, %f, %f, %f, %f\n",host_Hz[0],host_Hz[1],host_Hz[2],host_Hz[3],host_Hz[4],host_Hz[5],host_Hz[6]);
*/


    //Enable test signal generation
    //To receive data from RF, remove this line or change signal to LMS_TESTSIG_NONE
    //if (LMS_SetTestSignal(device, LMS_CH_RX, 0, LMS_TESTSIG_NCODIV8, 0, 0) != 0)
    //    error();
    float Flpw= 5e6;
    int Gain=10;

    //Configure LPF, bandwidth 8 MHz
    if (LMS_SetLPFBW(device, LMS_CH_RX, 0, Flpw) != 0)
        error();
    if (LMS_SetLPFBW(device, LMS_CH_RX, 1, Flpw) != 0)
        error();
    if (LMS_SetLPFBW(device, LMS_CH_TX, 0, Flpw) != 0)
        error();

    //Set RX gain
    if (LMS_SetGaindB(device, LMS_CH_RX, 0, Gain) != 0)
        error();
    if (LMS_SetGaindB(device, LMS_CH_RX, 1, Gain) != 0)
        error();

    /*if (LMS_SetNormalizedGain(device, LMS_CH_RX, 0, 0) != 0)
        error();
    if (LMS_SetNormalizedGain(device, LMS_CH_RX, 1, 0) != 0)
        error();*/
    //Print RX gain

    //Perform automatic calibration
    if (LMS_Calibrate(device, LMS_CH_RX, 0, Flpw, 0) != 0)
        error();
    if (LMS_Calibrate(device, LMS_CH_RX, 1, Flpw, 0) != 0)
        error();/**/
    if (LMS_Calibrate(device, LMS_CH_TX, 0, Flpw, 0) != 0)
        error();/**/

    // Switch off DC corrector
    LMS_WriteParam(device,LMS7_MAC,1);     //channel 0
    LMS_WriteParam(device,LMS7_DC_BYP_RXTSP,1);
    LMS_WriteParam(device,LMS7_MAC,2);     //channel 1
    LMS_WriteParam(device,LMS7_DC_BYP_RXTSP,1);

    //Streaming Setup

    //Initialize stream
    //lms_stream_t streamId; //stream structure
    rx_stream[0].channel = 0; //channel number
    rx_stream[0].fifoSize = 1024 * 1024*300; //fifo size in samples
    rx_stream[0].throughputVsLatency = 1.0; //optimize for max throughput
    rx_stream[0].isTx = false; //RX channel

    rx_stream[0].dataFmt = lms_stream_t::LMS_FMT_F32;//LMS_FMT_I12; //12-bit integers
    if (LMS_SetupStream(device, &rx_stream[0]) != 0)
        return error();

    rx_stream[1].channel = 1; //channel number
    rx_stream[1].fifoSize = 1024 * 1024* 300; //fifo size in samples
    rx_stream[1].throughputVsLatency = 1.0; //optimize for max throughput
    rx_stream[1].isTx = false; //RX channel

    rx_stream[1].dataFmt = lms_stream_t::LMS_FMT_F32;//LMS_FMT_I12; //12-bit integers
    if (LMS_SetupStream(device, &rx_stream[1]) != 0)
        return error();

    tx_stream.channel = 0; //channel number
    tx_stream.fifoSize = 1024 * 1024*300; //fifo size in samples
    tx_stream.throughputVsLatency = 1.0; //optimize for max throughput
    tx_stream.isTx = true; //RX channel

    tx_stream.dataFmt = lms_stream_t::LMS_FMT_F32;//LMS_FMT_I12; //12-bit integers
    if (LMS_SetupStream(device, &tx_stream) != 0)
        return error();

    //int buffer[sampleCnt * 2];
    //Start streaming
    fprintf(stderr,"setting done\n\n");

    return 0;
}

static void postprocessing(){

    LMS_DestroyStream(device, &tx_stream); //stream is deallocated and can no longer be used
    LMS_DestroyStream(device, &rx_stream[0]); //stream is deallocated and can no longer be used
    LMS_DestroyStream(device, &rx_stream[1]); //stream is deallocated and can no longer be used

    //Close device
    LMS_Close(device);

}



int main(int argc,char ** argv) {

    for(int i=0;i<queN;i++){
        pthread_mutex_init(&pMutex[0][i], NULL);
        pthread_mutex_init(&pMutex[1][i], NULL);
    }

    int num;
            extern char *optarg;
            extern int optind;
            while( (num = getopt(argc, argv, "c:f:s:t:h")) != -1) {

                switch(num) {
                        case 's':
                                sample_rate=atof(optarg)*1e6;
                                break;
                        case 'c':
                                center_freq=atof(optarg)*1e9;
                                center_int=round(atof(optarg)*1000);
                                printf("%d\n",center_int);
                                break;
                        case 'f':
                               // fft_size=atoi(optarg);
                                break;
                        case 't':
                                record_sec=atof(optarg);
                                break;
                        case 'h':
                                printf("Option : \n");
                                printf("\t-s [float] : set sample rate [float] Mhz\n");
                                printf("\t-c [float] : set center frequency of lms band [float] Ghz\n");
                                printf("\t-f [Integer] : set fft_size [Integer]\n");
                                printf("\t-t [float] : set measuring time [float] sec\n");
                                printf("\t-h : print Help page\n");
                                return 0;

                }
            }

        printf("samplerate:%f\n",sample_rate);
        printf("timeval:%.8f(s) / %.8f(ms) / %.8f(us-microsec)\n\n",1/sample_rate,1/sample_rate*1e3, 1/sample_rate*1e6);

        //open lms stream;
        if(preprocessing()<0)
           return -1;

        //do fft from lms right after getting i,q values from lms
        fftw3_from_lms();

        //close lms stream;
       postprocessing();

       // displayfile("result.txt","display.txt");

        return 0;
}


