#ifndef _gen_
#define _gen_
#include <iostream>
#include <list>

using namespace std;

double randa_b(void);

class vprocess{
        int vpid;
        int size;
        int total;
        int number_of_calcs;
        int time_state;
        string flag; //"vp_start" ,"vp_swapout","vp_swapin","stop"
        list<int> calcs;
    public:
        vprocess(int t,int lo,int hi,int k,int t_for_start);
        int retvpid(void){return vpid;}                     //returns vpid of the virtual process
        void setvpid(int a){vpid=a;}                        //sets vpid
        int get_calc(void);                                 //removes element from calcs list and return its value also removes 1 from number_of_calcs
        int& ret_time(void){return time_state;}             //returns time until next event
        int ret_size(void){return size;}                    //returns the size of the process
        int get_total(void){return total;}                  //returns total calcs
        int& get_num(void){return number_of_calcs;}         //returns number elements in list calcs
        string& ret_flag(void){return flag;}                //returns flag
        string ret_message_type(void){if (flag=="vp_swapin"||flag=="vp_swapout") return "vp_swap"; else return flag;} //returns the appropriate message for send
};

class message{
        int pid;        //vprocess id
        int size;       //size of the vprocess
        string op;      //operation type ={vp_start,vp_swap,vp_stop,stop} stop is used for normal termination of threads
    public:
        message(int a=0,int c=0,string d=""){pid=a;size=c;op=d;}    //a for pid c for size d for operation
        int ret_pid(void){return pid;}                              //return pid
        int ret_size(void){return size;}                            //returns size
        string ret_string(void){return op;}                         //returns operation
};

class Messages{
    list <message> messages;                        //list used in Generator for sending message in the transmitter thread
    public:
        void insert(vprocess a);                    //inserts a new message in queue
        void erase(void){messages.clear();}         //clears queue
        list<message>& ret_messages(void){return messages;}//returns message list
};

class generator{
        list <vprocess> vprocesses;                     //list of virtual processes
        Messages messages;                              //list of messages used for sending in the transmitter thread
        int t_for_happenings,T_for_calcs,hi,lo;
        int t_for_happening_creation;
        int id;
    public:
        generator(int t,int T,int hi,int lo);           //t for events creation T for calculations hi for vp size max lo for min
        int ret_happening_start(void){return t_for_happening_creation;} //returns first time for the next event
        Messages& ret_messages(void){return messages;}                  //returns list of messages
        list <vprocess>& ret_vplist(void){return vprocesses;}           //returns vprocesses list
        void create_vp(void);                                           //creates and initializes an new virual process
};

#endif
