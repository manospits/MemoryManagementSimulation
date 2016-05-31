#include <iostream>
#include <cstring>
#include <cstdlib>
#include "memory_management.h"
#include "generator.h"
#include <cstdlib>
#include <ctime>
#include <sys/wait.h>
#include <cmath>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <pthread.h>
#include <unistd.h>
#define MAX_TIME 10000
#define PERMS 0666
#include <fstream>
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
    };


struct sembuf semup[9]={{0,1,0},{1,1,0},{2,1,0},{3,1,0},{4,1,0},{5,1,0},{6,1,0},{7,1,0},{8,1,0}};
struct sembuf semdown[9]={{0,-1,0},{1,-1,0},{2,-1,0},{3,-1,0},{4,-1,0},{5,-1,0},{6,-1,0},{7,-1,0},{8,-1,0}};
//sh mem between G-transmiter and receiver MEMORY
typedef struct {
    list<message>::iterator it;
    list<message>::iterator en;
    list<message> *messages;
    int time;
    bool end;
    int max_time;
}Mes_G_T;

//sh mem between transmitter and receiver
typedef struct Mes_P_T{
    bool final;
    int id;
    int size;
    char s[30];
    int time;
    bool end;
    int max_time;
    void set_values(message a){id=a.ret_pid();size=a.ret_size();int len=a.ret_string().copy(s,30);s[len]='\0';}
    message ret_message(void){message a(id,size,s);return a;}
}Mes_P_T;

//sh mem arguments
class args{
    int semid;
    Mes_G_T *m1;
    Mes_P_T *m2;
    bool flag;
    public:
    int& ret_semid(void){return semid;}
    Mes_G_T* ret_messages1(void){return m1;}
    void set_messages1(Mes_G_T *a){m1=a;}
    Mes_P_T* ret_messages2(void){return m2;}
    void set_messages2(Mes_P_T *a){m2=a;}
};

//transmitter THREAD
void *transmitter(void *param){
    args  *arguments=(args *)param;
    int semid=arguments->ret_semid();
    Mes_G_T *mes1=arguments->ret_messages1();
    Mes_P_T *mes2=arguments->ret_messages2();
    list<message>::iterator m;
    list<message>::iterator e;
    list<message>::iterator t;
    int time;
    bool flag_stop=false;
    while(1){
        semop(semid,&semdown[1],1);
        semop(semid,&semdown[0],1);
        m=mes1->it;
        e=mes1->en;
        /* cout<<"transmitter call"<<endl; */
        time=mes1->time;
        //iterate and send all Messages from generator to receiver
        for(;m!=e;m++){
            t=m;
            t++;
            message a(m->ret_pid(),m->ret_size(),m->ret_string());
            semop(semid,&semdown[3],1);
            if (mes1->end){
                mes2->end=true;
                mes2->max_time=mes1->max_time;
            }
            if(t==e){//if it is the last message receiver thread must knnow it
                mes2->final=true;
                mes2->time=time;
            }
            else{
                mes2->final=false;
            }
            //sending only one message at a time
            mes2->set_values(a);
            /* cout<< "message in transmitter: "<< a.ret_pid() <<" " << a.ret_string()<<" "<< a.ret_size() <<endl; */
            semop(semid,&semup[3],1);
            semop(semid,&semup[4],1);
            /* cout<<"transmitter blocked"<<endl; */
            if(m->ret_string()=="stop"){
                flag_stop=true;
                break;
            }
            semop(semid,&semdown[5],1);
            /* cout<<"transmitter unblocked"<<endl; */
        }
        if(flag_stop){
            break;
        }
        semop(semid,&semup[0],1);
        semop(semid,&semup[2],1);
    }
    semop(semid,&semup[2],1);
    /* cout<<"Transmitter exited"<<endl; */
    pthread_exit(0);
}

void *receiver(void *param){
    args  *arguments=(args *)param;
    int semid=arguments->ret_semid();
    Mes_G_T *mes1=arguments->ret_messages1();
    Mes_P_T *mes2=arguments->ret_messages2();
    list<message>::iterator m;
    list<message>::iterator e;
    list<message>::iterator t;
    list<message> received;
    bool list_full=false;
    int time;
    bool end=false;
    int max_time;
    while(1){
        semop(semid,&semdown[4],1);
        /* cout<<"receiver call"<<endl; */
        //receive messages and add them into queue
        semop(semid,&semdown[3],1);
        if(mes2->final==true){
            list_full=true;
            time=mes2->time;
        }
        if(mes2->end){
            end=true;
            max_time=mes2->max_time;
        }
        received.push_front(mes2->ret_message());
        semop(semid,&semup[3],1);
        //if last message received send all messages to memory process
        if(list_full){
            list_full=false;
            semop(semid,&semdown[6],1);
            for(list<message>::iterator it=received.begin();it!=received.end();it++){
                mes1->messages->push_front(*it);
            }
            mes1->time=time;
            mes1->end=end;
            mes1->max_time=max_time;
            semop(semid,&semup[6],1);
            semop(semid,&semup[8],1);
            received.clear();
            if(mes2->ret_message().ret_string()=="stop"){
                break;
            }
            semop(semid,&semdown[7],1);
        }
        semop(semid,&semup[5],1);
    }
    semop(semid,&semup[5],1);
    /* cout<<"Receiver exited"<<endl; */
    pthread_exit(0);
}

using namespace std;

int main(int argc ,char*argv[]){
    int S=1024,lo=64,hi=256,t=10,T=10,t_to_next_creation,M_id,
        semid,shmid_transmiter,shmid_receiver,shmid_messages
        ,semid_start,max_time=MAX_TIME,status;
    Mes_G_T *mes_G_T1,*mes_T2_M;
    Mes_P_T *mes_T1_T2;
    args a,b;
    /* bool contend=false; */
    char c='n';
    union semun arg1,arg2;
    arg1.val=1;
    arg2.val=0;
    srand(time(0));
    ofstream logfile;
    logfile.open("mem_sim.log");
    //PARAMETER CHECK
    for(int i=1;i<argc;i++){
        if(strcmp(argv[i],"--lo")==0){
            lo=atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"--hi")==0){
            hi=atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-t")==0){
            t=atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-T")==0){
            T=atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"-S")==0){
            S=atoi(argv[i+1]);
        }
        else if(strcmp(argv[i],"--time")==0){
             max_time=atoi(argv[i+1]);
        }
        else if (strcmp(argv[i],"-a")==0){
            c=argv[i+1][0];
        }
        /* else if(strcmp(argv[i],"--novpl")==0){ */
            /* contend=true; */
        /* } */
    }
    //SHARED MEMORY
    if((shmid_messages = shmget ((key_t)IPC_PRIVATE , sizeof(Mes_P_T), PERMS| IPC_CREAT))==-1) return 1;
    if((shmid_transmiter = shmget ((key_t)IPC_PRIVATE , sizeof(Mes_G_T), PERMS| IPC_CREAT))==-1) return 1;
    if((shmid_receiver = shmget ((key_t)IPC_PRIVATE , sizeof(Mes_G_T), PERMS| IPC_CREAT))==-1) return 1;
    //SEMAPHORES
    if((semid = semget ((key_t)IPC_PRIVATE , 9, PERMS| IPC_CREAT))==-1) return 1;
    if((semid_start = semget ((key_t)IPC_PRIVATE , 1, PERMS| IPC_CREAT))==-1) return 1;
    //INITIALIZATION
    if((mes_G_T1=(Mes_G_T *)shmat(shmid_transmiter,0,0))==(void *)-1) return 1;
    if((mes_T1_T2=(Mes_P_T *)shmat(shmid_messages,0,0))==(void *)-1) return 1;
    if((mes_T2_M=(Mes_G_T *)shmat(shmid_receiver,0,0))==(void *)-1) return 1;
    semctl(semid,0,SETVAL,arg1);//sh mem
    semctl(semid,1,SETVAL,arg2);//transmiter can continue
    semctl(semid,2,SETVAL,arg2);//for blocking generator
    semctl(semid,3,SETVAL,arg1);//sh mem T-R THREADS
    semctl(semid,4,SETVAL,arg2);//receiver can continue
    semctl(semid,5,SETVAL,arg2);//for blocking transmitter
    semctl(semid,6,SETVAL,arg1);//sh mem R-M
    semctl(semid,7,SETVAL,arg2);//for blocking memory
    semctl(semid,8,SETVAL,arg2);//for blocking receiver
    semctl(semid_start,0,SETVAL,arg2);
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_t transmitter_id;
    //TRANSMITTTER-THREAD INITIALIZATION
    a.ret_semid()=semid;
    a.set_messages1(mes_G_T1);
    a.set_messages2(mes_T1_T2);
    pthread_create(&transmitter_id,&attr,transmitter,&a);
    cout<<"Call with values"<<endl;
    if((M_id=fork())!=0){
        //generator
        semop(semid_start,&semdown[0],1);
        bool flag_mes=false;
        cout <<"low: "<< lo <<" high: " << hi<<" t for events: " << t<<" T for calculations: " << T << " algorithm : "<<c<<endl;
        generator gen(t,T,hi,lo);
        t_to_next_creation=gen.ret_happening_start();
        for(int i=1;i<max_time;i++){
            t_to_next_creation--;
            for(list<vprocess>::iterator it=gen.ret_vplist().begin();it!=gen.ret_vplist().end()&&!gen.ret_vplist().empty() ;){
                (*it).ret_time()--;
                if(it->ret_time()<0){
                    gen.ret_vplist().erase(it++);
                    continue;
                }
                if((*it).ret_time()==0){
                    gen.ret_messages().insert(*it);
                    flag_mes=true;
                    (*it).ret_time()=it->get_calc();
                    if(it->get_num()==0){
                        it->ret_flag()="vp_stop";
                    }
                    else if(it->get_num()%2==0){
                        it->ret_flag()=="vp_swapout";
                    }
                    else{
                        it->ret_flag()=="vp_swapin";
                    }
                }
                it++;
            }
            /* cout<<endl; */
            if(t_to_next_creation==0){
                flag_mes=true;
                t_to_next_creation=ceil((-log(randa_b()))/(1.0/t));
                if(t_to_next_creation==0) t_to_next_creation=1;
                gen.create_vp();
                gen.ret_messages().insert(gen.ret_vplist().front());
                //-------------------------------------------VP_START
                gen.ret_vplist().front().ret_flag()="vp_swapout"; //next message to be sent
                gen.ret_vplist().front().ret_time()=gen.ret_vplist().front().get_calc();
            }
            if(flag_mes==true){
                //sending messages
                flag_mes=false;
                semop(semid,&semdown[0],1);
                mes_G_T1->it=gen.ret_messages().ret_messages().begin();
                mes_G_T1->en=gen.ret_messages().ret_messages().end();
                mes_G_T1->time=i;
                /* cout<<"time in generator "<<i<<endl; */
                /* cout<<"Messages to sent"<<endl; */
                /* for(list<message>::iterator it=gen.ret_messages().ret_messages().begin();it!=gen.ret_messages().ret_messages().end();it++) */
                    /* cout<<"g "<<it->ret_pid()<<" "<<it->ret_string()<<"| "; */
                /* cout<<endl; */
                semop(semid,&semup[0],1);
                semop(semid,&semup[1],1);
                /* cout<<"generator blocked"<<endl; */
                semop(semid,&semdown[2],1);
                /* cout<<"generator unblocked"<<endl; */
                gen.ret_messages().ret_messages().clear();
            }
        }
        /* cout<<"Generator End"<<endl; */
        message a(0,0,"stop");
        gen.ret_messages().ret_messages().push_front(a);
        semop(semid,&semdown[0],1);
        mes_G_T1->time=max_time;
        mes_G_T1->it=gen.ret_messages().ret_messages().begin();
        mes_G_T1->en=gen.ret_messages().ret_messages().end();
        semop(semid,&semup[0],1);
        /* cout<<"Termination message sent"<<endl; */
        semop(semid,&semup[1],1);
        pthread_join(transmitter_id,NULL);
        waitpid(M_id, &status, WUNTRACED);
    }
    else{
            //Memory
            Memory mem(S,c,ceil(log2(lo)));
            bool print_space=false;
            waiting_queue wait;
            int time=0;
            long double mean_of_free_space=0,count=0;
            unsigned long int sum2=0,countofe=0;
            pthread_attr_t attr2;
            pthread_attr_init(&attr2);
            pthread_t receiver_id;
            //RECEIVER-THREAD INITIALIZATION
            b.ret_semid()=semid;
            b.set_messages1(mes_T2_M);
            b.set_messages2(mes_T1_T2);
            pthread_create(&receiver_id,&attr2,receiver,&b);
            list<message> received;
            semop(semid,&semdown[6],1);
            mes_T2_M->messages=&received;
            semop(semid,&semup[6],1);
            semop(semid_start,&semup[0],1);
            int out,i;
            for(i=0;i<max_time;i++){
                mem.plus();
                /* cout<<"mem residence total"<<mem.ret_res()<<endl; */
                if(i==time){
                    if(time!=0){
                        semop(semid,&semup[7],1);
                    }
                    semop(semid,&semdown[8],1);
                }
                /* cout<<"time : "<<i<<endl; */
                wait.advance();//advancing waiting list
                //checking if there is any message that can complete its request
                for(list<waiting_queue_elem>::iterator it=wait.ret_waiting_messages().begin();it!=wait.ret_waiting_messages().end();){
                    if(mem.insert_pid(it->ret_message().ret_size(),it->ret_message().ret_pid(),c)==0){
                        print_space=true;
                        logfile<< i <<" : "<<"process : "<<it->ret_message().ret_pid()<<" "<<it->ret_message().ret_string()<<" message removed from queue"<<endl;
                        wait.remove_vpid_from_w((it++)->ret_message().ret_pid());
                    }
                    else{
                        it++;
                    }
                }
                //checking for suspended messages
                for(list<waiting_queue_ops_elem>::iterator it=wait.ret_waitings_ops().begin();it!=wait.ret_waitings_ops().end();){
                    if(it->ret_list().begin()->ret_time()==0){
                        if(it->ret_list().begin()->ret_message().ret_string()=="vp_swap"){
                            out=mem.swap(it->ret_list().begin()->ret_message().ret_size(),it->ret_list().begin()->ret_message().ret_pid(),c);
                            if(out==0||out==1){
                                print_space=true;
                                if(out==1){
                                    logfile<<i <<" : "<<"process : "<<it->ret_list().begin()->ret_message().ret_pid()<<" "
                                        <<it->ret_list().begin()->ret_message().ret_string()<<"out delayed message removed from queue"<<endl;
                                    wait.remove_message_from_ops((it++)->ret_vpid());
                                    it=wait.ret_waitings_ops().begin();
                                    continue;
                                }
                                else{
                                    logfile<<i <<" : "<<"process : "<<it->ret_list().begin()->ret_message().ret_pid()<<" "
                                        <<it->ret_list().begin()->ret_message().ret_string()<<"in delayed message removed from queue"<<endl;
                                    wait.remove_message_from_ops((it++)->ret_vpid());
                                }
                            }
                            else if(out==-1){
                                wait.insert_message(it->ret_list().begin()->ret_message(),true);
                                logfile<<i <<" : "<<"process : "<<it->ret_list().begin()->ret_message().ret_pid()<<" "
                                    <<it->ret_list().begin()->ret_message().ret_string()<<" delayed message inserted in queue"<<endl;
                                wait.remove_message_from_ops((it++)->ret_vpid());
                            }
                            else{
                                 /* cout<<"ERROR IN WAITING QUEUE"<<endl; */
                            }
                        }
                        else if(it->ret_list().begin()->ret_message().ret_string()=="vp_stop"){
                            print_space=true;
                            logfile<<i <<" : "<<"process : "<<it->ret_list().begin()->ret_message().ret_pid()<<" "
                                    <<it->ret_list().begin()->ret_message().ret_string()<<" delayed message removed from queue"<<endl;
                            mem.remove_pid(it->ret_vpid());
                            wait.remove_message_from_ops((it++)->ret_vpid());
                            it=wait.ret_waitings_ops().begin();
                            continue;
                        }
                    }
                    else{
                         it++;
                    }
                }
                //checking for new received Messages
                semop(semid,&semdown[6],1);
                if(!received.empty()){
                    time=mes_T2_M->time;
                    /* cout<<"time "<<time <<"time here"<<i<<endl; */
                    if(received.front().ret_string()=="stop"){
                        /* logfile<<"time to stop "<<time<<endl; */
                        received.clear();
                        semop(semid,&semup[6],1);
                        continue;
                    }
                    print_space=true;
                    for(list<message>::iterator it=received.begin();it!=received.end();){
                        logfile<<i <<" : "<<"message received "<<it->ret_pid()<<" "<<it->ret_string()<<" "<<it->ret_size()<<endl;
                        if(!wait.vpid_in_w_or_ops(it->ret_pid())){
                            if(it->ret_string()=="vp_swap"){
                                if((out=mem.swap(it->ret_size(),it->ret_pid(),c))==-1){
                                    logfile<<i <<" : "<<"process : "<< it->ret_pid() <<" "<<it ->ret_string() <<" message inserted in queue "<<endl;
                                    wait.insert_message(*it);
                                    received.erase(it++);
                                }
                                else{
                                    if(out==1){
                                        logfile<<i <<" : "<<"process : "<<it->ret_pid()<<" "<<it->ret_string()<<"out"<<endl;
                                        received.erase(it++);
                                        /* it=received.begin(); */
                                        /* continue; */
                                    }
                                    else if(out==0){
                                        logfile<<i <<" : "<<"process : "<<it->ret_pid()<<" "<<it->ret_string()<<"in"<<endl;
                                        received.erase(it++);
                                    }
                                }
                            }
                            else if(it->ret_string()=="vp_start"){
                                if((mem.insert_pid(it->ret_size(),it->ret_pid(),c))==-1){
                                    logfile<<i <<" : "<< "process : "<< it->ret_pid() <<" "<<it ->ret_string() <<" message inserted in queue"<<endl;
                                    wait.insert_message(*it);
                                }
                                else{
                                    logfile<<i <<" : "<< "process : "<<it->ret_pid()<<" "<<it->ret_string()<<endl;
                                }
                                received.erase(it++);
                            }
                            else if(it->ret_string()=="vp_stop"){
                                logfile<<i <<" : "<<"process : "<<it->ret_pid()<<" "<<it->ret_string()<<endl;
                                mem.remove_pid(it->ret_pid());
                                received.erase(it++);
                                /* it=received.begin(); */
                                /* continue; */
                            }
                        }
                        else{
                            logfile<<i<<" : "<<"process : "<<it->ret_pid()<<" "<<it->ret_string()<<" message suspended"<<endl;
                            wait.insert_message(*it);
                            received.erase(it++);
                        }
                    }
                    received.clear();
                }
                semop(semid,&semup[6],1);
                if(print_space){
                    /* count++; */
                    /* mean_of_free_space+=mem.usable(); */
                    mem.print(logfile,i);
                    /* sum2+=mem.usable()*mem.usable(); */
                    logfile<<"-----memory free space : " <<mem.freespace()<<" space taken: "<<mem.spacetaken()<<" usable free space: "<<mem.usable()<<" -----"<<endl;
                    print_space=false;
                    wait.print(logfile,i);
                    logfile<<"\n"<<endl;
                }
                mean_of_free_space+=(double)mem.usable();
                /* cout<<mem.usable()<<endl; */
                sum2+=mem.usable2();
                /* cout<<sum2<<endl; */
                /* if(mem.numofe()!=0) */
                    /* sum2+=mem.usable2()/mem.numofe()-(mem.usable()*mem.usable())/(mem.numofe()*mem.numofe()); */
                countofe+=mem.numofe();
                count++;
            }
            logfile<<"-----Memory management simulation finished-----"<<endl;
            cout<<"-----Memory management simulation finished-----"<<endl;
        /* mem.print(logfile); */
        /* cout<<"-----memory free space : " <<mem.freespace()<<" space taken: "<<mem.spacetaken()<<" usable free space: "<<mem.usable()<<" -----"<<endl; */
        /* cout<<"sum2: "<<sum2<<endl; */
        mean_of_free_space/=countofe;
        double variance=sum2/countofe - (mean_of_free_space*mean_of_free_space);
        cout<<"-Mean of free usable spaces       : "<<mean_of_free_space<<endl;
        cout<<"-Variance of free usable spaces   : "<<variance<<endl;
        cout<<"-Average request size             : "<<mem.ret_req()/mem.ret_totala()<<endl;
        cout<<"-Number of requests               : "<<mem.ret_totala()<<endl;
        cout<<"-Average residence stay           : "<<(double)mem.ret_res()/mem.ret_totald()<<endl;
        cout<<"-Time - memory Product efficiency : "<<((double)mem.ret_res()*mem.ret_req()/mem.ret_totald())/(S*i)<<endl;
        logfile<<"-Mean of free usable spaces       : "<<mean_of_free_space<<endl;
        logfile<<"-Variance of free usable spaces   : "<<variance<<endl;
        logfile<<"-Average request size             : "<<mem.ret_req()/mem.ret_totala()<<endl;
        logfile<<"-Number of requests               : "<<mem.ret_totala()<<endl;
        logfile<<"-Average residence stay           : "<<(double)mem.ret_res()/mem.ret_totald()<<endl;
        logfile<<"-Time - memory Product efficiency : "<<((double)mem.ret_res()*mem.ret_req()/mem.ret_totald())/(S*i)<<endl;

        semop(semid,&semup[7],1);
        pthread_join(receiver_id,NULL);
    }
    for(int i=0;i<9;i++)
        semctl(semid,i,IPC_RMID);
    semctl(semid_start,0,IPC_RMID);
    shmctl(shmid_receiver,0,IPC_RMID);
    shmctl(shmid_messages,0,IPC_RMID);
    shmctl(shmid_transmiter,0,IPC_RMID);
    logfile.close();
}
