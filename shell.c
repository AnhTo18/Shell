#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
void clear(){					//clear console for commands
    static int count=1;
    if(count){
        write(STDOUT_FILENO, "\e[1;1H\e[2J", 12);		//use regrex "\e[1;1H\e[2j" for clearing
        count=0;
    }
}

char **parseArray(char *in){
    char *token=strtok(in, " ") ;  //delimeter will just be a single space
    char **tok=malloc(sizeof(char*) *255);
    int index=0;

    while(token!=NULL){            //can't have a null element
        tok[index]=token;
        index++;
        token=strtok(NULL, " ");
    }
    tok[index]=NULL;
    return tok;
}

int main(int argc, char**argv){
    char path[255],input[255],input2[255];
    int status=-1;
    char **parameters;
    int background=0;
    //Set the prompt
    char *prompt="";
    pid_t pid, pid1;
    if(argc==3&&strcmp("-p", argv[1])==0){    			//valid -p <prompt>
        prompt=argv[2];
        asprintf(&prompt, "%s%s", prompt, "> ");
    }else if(argc=1){       							// only -p and no specified optioned, so default is 308sh
        prompt="308sh> ";
    }else{
        prompt="invlaid";
    }


    //User input infinite loop
    while(1){

        if(waitpid(-1, &status, WNOHANG)>0){
            if (WIFSIGNALED(status)) {
                printf("child %d exited with signal %d\n", getpid(), WTERMSIG(status));
            }
            if (WIFEXITED(status)) {
                printf("child %d exited with status %d\n", getpid(), WEXITSTATUS(status));
            }
        }

        clear();										//clear console
        printf(prompt);									//print prompt

        //Get user input
        fgets(input, 200, stdin);						//input
        if(waitpid(-1, &status, WNOHANG)>0){
            if (WIFSIGNALED(status)) {
                printf("child %d exited with signal %d\n", getpid(), WTERMSIG(status));
            }
            if (WIFEXITED(status)) {
                printf("child %d exited with status %d\n", getpid(), WEXITSTATUS(status));
            }
        }
        input[strlen(input)-1]='\0';
        char c=input[strlen(input)-1];
        int size=strlen(input);
        strcpy(input2,input);

        // printf(input);
        // printf("\n");
        parameters=parseArray(input);

        //********************Builtin Commands************************************
        if(strcmp(input, "exit")==0){					//exit command
            free(parameters);
            return 0;
        }else if(strcmp(input, "pid")==0){    			//get pid command
            printf("Shell PID: %d \n", getpid());
        }else if(strcmp(input, "ppid")==0){   			//get ppid command
            printf("Shell PPID: %d \n", getpid());
        }


        else if(strncmp(input, "cd..", 4)==0){   //Going backward works now
            int track;
            getcwd(path,sizeof(path));
            chdir("..");                                //needs to fix path after going back
            int i;
            for(i=0;i<strlen(path);i++){                //two loops for removing chars until last / to fix path so cd works correctly after cd..
                if(path[i]=='/'){
                    track=i;
                }
            }
            //printf("last index of / is at %d\n", track);
            int j;
            for(j=strlen(path);j>track;j--){
                path[j]='\0';
                //printf(path);
                //printf("\n");
            }
            getcwd(path,sizeof(path));
            //printf("going back");
            //printf(path);
            //printf("\n");
        }



        else if(strncmp(input, "cd", 2)==0){
            if(parameters[1]==NULL){
                int track;
                getcwd(path,sizeof(path));
                chdir(getenv("HOME"));   //no arguments so go home
                int i;
                for(i=0;i<strlen(path);i++){
                    if(path[i]=='3'&&path[i+1]=='0'&&path[i+2]=='8'){    //kinda flimsy....
                        track=i;
                    }
                }
                int j;
                for(j=strlen(path);j>=track;j--){
                    path[j]='\0';
                    //printf(path);
                    //printf("\n");
                }



            }
            else{
                //printf(path);
                //printf("\n");
                getcwd(path,sizeof(path));
                //printf("hi there \n");
                path[strlen(path)]='/';
                //path[strlen(path)]='\0';
                int i;

                for(i=0;i<size-3;i++){                       //adding / and user input to create new path for cd
                    path[strlen(path)]=input[i+3];
                    path[strlen(path)+1]='\0';
                    //printf(path);
                    //printf("\n");
                }
                //printf(path);

                chdir(path);

            }
        }

        else if(strcmp(input, "pwd")==0){
            getcwd(path, sizeof(path));
            printf("%s\n",path);
        }


        else{
            //****************Non-Builtin Commands********************************
            if(waitpid(-1,&status,WNOHANG)>0){
                if(WIFSIGNALED(status)){
                    printf("child %d exited with signal %d\n", waitpid(-1,&status,WNOHANG), WTERMSIG(status));
                }
                if(WIFEXITED(status)){
                    printf(" child %d exited with status %d\n", waitpid(-1,&status,WNOHANG), WEXITSTATUS(status));
                }

            }
            if(input2[strlen(input2)-1]=='&'){
                input2[strlen(input2)-1]='\0';

            }
            parameters=parseArray(input2);
            int back=0;
            if(c=='&'){				//checking & for background process
                back=1;
            }
            if(!back){
                pid=fork();                                 //normal foreground wait for child to die then continue
                if (pid){
                    do{
                        waitpid(pid, &status, WUNTRACED);
                    }while (!WIFSIGNALED(status)&&!WIFEXITED(status));    //wouldn't go if child doesn't die
                    if (WIFSIGNALED(status)) {
                        printf("child %d exited with signal %d\n", getpid(), WTERMSIG(status));
                    }
                    if (WIFEXITED(status)) {
                        printf("child %d exited with status %d\n", getpid(), WEXITSTATUS(status));
                    }
                }
                else{
                    if(execvp(parameters[0], parameters)==-1){
                        printf("error \n");
                    }
                }

            }else if(back){                              //background doesn't wait for child to die
                pid=fork();
                if(pid){                                   //parent
                    pid1=fork();
                    if (pid1) {
                        waitpid(-1,&status,WUNTRACED);
                        if (WIFSIGNALED(status)){
                            printf("child %d exited with signal %d\n", getpid(), WTERMSIG(status));
                        }
                        if (WIFEXITED(status)){
                            printf("child %d exited with status %d\n", getpid(), WEXITSTATUS(status));
                        }
                    } else {                                //child
                        if (execvp(parameters[0], parameters)==-1) {
                            printf("error \n");
                        }
                        if (waitpid(-1, &status, WNOHANG)>0) {
                            if (WIFSIGNALED(status)) {
                                printf("child %d exited with signal %d\n", getpid(), WTERMSIG(status));
                            }
                            if (WIFEXITED(status)) {
                                printf("child %d exited with status %d\n", getpid(), WEXITSTATUS(status));
                            }
                        }

                    }
                }

            }

        }
    }


    return 0;
}

//***************************Helpers**********************************************





int nonBuiltn(char input[]){

}
