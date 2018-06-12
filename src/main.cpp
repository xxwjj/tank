#include <stdio.h>
#include <string.h>
#include "debug.h"
#include "define.h"
#include "socket.h"
#include "agent.h"
#include "config.h"

using namespace std;

int main(int argc, char * argv[])
{
    if (argc != 4)
    {
        printf("Usage %s [player_id] [server_ip] [server_port]\n", argv[0]);
        return -1;
    }

    in team_id = atoi(argv[1]);
    unsigned long server_ip = inet_addr(argv[2]);
    unsigned short server_port = atoi[argv[3]];

    InitConfigFile();
    initLogModule();

    CONNECTOR->init(server_ip, server_port);
    CONNECTOR->connect();

    AGENT->Registation(team_id);

    unsigned  int round_time_out = g_config.Read<unsigned int>("round_timeout");

    while (AGENT->Once(round_time_out)){}
    log("Exit !\n");
    return 0;

}