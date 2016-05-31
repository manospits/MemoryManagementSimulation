#include <iostream>
#include <cstdlib>
#include "generator.h"
#include <cmath>

using namespace std;
double randa_b(void)
{
    return (double)rand() / (double)((unsigned)RAND_MAX + 1);
}

vprocess::vprocess(int t,int lo ,int hi,int k,int t_for_start=0){
    int calc;
    total=0;
    /* cout<<t<<endl; */
    /* srand((unsigned)time(NULL)); */
    size=((double) (rand()%(RAND_MAX-1)) / (double)(RAND_MAX)) * (hi-lo+1) + lo;
    time_state=t_for_start;
    flag="vp_start";
    number_of_calcs=2*k+1;
    for(int i=0;i<number_of_calcs;i++){
        calc=ceil(-log(randa_b())/(1.0/ t));
        if(calc==0){
            calc=1;
        }
        /* cout<< calc<<" "; */
        total+=calc;
        calcs.push_front(calc);
    }
    /* cout<<endl; */
}

int vprocess:: get_calc(void){
    int b;
    if(!calcs.empty()){
        b=calcs.front();
        calcs.pop_front();
        number_of_calcs--;
    }
    else{
        return 0;
    }
    return b;
}

void Messages::insert(vprocess a){
    message m(a.retvpid(),a.ret_size(),a.ret_message_type());
    messages.push_front(m);
}

generator::generator(int t,int T,int hi,int lo){
    t_for_happenings=t;
    T_for_calcs=T;
    this->hi=hi;
    this->lo=lo;
    id=0;
    t_for_happening_creation=ceil(-log(randa_b())/(1.0/ t));
    if(t_for_happening_creation==0) t_for_happening_creation=1;
}

void generator::create_vp(void){
    int k;
    k=(rand()%19)+2;
    /* cout<<k<<endl; */
    vprocess a(T_for_calcs,lo,hi,k);
    a.setvpid(id++);
    /* cout<<"process id : "<<id<<endl; */
    vprocesses.push_front(a);
}
