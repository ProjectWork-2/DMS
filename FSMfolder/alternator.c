#include <stdio.h>
#include <string.h>
#include "fsm.h"

void op(state_t *from, state_t *to,char *input_buff, unsigned int input_buff_size,fsm_output_buff_t *fsm_output_buff){
    char out;
    out = (*input_buff == '1') ? '0' : '1';
    fsm_output_buff->curr_pos += snprintf(fsm_output_buff->output_buffer + 
                                          fsm_output_buff->curr_pos, 
             (MAX_FSM_OUTPUT_BUFFER - fsm_output_buff->curr_pos - 1), 
             "%c", out);

}

int main(){

    fsm_t *fsm = create_new_fsm("FSM Alternate");
    char ip0 = '0',ip1 = '1';
    unsigned int keysize = 1;
    
    state_t *q0, *q1, *q2, *D;
    q0 = create_new_state("q0", FSM_FALSE);
    q1 = create_new_state("q1", FSM_TRUE);
    q2 = create_new_state("q2", FSM_TRUE);
    D = create_new_state("D", FSM_FALSE);
    set_fsm_initial_state(fsm, q0);


    create_and_insert_new_tt_entry(&q0->state_trans_table,&ip1, keysize, op, q1);
    create_and_insert_new_tt_entry(&q0->state_trans_table,&ip0, keysize, op, q2);

    create_and_insert_new_tt_entry(&q1->state_trans_table,&ip1, keysize, op, D);
    create_and_insert_new_tt_entry(&q1->state_trans_table,&ip0, keysize, op, q2);

    create_and_insert_new_tt_entry(&q2->state_trans_table,&ip1, keysize, op, q1);
    create_and_insert_new_tt_entry(&q2->state_trans_table,&ip0, keysize, op, D);

    create_and_insert_new_tt_entry(&D->state_trans_table,&ip1, keysize, op, D);
    create_and_insert_new_tt_entry(&D->state_trans_table,&ip0, keysize, op, D);



    fsm_bool_t fsm_result;
    fsm_error_t fsm_error;
    fsm_output_buff_t fsm_output_buff;
    init_fsm_output_buffer(&fsm_output_buff);
    char input[100];
    printf("\t\t FSM THAT ONLY ACCEPTS STRINGS WITH ALTERNATING 0's AND 1's\n\n");
    printf("Enter Input String : ");
    scanf("%s",input);

    fsm_error = execute_fsm(fsm,input,strlen(input), &fsm_output_buff,&fsm_result);

    if(fsm_error == FSM_NO_ERROR){
        if(fsm_result == FSM_TRUE){
            printf("Input String is Accepted!\n");
            printf("Output Generated : %s\n", fsm_output_buff.output_buffer);
        }
        else{
            printf("Input String is Rejected!\n");
        }
    }
    return 0;
}