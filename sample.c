#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "ltev2x.h"
#include "gnss-rx.h"
#include "time-get.h"

typedef enum
{
    false = 0, 
    true = 1
}bool;

LTEV2X_HANDLE initV2xHdl(int power, int ueid, int psid, int period) {
    LTEV2X_Option options;
    
    //open ltev2x
    LTEV2X_HANDLE g_ltev2x_hdl = LTEV2X_Open();
    if (-1 == g_ltev2x_hdl) {
        printf("open ltev2x error!\n");
        return -1;
    }      
    //config ltev2x
    options.ue_id = ueid;
    options.ps_id = psid;
    options.tx_power = power;
    options.priority = 7;
    options.period = period;
    options.proc_id = 0;
    options.data_len = 512;
    options.test_mode = 0;
    options.test_sfn = 0;
    if (0 != LTEV2X_SetOption(g_ltev2x_hdl, &options)) {
        printf("set ltev2x error!\n");
        return -1;
    }
    return g_ltev2x_hdl;
}
 
GNSS_HANDLE initGnssHdl()
{
    int ret;
    GNSS_HANDLE hdl = GNSS_Open();
    if (-1 == hdl)
    {
        printf("open gnss error!\n");
        return NULL;
    }

    //time
    ret = Time_Init();
    if (-1 == ret)
    {
        printf("time init error!\n");
        return NULL;
    }
    return hdl;
}

GnssUtcTime getGPSTime(GNSS_HANDLE hdl) {
    int t_val;
    GnssLocation fix;

    if (-1 == GNSS_Recv(hdl, &fix)) return 0;
    t_val = Time_GetElapsedTime_us();

    return t_val + fix.timestamp;
}

void *gnss_event_loop(LTEV2X_HANDLE g_ltev2x_hdl, int datalen)
{
    int ret;
    int i;
    GNSS_HANDLE hdl;
    unsigned char *data = (unsigned char*)malloc(datalen * sizeof(char));
    memset(data, -1, datalen * sizeof(data));

    //open gnss
    hdl = initGnssHdl();

    //loop
    for (i = 1;; i++)
    {
        *((GnssUtcTime *)data) = getGPSTime(hdl);
        *((unsigned int *)(data + 8)) = i;

        LTEV2X_Send(g_ltev2x_hdl, (void *)data, datalen * sizeof(data));
    }

    return NULL;
}

void* v2x_event_loop(LTEV2X_HANDLE g_ltev2x_hdl, int datalen)
{
    int ret;
    int i = 0;
    GNSS_HANDLE hdl;
    GnssUtcTime time_before;
    GnssUtcTime time_diff;
    int num = 0;
    bool same = false;
    int power = 0;
    unsigned char *data = (unsigned char*)malloc(datalen * sizeof(char));
    unsigned char *buf = (unsigned char*)malloc(datalen * sizeof(char));
    memset(buf, -1, datalen * sizeof(buf));
    memset(data, -1, datalen * sizeof(data));

    //open gnss
    hdl = initGnssHdl();

    //loop
    for ( i = 1; ; i++ ) {
        ret = LTEV2X_Recv(g_ltev2x_hdl, buf, datalen, &power);
        printf("[%s] %d received.\n", __FUNCTION__, ret);

        time_before = *((GnssUtcTime *)buf);
        time_diff = getGPSTime(hdl) - time_before;
        num = *((unsigned int *)(buf + 8));

        *((GnssUtcTime *)data) = time_before;
        *((unsigned int *)(data + 8)) = num;
        same = (memcpy((void *)buf, (void *)data, datalen) == 0)?true:false;

        printf("%d,%d,%d,%s", num, time_diff, power, same?"true":"false" );
    }

    return NULL;
}

void exitApp(int sign)
{
    printf("exit();");
    exit(0);
}

int main(int argc, char *argv[])
{
    int i = 1;
    bool send = false;
    bool recive = false;
    int power = 10;
    int ueid = 211;
    int psid = 0;
    int datalen = 0;
    int period = 1;
    LTEV2X_HANDLE g_ltev2x_hdl;

    // parameter check
    for (i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "-s")) send = true;
        else if (strcmp(argv[i], "-r")) recive = true;
        else if (strcmp(argv[i], "-power")) // power
        {
            if(++i < argc ) power = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-ueid")) // ue_id
        {
            if (++i < argc) ueid = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-len")) // length
        {
            if (++i < argc) datalen = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-psid")) // psid
        {
            if (++i < argc)
                psid = atoi(argv[i]);
        }
        else if (strcmp(argv[i], "-period")) // period
        {
            if (++i < argc)
                period = atoi(argv[i]);
        }
    }

    // v2x handle create
    g_ltev2x_hdl = initV2xHdl(power, ueid, psid, period);
    if(g_ltev2x_hdl == -1) return -1;

    // ctrl+c catch
    signal(SIGINT, exitApp);

    // send or recive v2x
    if(send == true)
    {
        gnss_event_loop(g_ltev2x_hdl, datalen);
    }
    else if (recive == true)
    {
        v2x_event_loop(g_ltev2x_hdl, datalen);
    }

}
