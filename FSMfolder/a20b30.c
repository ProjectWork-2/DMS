#include <stdio.h>
#include <string.h>
#include "fsm.h"


int main(){

    fsm_t *fsm = create_new_fsm("22a33b");
    state_t *q00,*q01,*q02,*q10,*q11,*q12;
    char ipa='a',ipb='b';
    q00 = create_new_state("q00",FSM_TRUE);
    q01 = create_new_state("q01",FSM_FALSE);
    q02 = create_new_state("q02",FSM_FALSE);
    q10 = create_new_state("q10",FSM_FALSE);
    q11 = create_new_state("q11",FSM_FALSE);
    q12 = create_new_state("q12",FSM_FALSE);
    set_fsm_initial_state(fsm,q00);
    create_and_insert_new_tt_entry(&q00->state_trans_table,&ipa,1,NULL,q10);
    create_and_insert_new_tt_entry(&q00->state_trans_table,&ipb,1,NULL,q01);
    create_and_insert_new_tt_entry(&q01->state_trans_table,&ipa,1,NULL,q11);
    create_and_insert_new_tt_entry(&q01->state_trans_table,&ipb,1,NULL,q02);
    create_and_insert_new_tt_entry(&q02->state_trans_table,&ipa,1,NULL,q12);
    create_and_insert_new_tt_entry(&q02->state_trans_table,&ipb,1,NULL,q00);
    create_and_insert_new_tt_entry(&q10->state_trans_table,&ipa,1,NULL,q00);
    create_and_insert_new_tt_entry(&q10->state_trans_table,&ipb,1,NULL,q11);
    create_and_insert_new_tt_entry(&q11->state_trans_table,&ipa,1,NULL,q01);
    create_and_insert_new_tt_entry(&q11->state_trans_table,&ipb,1,NULL,q12);
    create_and_insert_new_tt_entry(&q12->state_trans_table,&ipa,1,NULL,q02);
    create_and_insert_new_tt_entry(&q12->state_trans_table,&ipb,1,NULL,q10);
    char input[100];
    fsm_bool_t fsm_result;
    fsm_error_t fsm_error;
    fsm_output_buff_t fsm_output_buff;
    init_fsm_output_buffer(&fsm_output_buff);
    printf("\t\tFSM THAT ONLY ACCEPTS STRINGS IN WHICH 2|N(a) and 3|N(b)\n\n");
    printf("Enter Input String : ");
    fgets(input, 100, stdin);    
    fsm_error = execute_fsm(fsm,input,strlen(input),NULL,&fsm_result);
    if(fsm_error == FSM_NO_ERROR){
        if(fsm_result == FSM_TRUE){
            printf("Input String is Accepted\n");
        }
        else{
            printf("Input String is Rejected\n");
        }
    }
    return 0;
}