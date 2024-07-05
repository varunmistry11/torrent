#include "global.h"
#include "customlogger.h"
#include "dataStructures.h"
#include "clientServer.h"
#include "semaphore.h"
#include "fileFunctions.h"
#include "serveServerCommands.h"
#include "downloadFileCommand.h"
#include "serveClientCommands.h"

bool isCommanValid(string command, int argsLen, int requiredLen){
    if(argsLen < requiredLen){
        LOGGER::ERROR("Less arguments in %s",command);
        return false;
    }
            
    if(argsLen > requiredLen){
        LOGGER::ERROR("More arguments in %s", command);
        return false;
    }

    return true;
}

int main(int argc, char** argv){

    getServerIPAndPORT(argv[1]);

    sem_init(&fileInfoMap_RLOCK, 0, 1);
    sem_init(&fileInfoMap_WLOCK, 0, 1);

    string serverInfo = serverIPAddress + "|" + to_string(serverPort);
    // make thread for server running paralelly
    pthread_create(&serverThreadId, NULL, createServerThread, &serverInfo);

    // get trackerInfo ip and port
    readTrackerInfoFromFile(argv[2]);

    int client_sd,tracker_ssd;

    if ((client_sd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        LOGGER::ERROR("Socket creation error client");
        return -1;
    }

    LOGGER::INFO("client_sd client %d", client_sd);
    
    struct sockaddr_in trackerAdd;

    LOGGER::SUCCESS("Socket created");

    LOGGER::INFO("trcaker port => %d",trackerPort);
    LOGGER::INFO("tracker ip address =>%s",trackerIPAddress.c_str());

    trackerAdd.sin_family = AF_INET;
    trackerAdd.sin_port = htons(trackerPort);
    trackerAdd.sin_addr.s_addr = inet_addr(trackerIPAddress.c_str());
    
    if ((tracker_ssd = connect(client_sd, (struct sockaddr *)&trackerAdd,
                                 sizeof(trackerAdd))) < 0){
        LOGGER::ERROR("Connection Failed on ip : %s",trackerIPAddress.c_str());
        return -1;
    }

    LOGGER::SUCCESS("Connection established with tracker");

    string command;

    tracker_sd = client_sd;

    while(1){

        cout << "\nCommand $: "; 

        command = "";
        getline(cin, command);
        vector<string> args = getCommandArguments(command);
        int argsLen = args.size();

        if(args[0] == "create_user"){

            if(isCommanValid(args[0],argsLen,3))
                serveCreateUser(client_sd,command);
        }
        
        else if(args[0] == "login"){
            
            LOGGER::INFO("login");
            if(isCommanValid(args[0],argsLen,3))
                serveLogin(client_sd,command);  
        }
        else if(isUserLoggedIn){
            
            if(args[0] == "create_group"){
                LOGGER::INFO("create_group command detected");
                if(isCommanValid(args[0],argsLen,2))
                    serveCreateGroup(client_sd,command);
            }

            else if(args[0] == "join_group"){
                LOGGER::INFO("join_group command detected");
                if(isCommanValid(args[0],argsLen,2))
                    serveJoinGroup(client_sd,command);
            }

            else if(args[0] == "list_requests"){
                LOGGER::INFO("list_requests command detected");
                if(isCommanValid(args[0],argsLen,2))
                    serveListPendingRequest(client_sd,command);
            }

            else if(args[0] == "accept_request"){
                LOGGER::INFO("accept_request command detected");
                if(isCommanValid(args[0],argsLen,3))
                    serveAcceptRequest(client_sd,command);
            }

            else if(args[0] == "list_groups"){
                LOGGER::INFO("list_groups command detected");
                if(isCommanValid(args[0],argsLen,1))
                    serveListAllGroups(client_sd,command);
            }
        
            else if (args[0] == "upload_file"){
                //[upload_file,filePath,groupid]
                LOGGER::INFO("upload_file command detected");
                if(isCommanValid(args[0],argsLen,3))
                    serveUploadFile(client_sd,command);
            }

            else if(args[0] == "download_file"){
                //[download_file, groupid, filename, destPath]
                LOGGER::INFO("download_file command detected");
                if(isCommanValid(args[0],argsLen,4))
                    serveDownloadFile(client_sd, command);
            }

            else if(args[0] == "list_files"){
                LOGGER::INFO("list_files command detected");
                if(isCommanValid(args[0],argsLen,2))
                    serveListAllFiles(client_sd,command);
            }

            else if(args[0] == "logout"){
                LOGGER::INFO("logout command detected");
                if(isCommanValid(args[0],argsLen,1))
                    serveLogout(client_sd,command);
            }
            else if(args[0] == "leave_group"){
                LOGGER::INFO("leave_group command detected");
                if(isCommanValid(args[0],argsLen,2))
                    serveLeaveGroup(client_sd,command);
            }
            else if(args[0] == "stop_share"){
                LOGGER::INFO("stop_share command detected");
                if(isCommanValid(args[0],argsLen,3))
                    serveStopShare(client_sd,command);
            }
            else {
                LOGGER::ERROR("Invalid command!!");
            }
        }

        else {
            LOGGER::ERROR("You are not logged in");
        } 

    }

     // JOIN all the joinable threads here.
    for(int i = 0;i < liveThreads;i++){
        pthread_join(threadIds[i],NULL);
    }

    pthread_join(serverThreadId,NULL);


    return 0;

}
