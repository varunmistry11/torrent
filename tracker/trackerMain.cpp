#include "global.h"
#include "customlogger.h"
#include "dataStructures.h"
#include "semaphore.h"
#include "fileFunctions.h"
#include "serveClientCommands.h"

// creating new thread for each client.
void* serverNewClient(void* param){

    // get client client socket descriptor
    int client_sd = *(int *)param;
    int isClientLogedIn = false;
    string loggedInUserId = "";

    // write code for each function

    while(1){

        // read the command info
        char buffer[MAXBUFFERSIZE] = {0};
        read(client_sd, buffer, sizeof(buffer));

        string commandInfo = buffer;
        
        vector<string> commandArgs = parseString(commandInfo,' ');

        string command = commandArgs[0];

        if(command == "create_user"){    
            LOGGER::DEBUG("create_user command detected");
            // [create_user, userid, password]
            serveCreateUser(client_sd,commandInfo);
        }

        else if(command == "login"){
            LOGGER::DEBUG("login command detected");
            // [login, userid, password]
            if(serveClientLogin(client_sd,commandInfo)){
                isClientLogedIn = true;
                loggedInUserId = commandArgs[1];
                LOGGER::INFO("loggedInUser %s",loggedInUserId);
            }
        }
        
        else if(command == "create_group"){
            LOGGER::DEBUG("create_group command detected");
            // [create_group, grpupid]
            commandInfo += " " + loggedInUserId;
            serveCreateGroup(client_sd,commandInfo);

        }

        else if(command == "join_group"){
            LOGGER::DEBUG("join_group command detected");
            // [join_group, groupid]
            commandInfo += " " + loggedInUserId;
            serveJoinGroup(client_sd, commandInfo);
        }

        else if(command == "list_requests"){
            LOGGER::DEBUG("list_requests command detected");
            //[list_requests, groupid]
            commandInfo += " " + loggedInUserId;
            LOGGER::INFO("logged in userid %s",loggedInUserId.c_str());
            serveListPendingRequest(client_sd, commandInfo);
        }

        else if(command == "list_groups"){
            LOGGER::DEBUG("list_groups command detected");
            // [list_groups]
            commandInfo += " " + loggedInUserId;
            serveListAllGroups(client_sd, commandInfo);
        }

        else if(command == "accept_request"){
            LOGGER::DEBUG("accept_request command detected");
            // [list_groups]
            commandInfo += " " + loggedInUserId;
            serveAcceptRequest(client_sd, commandInfo);
        }

        else if(command == "upload_file"){
            //[upload_file, fileName, groupid, fileSize, SHA1]
            LOGGER::DEBUG("upload_file command detected");
            commandInfo += " " + loggedInUserId;
            serveUploadFile(client_sd, commandInfo);

        }
        else if(command == "download_file"){
            LOGGER::DEBUG("download_file command detected");
            // [download_file, groupid, filename, destPath]
            commandInfo += (" " + loggedInUserId);
            serveDownloadFile(client_sd, commandInfo);
        }

        else if(command == "list_files"){
            LOGGER::DEBUG("list_files command detected");
            // [download_file, groupid, filename, destPath]
            commandInfo += (" " + loggedInUserId);
            serveListFiles(client_sd, commandInfo);
        }
        else if(command == "logout"){
            LOGGER::DEBUG("logout command detected");
            serveLogout(client_sd,isClientLogedIn, loggedInUserId);
            isClientLogedIn = false;
            loggedInUserId = "";
        }else if(command == "leave_group"){
            LOGGER::DEBUG("leave_group command detected");
            // [leave_group, groupid]
            commandInfo += (" " + loggedInUserId);
            serveLeaveGroup(client_sd,commandInfo);
        }else if(command == "stop_share"){
            LOGGER::DEBUG("leave_group command detected");
            // [stop_share, groupid, fileName]
            commandInfo += (" " + loggedInUserId);
            serveStopShare(client_sd,commandInfo);
        }else if(command == "add_seeder"){
            LOGGER::DEBUG("leave_group command detected");
            // [add_seeder, fileName, groupid]
            commandInfo += (" " + loggedInUserId);
            serveAddSeeder(client_sd,commandInfo);
        }else if(command == "add_leecher"){
            LOGGER::DEBUG("add_leecher command detected");
            // [add_leecher, fileName, groupid]
            commandInfo += (" " + loggedInUserId);
            serveAddLeecher(client_sd,commandInfo);
        }
        else if(command == "quit"){
            break;
        }
        else{
            LOGGER::INFO("Invalid command");
        }
    }

    pthread_exit(NULL);    

}

int main(int argc, char** argv){

    LOGGER::INFO("Tracker main started..");

    reloadClientInfoFromFile();
    reloadGroupInfoFromFile();
    reloadFileInfoFromFile();

    sem_init(&clientInfoMap_RLOCK, 0, 1);
    sem_init(&clientInfoMap_WLOCK, 0, 1);

    sem_init(&groupInfoMap_RLOCK, 0, 1);
    sem_init(&groupInfoMap_WLOCK, 0, 1);

    sem_init(&fileInfoMap_RLOCK, 0, 1);
    sem_init(&fileInfoMap_WLOCK, 0, 1);
    
    int trackerPort,tracker_sd;
    string trackerIPAddress;
    struct sockaddr_in trackerAdd;

    // get ip-address and port of tracker-1
    readTrackerInfoFromFile(argv[1],trackerPort,trackerIPAddress);

    // Creating socket file descriptor
    if ((tracker_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        printf("Tracker socket conneciton failed");
        return 0;
    }
    
    LOGGER::SUCCESS("Tracker socket created");

    // assign type of ip address ipv4 here
    trackerAdd.sin_family = AF_INET;         
    // assign port 
    trackerAdd.sin_port = htons(trackerPort);
    // store this IP address in trackerAdd.sin_addr
    // trackerAdd.sin_addr.s_addr = inet_addr(trackerIPAddress.c_str());
    inet_pton(AF_INET, trackerIPAddress.c_str(), &(trackerAdd.sin_addr));

    // Forcefully attaching socket to the port trackerPort
    int opt = 1;
    if (setsockopt(tracker_sd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    if (::bind(tracker_sd, (struct sockaddr *)&trackerAdd, sizeof(trackerAdd)) < 0)
    {
        LOGGER::ERROR("Tracker bind failed");
        exit(EXIT_FAILURE);
    }

    LOGGER::SUCCESS("Tracker binded");

    int client_sd;
    // it can keep at max 500 connection requests at a time
    if (listen(tracker_sd, MAXREQUESTS) < 0){
        LOGGER::ERROR("Tracker is not able to listen");
        exit(EXIT_FAILURE);
    }

    LOGGER::SUCCESS("Tracker listening on port=> %d", trackerPort);

    while(true){

        socklen_t size;

        if ((client_sd = accept(tracker_sd, (struct sockaddr *)&trackerAdd,
                            &size)) < 0){
            
            LOGGER::ERROR("Connection failed with client!!");
            exit(EXIT_FAILURE);
        }

        LOGGER::SUCCESS("Connection established with client %s",getClientIP(client_sd).c_str());
        LOGGER::INFO("client_sd %d",client_sd);
        pthread_create(&clientLoginThreadIds[noOfClientsLoggedIn++],NULL,
        serverNewClient,&client_sd);

    }

    for(int i = 0;i < noOfClientsLoggedIn; i++){
        pthread_join(clientLoginThreadIds[i],NULL);
    }

    return 0;

}
